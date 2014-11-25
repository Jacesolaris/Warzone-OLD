#include "client.h"
#include "snd_local.h"

#ifdef __USE_BASS__

//#define __BASS_PLAYER_BASED_LOCATIONS__ // UQ1: This puts player always at 0,0,0 and calculates sound positions from that...

#include "snd_bass.h"
#include "fast_mutex.h"
#include "tinythread.h"

using namespace tthread;

extern cvar_t		*s_khz;
extern vec3_t		s_entityPosition[MAX_GENTITIES];

extern int	s_soundStarted;
extern int	s_soundtime;		// sample PAIRS
extern int	s_numSfx;

qboolean EAX_SUPPORTED = qtrue;

#define MAX_BASS_CHANNELS	256

#define SOUND_3D_METHOD					BASS_3DMODE_NORMAL //BASS_3DMODE_RELATIVE

float MIN_SOUND_RANGE				=	256.0; //256.0
float MAX_SOUND_RANGE				=	2048.0; // 3072.0

int SOUND_CONE_INSIDE_ANGLE			=	-1;//120;
int SOUND_CONE_OUTSIDE_ANGLE		=	-1;//120;
float SOUND_CONE_OUTSIDE_VOLUME		=	0;//0.9;

// Use meters as distance unit, real world rolloff, real doppler effect
// 1.0 = use meters, 0.9144 = use yards, 0.3048 = use feet.
float SOUND_DISTANCE_UNIT_SIZE		=	0.3048; // UQ1: It would seem that this is close to the right conversion for Q3 units... unsure though...
// 0.0 = no rolloff, 1.0 = real world, 2.0 = 2x real.
float SOUND_REAL_WORLD_FALLOFF		=	1.0;//0.3048;//1.0; //0.3048
// 0.0 = no doppler, 1.0 = real world, 2.0 = 2x real.
float SOUND_REAL_WORLD_DOPPLER		=	0.3048; //1.0 //0.3048


qboolean BASS_UPDATE_THREAD_RUNNING = qfalse;
qboolean BASS_UPDATE_THREAD_STOP = qfalse;

thread *BASS_UPDATE_THREAD;


// channel (sample/music) info structure
typedef struct {
	DWORD			channel, originalChannel;			// the channel
	BASS_3DVECTOR	pos, vel, ang;	// position,velocity,angles
	vec3_t			origin;
	int				entityNum;
	int				entityChannel;
	float			volume;
	qboolean		isActive;
	qboolean		startRequest;
	qboolean		isLooping;
} Channel;

Channel		MUSIC_CHANNEL;

qboolean	SOUND_CHANNELS_INITIALIZED = qfalse;
Channel		SOUND_CHANNELS[MAX_BASS_CHANNELS];

//
// Channel Utils...
//

void BASS_InitializeChannels ( void )
{
	if (!SOUND_CHANNELS_INITIALIZED)
	{
		for (int c = 0; c < MAX_BASS_CHANNELS; c++) 
		{// Set up this channel...
			memset(&SOUND_CHANNELS[c],0,sizeof(Channel));
		}

		SOUND_CHANNELS_INITIALIZED = qtrue;
	}
}

void BASS_StopChannel ( int chanNum )
{
	if (BASS_ChannelIsActive(SOUND_CHANNELS[chanNum].channel) == BASS_ACTIVE_PLAYING)
	{
		BASS_ChannelStop(SOUND_CHANNELS[chanNum].channel);
		//BASS_ChannelPause(SOUND_CHANNELS[chanNum].channel);
	}

	SOUND_CHANNELS[chanNum].isActive = qfalse;
	SOUND_CHANNELS[chanNum].startRequest = qfalse;
	SOUND_CHANNELS[chanNum].isLooping = qfalse;
}

void BASS_StopEntityChannel ( int entityNum, int entchannel )
{
#pragma omp parallel for num_threads(8)
	for (int c = 0; c < MAX_BASS_CHANNELS; c++) 
	{
		if (SOUND_CHANNELS[c].entityNum == entityNum && SOUND_CHANNELS[c].isActive && SOUND_CHANNELS[c].entityChannel == entchannel)
		{
			BASS_StopChannel(c);
		}
	}
}

void BASS_FindAndStopSound ( DWORD handle )
{
#pragma omp parallel for num_threads(8)
	for (int c = 0; c < MAX_BASS_CHANNELS; c++) 
	{
		if (SOUND_CHANNELS[c].originalChannel == handle && SOUND_CHANNELS[c].isActive)
		{
			BASS_StopChannel(c);
		}
	}
}

void BASS_StopAllChannels ( void )
{
#pragma omp parallel for num_threads(8)
	for (int c = 0; c < MAX_BASS_CHANNELS; c++) 
	{
		if (SOUND_CHANNELS[c].isActive)
		{
			BASS_StopChannel(c);
		}
	}
}

void BASS_StopLoopChannel ( int entityNum )
{
#pragma omp parallel for num_threads(8)
	for (int c = 0; c < MAX_BASS_CHANNELS; c++) 
	{
		if (SOUND_CHANNELS[c].entityNum == entityNum && SOUND_CHANNELS[c].isActive && SOUND_CHANNELS[c].isLooping)
		{
			BASS_StopChannel(c);
		}
	}
}

void BASS_StopAllLoopChannels ( void )
{
#pragma omp parallel for num_threads(8)
	for (int c = 0; c < MAX_BASS_CHANNELS; c++) 
	{
		if (SOUND_CHANNELS[c].isActive && SOUND_CHANNELS[c].isLooping)
		{
			BASS_StopChannel(c);
		}
	}
}

int BASS_FindFreeChannel ( void )
{
	// Fall back to full lookup when we have started too many sounds for the update threade to catch up...
	for (int c = 0; c < MAX_BASS_CHANNELS; c++) 
	{
		if (!SOUND_CHANNELS[c].isActive)
		{
			return c;
		}
	}

	return -1;
}

void BASS_UnloadSamples ( void )
{
	if (BASS_ChannelIsActive(MUSIC_CHANNEL.channel) == BASS_ACTIVE_PLAYING)
	{
		BASS_ChannelStop(MUSIC_CHANNEL.channel);
		BASS_SampleFree(MUSIC_CHANNEL.channel);
		BASS_MusicFree(MUSIC_CHANNEL.channel);
		BASS_StreamFree(MUSIC_CHANNEL.channel);
	}

	if (SOUND_CHANNELS_INITIALIZED)
	{
		for (int c = 0; c < MAX_BASS_CHANNELS; c++) 
		{// Free channel...
			BASS_StopChannel(c);
			BASS_SampleFree(SOUND_CHANNELS[c].channel);
			BASS_MusicFree(SOUND_CHANNELS[c].channel);
			BASS_StreamFree(SOUND_CHANNELS[c].channel);
		}

		SOUND_CHANNELS_INITIALIZED = qfalse;
	}

	if (BASS_UPDATE_THREAD_RUNNING && thread::hardware_concurrency() > 1)
	{// More then one CPU core. We need to shut down the update thread...
		BASS_UPDATE_THREAD_STOP = qtrue;

		// Wait for update thread to finish...
		BASS_UPDATE_THREAD->join();
		BASS_UPDATE_THREAD_RUNNING = qfalse;
	}
}


HINSTANCE			bass = 0;								// bass handle
char				tempfile[MAX_PATH];						// temporary BASS.DLL

void BASS_Shutdown ( void )
{
	if (BASS_ChannelIsActive(MUSIC_CHANNEL.channel) == BASS_ACTIVE_PLAYING)
	{
		BASS_ChannelStop(MUSIC_CHANNEL.channel);
		BASS_SampleFree(MUSIC_CHANNEL.channel);
		BASS_MusicFree(MUSIC_CHANNEL.channel);
		BASS_StreamFree(MUSIC_CHANNEL.channel);
	}

	if (SOUND_CHANNELS_INITIALIZED)
	{
		for (int c = 0; c < MAX_BASS_CHANNELS; c++) 
		{// Free channel...
			BASS_StopChannel(c);
			BASS_SampleFree(SOUND_CHANNELS[c].channel);
			BASS_MusicFree(SOUND_CHANNELS[c].channel);
			BASS_StreamFree(SOUND_CHANNELS[c].channel);
		}

		SOUND_CHANNELS_INITIALIZED = qfalse;
	}

	if (BASS_UPDATE_THREAD_RUNNING && thread::hardware_concurrency() > 1)
	{// More then one CPU core. We need to shut down the update thread...
		BASS_UPDATE_THREAD_STOP = qtrue;

		// Wait for update thread to finish...
		BASS_UPDATE_THREAD->join();
	}

	BASS_Free();
}

#if 0
/* load BASS and the required functions */
qboolean LoadBASS ( void )
{
	BYTE	*data;
	HANDLE	hfile;
	HRSRC	hres;
	DWORD	len, c;
	char	temppath[MAX_PATH];

	/* get the BASS.DLL resource */

	/*
	if
	(
		!(hres = FindResource( GetModuleHandle( "openjk.x86.exe"), "BASS_DLL", RT_RCDATA)) ||
		!(len = SizeofResource( GetModuleHandle( "openjk.x86.exe"), hres)) ||
		!(hres = (HRSRC)LoadResource( GetModuleHandle( "openjk.x86.exe"), hres)) ||
		!(data = (byte *)LockResource( hres))
	)*/
	if (!(hres=FindResource(GetModuleHandle(NULL),"BASS_DLL",RT_RCDATA))
		|| !(len=SizeofResource(NULL,hres))
		|| !(hres=(HRSRC)LoadResource(NULL,hres))
		|| !(data=(byte *)LockResource(hres)))
	{
		Com_Printf( "Error: Can't get the BASS.DLL resource\n" );
		return ( qfalse );
	}

	/* get a temporary filename */
	GetTempPath( MAX_PATH, temppath );
	GetTempFileName( temppath, "bas", 0, tempfile );

	/* write BASS.DLL to the temporary file */
	if
	(
		INVALID_HANDLE_VALUE ==
			(hfile = CreateFile( tempfile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL))
	)
	{
#ifndef CGAME
		Com_Printf( "Error: Can't write BASS.DLL\n" );
#else
		CG_Printf( "Error: Can't write BASS.DLL\n" );
#endif
		return ( qfalse );
	}

	WriteFile( hfile, data, len, &c, NULL );
	CloseHandle( hfile );

	/* load the temporary BASS.DLL library */
	if ( !(bass = LoadLibrary( tempfile)) )
	{
#ifndef CGAME
		Com_Printf( "Error: Can't load BASS.DLL\n" );
#else
		CG_Printf( "Error: Can't load BASS.DLL\n" );
#endif
		return ( qfalse );
	}

/* "load" all the BASS functions that are to be used */
#define LOADBASSFUNCTION( f )	*( (void **) &f ) = GetProcAddress( bass, #f )
	LOADBASSFUNCTION( BASS_ErrorGetCode );
	LOADBASSFUNCTION( BASS_Init );
	LOADBASSFUNCTION( BASS_Free );
	LOADBASSFUNCTION( BASS_GetCPU );
	LOADBASSFUNCTION( BASS_StreamCreateFile );
	LOADBASSFUNCTION( BASS_StreamCreateURL );
	LOADBASSFUNCTION( BASS_GetDeviceInfo );
	LOADBASSFUNCTION( BASS_Set3DFactors );
	LOADBASSFUNCTION( BASS_Set3DPosition );
	LOADBASSFUNCTION( BASS_SetEAXParameters );
	LOADBASSFUNCTION( BASS_Start );
	LOADBASSFUNCTION( BASS_ChannelSetAttribute );
	LOADBASSFUNCTION( BASS_ChannelSet3DPosition );
	LOADBASSFUNCTION( BASS_ChannelSet3DAttributes );
	LOADBASSFUNCTION( BASS_Apply3D );
	LOADBASSFUNCTION( BASS_ChannelStop );
	LOADBASSFUNCTION( BASS_SampleLoad );
	LOADBASSFUNCTION( BASS_SampleGetChannel );
	LOADBASSFUNCTION( BASS_Set3DPosition );
	LOADBASSFUNCTION( BASS_Set3DPosition );
	LOADBASSFUNCTION( BASS_Set3DPosition );
	LOADBASSFUNCTION( BASS_StreamGetFilePosition );
	LOADBASSFUNCTION( BASS_ChannelPlay );
	LOADBASSFUNCTION( BASS_ChannelBytes2Seconds );
	LOADBASSFUNCTION( BASS_ChannelIsActive );
	LOADBASSFUNCTION( BASS_ChannelIsSliding );
	LOADBASSFUNCTION( BASS_ChannelGetPosition );
	LOADBASSFUNCTION( BASS_ChannelGetLevel );
	LOADBASSFUNCTION( BASS_ChannelSetSync );
	LOADBASSFUNCTION( BASS_GetVersion );
	LOADBASSFUNCTION( BASS_ErrorGetCode );
	LOADBASSFUNCTION( BASS_MusicLoad );
	LOADBASSFUNCTION( BASS_ChannelBytes2Seconds );
	LOADBASSFUNCTION( BASS_ChannelSetSync );
	LOADBASSFUNCTION( BASS_ChannelPlay );
	LOADBASSFUNCTION( BASS_StreamFree );
	LOADBASSFUNCTION( BASS_ChannelIsActive );
	LOADBASSFUNCTION( BASS_ChannelGetLevel );
	LOADBASSFUNCTION( BASS_ChannelGetPosition );
	LOADBASSFUNCTION( BASS_ChannelGetLength );
	LOADBASSFUNCTION( BASS_ChannelGetData );
	LOADBASSFUNCTION( BASS_ChannelGetTags );
	LOADBASSFUNCTION( BASS_SetVolume ); // To set global volume.
	LOADBASSFUNCTION( BASS_ChannelSetAttribute ); // Lets me set chanel volume separetly.

	return ( qtrue );
}
#endif

qboolean BASS_Initialize ( void )
{
#if 0
	if (!LoadBASS()) Com_Error(ERR_FATAL, "Unable to load BASS sound library.\n");
#endif

	EAX_SUPPORTED = qfalse;

	// check the correct BASS was loaded
	if (HIWORD(BASS_GetVersion()) != BASSVERSION) {
		Com_Printf("An incorrect version of BASS.DLL was loaded.");
		return qfalse;
	}

	HWND	win = 0;
	DWORD	freq = 44100;

	if (s_khz->integer == 96)
		freq = 96000;
	else if (s_khz->integer == 48)
		freq = 48000;
	else if (s_khz->integer == 44)
		freq = 44100;
	else if (s_khz->integer == 22)
		freq = 22050;
	else
		freq = 11025;

	Com_Printf("^3BASS Sound System Initializing...\n");

	// Initialize the default output device with 3D support
	if (!BASS_Init(-1, freq, BASS_DEVICE_3D, win, NULL)) {
		Com_Printf("^3- ^5BASS could not find a sound device.");
		Com_Printf("^3BASS Sound System Initialization ^1FAILED^7!\n");
		return qfalse;
	}

	Com_Printf("^5BASS Selected Device:\n");
	BASS_DEVICEINFO info;
	BASS_GetDeviceInfo(BASS_GetDevice(), &info);
	Com_Printf("^3%s^5.\n", info.name);

	// Use meters as distance unit, real world rolloff, real doppler effect
	BASS_Set3DFactors(SOUND_DISTANCE_UNIT_SIZE, SOUND_REAL_WORLD_FALLOFF, SOUND_REAL_WORLD_DOPPLER);

	// Turn EAX off (volume=0), if error then EAX is not supported
	if (!BASS_SetEAXParameters(-1,0,-1,-1))
	{
		Com_Printf("^3+ ^5EAX Features Supported.\n");
		EAX_SUPPORTED = qtrue;
		BASS_SetEAX_NORMAL();
	}
	else
	{
		Com_Printf("^1- ^5EAX Features NOT Supported.\n");
		EAX_SUPPORTED = qfalse;
	}

	BASS_Start();

	BASS_SetConfig(BASS_CONFIG_GVOL_MUSIC, (DWORD)(float)(s_volume->value*10000.0));
	BASS_SetConfig(BASS_CONFIG_GVOL_SAMPLE, (DWORD)(float)(s_volume->value*10000.0));
	BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, (DWORD)(float)(s_volume->value*10000.0));

	BASS_SetConfig(BASS_CONFIG_UPDATETHREADS, (DWORD)16);
	BASS_SetConfig(BASS_CONFIG_BUFFER, (DWORD)100); // set the buffer length

	//Com_Printf("Volume %f. Sample Volume %i. Stream Volume %i.\n", BASS_GetVolume(), (int)BASS_GetConfig(BASS_CONFIG_GVOL_SAMPLE), (int)BASS_GetConfig(BASS_CONFIG_GVOL_STREAM));

	// Try to use the WDM full 3D algorythm...
	if (BASS_SetConfig(BASS_CONFIG_3DALGORITHM, BASS_3DALG_FULL))
	{
		Com_Printf("^3+ ^5Enhanced Surround Enabled.\n");
	}
	else
	{
		BASS_SetConfig(BASS_CONFIG_3DALGORITHM, BASS_3DALG_DEFAULT);
		Com_Printf("^1- ^5Default Surround Enabled. You need a WDM sound driver to use Enhanced Surround.\n");
	}

	// Set view angles and position to 0,0,0. We will rotate the sound angles...
	vec3_t forward, right, up;
	BASS_3DVECTOR pos, ang, top, vel;

	vel.x = cl.snap.ps.velocity[0];
	vel.y = cl.snap.ps.velocity[1];
	vel.z = -cl.snap.ps.velocity[2];

	pos.x = cl.snap.ps.origin[0];
	pos.y = cl.snap.ps.origin[1];
	pos.z = cl.snap.ps.origin[2];
	
	AngleVectors(cl.snap.ps.viewangles, forward, right, up);

	ang.x = forward[0];
	ang.y = forward[1];
	ang.z = -forward[2];

	top.x = up[0];
	top.y = up[1];
	top.z = -up[2];

	BASS_Set3DPosition(&pos, &vel, &ang, &top);

	Com_Printf("^3BASS Sound System initialized ^7OK^3!\n");

	BASS_UPDATE_THREAD_STOP = qfalse;

	// Initialize all the sound channels ready for use...
	BASS_InitializeChannels();

	//BASS_SetConfig(BASS_CONFIG_CURVE_VOL, true);

	// UQ1: Play a BASS startup sound...
	//BASS_AddStreamChannel ( "OJK/sound/startup.wav", -1, s_volume->value, NULL );

	s_soundStarted = 1;
	s_soundtime = 0;
	s_numSfx = 0;
	S_BeginRegistration();

	return qtrue;
}


//
// Position Utils...
//

float BASS_GetVolumeForChannel ( int entchannel )
{
	float		volume = 1;
	float		normal_vol, voice_vol, effects_vol, ambient_vol, weapon_vol, item_vol, body_vol, music_vol, local_vol;

	normal_vol		= s_volume->value;
	voice_vol		= (s_volumeVoice->value*normal_vol);
	ambient_vol		= (s_volumeAmbient->value*normal_vol);
	music_vol		= (s_volumeMusic->value*normal_vol);
	local_vol		= (s_volumeLocal->value*normal_vol);

	effects_vol		= (s_volumeEffects->value*normal_vol);
	weapon_vol		= (s_volumeWeapon->value*effects_vol);
	item_vol		= (s_volumeItem->value*effects_vol);
	body_vol		= (s_volumeBody->value*effects_vol);

	if ( entchannel == CHAN_VOICE || entchannel == CHAN_VOICE_ATTEN )
	{
		volume = voice_vol;
	}
	else if (entchannel == CHAN_WEAPON)
	{
		volume = weapon_vol;
	}
	else if (entchannel == CHAN_ITEM)
	{
		volume = item_vol;
	}
	else if (entchannel == CHAN_BODY)
	{
		volume = body_vol;
	}
	else if ( entchannel == CHAN_LESS_ATTEN || entchannel == CHAN_AUTO)
	{
		volume = effects_vol;
	}
	else if ( entchannel == CHAN_AMBIENT )
	{
		volume = ambient_vol;
	}
	else if ( entchannel == CHAN_MUSIC )
	{
		volume = music_vol;
	}
	else //CHAN_LOCAL //CHAN_ANNOUNCER //CHAN_LOCAL_SOUND //CHAN_MENU1 //CHAN_VOICE_GLOBAL
	{
		volume = local_vol;
	}

	return volume;
}

void BASS_UpdatePosition ( int ch, qboolean IS_NEW_SOUND )
{// Update this channel's position, etc...
	qboolean	IS_LOCAL_SOUND = qfalse;
	Channel		*c = &SOUND_CHANNELS[ch];
	int			SOUND_ENTITY = -1;
	float		CHAN_VOLUME = 0.0;
	vec3_t		porg, corg;

	if (!c) return; // should be impossible, but just in case...
	if (!IS_NEW_SOUND && !c->isLooping) return; // We don't even need to update do we???

	SOUND_ENTITY = c->entityNum;
	CHAN_VOLUME = c->volume*BASS_GetVolumeForChannel(c->entityChannel);

	if (IS_NEW_SOUND && !(c->origin[0] == 0 && c->origin[1] == 0 && c->origin[2] == 0))
	{// New sound with an origin... Use the specified origin...

	}
	else if (SOUND_ENTITY == -1)
	{// Either a local sound, or we hopefully already have an origin...

	}
	else if (!c->isLooping
		&& !(s_entityPosition[SOUND_ENTITY][0] == 0 
		&& s_entityPosition[SOUND_ENTITY][1] == 0 
		&& s_entityPosition[SOUND_ENTITY][2] == 0))
	{// UPDATE POSITION - Primary - use s_entityPosition if we have one...
		VectorCopy(s_entityPosition[SOUND_ENTITY], c->origin);
	}
	else if (!c->isLooping
		&& !(cl.entityBaselines[SOUND_ENTITY].origin[0] == 0 
		&& cl.entityBaselines[SOUND_ENTITY].origin[1] == 0 
		&& cl.entityBaselines[SOUND_ENTITY].origin[2] == 0))
	{// UPDATE POSITION - Backup - use entity baseline origins...
		VectorCopy(cl.entityBaselines[SOUND_ENTITY].origin, c->origin);
	}
	
	if (c->origin[0] == 0 
		&& c->origin[1] == 0 
		&& c->origin[2] == 0)
	{// Must be a local sound...
		VectorSet(c->origin, 0, 0, 0);
		IS_LOCAL_SOUND = qtrue;
	}

	c->vel.x = 0;
	c->vel.y = 0;
	c->vel.z = 0;

#ifndef __BASS_PLAYER_BASED_LOCATIONS__
	VectorCopy(cl.snap.ps.origin, porg);
#else //__BASS_PLAYER_BASED_LOCATIONS__
	VectorSet(porg, 0, 0, 0);
#endif //__BASS_PLAYER_BASED_LOCATIONS__
	//porg[0] /= 1000;
	//porg[1] /= 1000;
	//porg[2] /= 1000;

	if (IS_LOCAL_SOUND)
	{
		// Set origin...
		c->pos.x = porg[0];
		c->pos.y = porg[1];
		c->pos.z = porg[2];

		BASS_ChannelSet3DPosition(c->channel, &c->pos, NULL, &c->vel);

		//if (!BASS_ChannelIsSliding(c->channel, BASS_ATTRIB_VOL))
		{
			BASS_ChannelSet3DAttributes(c->channel, SOUND_3D_METHOD, -1, -1, -1, -1, -1);
			BASS_ChannelSetAttribute(c->channel, BASS_ATTRIB_VOL, CHAN_VOLUME);
			//if (CHAN_VOLUME > 0) Com_Printf("LOCAL Volume %f.\n", CHAN_VOLUME);
		}
	}
	else
	{
#ifndef __BASS_PLAYER_BASED_LOCATIONS__
		VectorCopy(c->origin, corg);
#else //__BASS_PLAYER_BASED_LOCATIONS__
		VectorSubtract(c->origin, cl.snap.ps.origin, corg);
#endif //__BASS_PLAYER_BASED_LOCATIONS__
		//corg[0] /= 1000;
		//corg[1] /= 1000;
		//corg[2] /= 1000;

		// Set origin...
		c->pos.x = corg[0];
		c->pos.y = corg[1];
		c->pos.z = corg[2];

		//Com_Printf("SOUND: Pos %f %f %f. Vel %f %f %f.\n", c->pos.x, c->pos.y, c->pos.z, c->vel.x, c->vel.y, c->vel.z);

		BASS_ChannelSet3DPosition(c->channel, &c->pos, NULL, &c->vel);

		//if (!BASS_ChannelIsSliding(c->channel, BASS_ATTRIB_VOL))
		{
			BASS_ChannelSet3DAttributes(c->channel, SOUND_3D_METHOD, MIN_SOUND_RANGE, MAX_SOUND_RANGE, SOUND_CONE_INSIDE_ANGLE, SOUND_CONE_OUTSIDE_ANGLE, SOUND_CONE_OUTSIDE_VOLUME*CHAN_VOLUME);
			BASS_ChannelSetAttribute(c->channel, BASS_ATTRIB_VOL, CHAN_VOLUME);
			BASS_ChannelFlags(c->channel, BASS_SAMPLE_MUTEMAX, BASS_SAMPLE_MUTEMAX); // enable muting at the max distance
			//if (CHAN_VOLUME > 0) Com_Printf("3D Volume %f.\n", CHAN_VOLUME);
		}
	}
}

void BASS_ProcessStartRequest( int channel )
{
	Channel *c = &SOUND_CHANNELS[channel];

	DWORD count = BASS_SampleGetChannels(c->originalChannel, NULL);
	if (count == -1) 
	{// fail to find a channel...
		c->startRequest = qfalse;
		c->isLooping = qfalse;
		c->isActive = qfalse;
		return;
	}

	//Com_Printf("Channel %i. Count: %i. Looping %i. OutChan %i. Volume %f.\n", c->entityChannel, count, (int)c->isLooping, c->channel, c->volume);

	c->channel = BASS_SampleGetChannel(c->originalChannel,FALSE); // initialize sample channel
	
	//BASS_ChannelSlideAttribute(c->channel, BASS_ATTRIB_VOL, c->volume*BASS_GetVolumeForChannel(c->entityChannel), 1000); // fade-in over 100ms

	// Apply the 3D changes
	BASS_UpdatePosition(channel, qtrue);
	BASS_Apply3D();

	// Play
	if (c->isLooping)
		BASS_ChannelPlay(c->channel,TRUE);
	else
		BASS_ChannelPlay(c->channel,FALSE);

	c->startRequest = qfalse;
}

void BASS_UpdateSounds_REAL ( void )
{
	//int NUM_ACTIVE = 0;
	//int NUM_FREE = 0;

	vec3_t forward, right, up, porg;
	BASS_3DVECTOR pos, ang, top, vel;

#ifndef __BASS_PLAYER_BASED_LOCATIONS__
	VectorCopy(cl.snap.ps.origin, porg);
#else //__BASS_PLAYER_BASED_LOCATIONS__
	VectorSet(porg, 0, 0, 0);
#endif //__BASS_PLAYER_BASED_LOCATIONS__
	//porg[0] /= 1000;
	//porg[1] /= 1000;
	//porg[2] /= 1000;

	vel.x = cl.snap.ps.velocity[0];
	vel.y = cl.snap.ps.velocity[1];
	vel.z = cl.snap.ps.velocity[2]*-1.0;

	pos.x = porg[0];
	pos.y = porg[1];
	pos.z = porg[2];
	
	AngleVectors(cl.snap.ps.viewangles, forward, right, up);
	
	//VectorNormalize(forward);
	//VectorNormalize(up);

	ang.x = forward[0];
	ang.y = forward[1];
	ang.z = forward[2]*-1.0;

	top.x = 0;//0-up[0];
	top.y = 1;//0-up[1];
	top.z = 0;//up[2];

	BASS_Set3DPosition(&pos, &vel, &ang, &top);

	//Com_Printf("PL: Pos %f %f %f. Ang %f %f %f. Top %f %f %f. Vel %f %f %f.\n", pos.x, pos.y, pos.z, ang.x, ang.y, ang.z, top.x, top.y, top.z, vel.x, vel.y, vel.z);

	BASS_ChannelSetAttribute(MUSIC_CHANNEL.channel, BASS_ATTRIB_VOL, MUSIC_CHANNEL.volume*BASS_GetVolumeForChannel(CHAN_MUSIC));

	for (int c = 0; c < MAX_BASS_CHANNELS; c++) 
	{
		if (SOUND_CHANNELS[c].startRequest)
		{// Start any channels that have been requested...
			BASS_ProcessStartRequest( c );
			//NUM_ACTIVE++;
		}
		else if (SOUND_CHANNELS[c].isActive && BASS_ChannelIsActive(SOUND_CHANNELS[c].channel) == BASS_ACTIVE_PLAYING) 
		{// If the channel's playing then update it's position
			BASS_UpdatePosition(c, qfalse);
			//NUM_ACTIVE++;
		}
		else
		{// Finished. Remove the channel...
			if (SOUND_CHANNELS[c].isActive)
			{// Still marked as active. Stop the channel and reset it...
				//Com_Printf("Removing inactive channel %i.\n", c);
				BASS_StopChannel(c);
				//BASS_ChannelSetAttribute(SOUND_CHANNELS[c].channel, BASS_ATTRIB_VOL, 0.0);
				if (SOUND_CHANNELS[c].channel != SOUND_CHANNELS[c].originalChannel) 
				{// free the copied channel's sample memory...
					BASS_SampleFree(SOUND_CHANNELS[c].channel);
				}
				memset(&SOUND_CHANNELS[c],0,sizeof(Channel));
				//NUM_FREE++;
			}
		}
	}

	// Apply the 3D changes.
	BASS_Apply3D();

	//Com_Printf("There are currently %i active and %i free channels.\n", NUM_ACTIVE, NUM_FREE);
}

void BASS_UpdateThread(void * aArg)
{
	while (!BASS_UPDATE_THREAD_STOP)
	{
		BASS_UpdateSounds_REAL();
		this_thread::sleep_for(chrono::milliseconds(10));
	}

	BASS_UPDATE_THREAD_RUNNING = qfalse;
}

void BASS_UpdateSounds ( void )
{
	if (thread::hardware_concurrency() <= 1)
	{// Only one CPU core to use. Don't thread...
		BASS_UpdateSounds_REAL();
		return;
	}

	if (!BASS_UPDATE_THREAD_RUNNING)
	{
		BASS_UPDATE_THREAD_RUNNING = qtrue;
		BASS_UPDATE_THREAD = new thread (BASS_UpdateThread, 0);
	}
}

//
// Effects...
//

/*
// EAX presets, usage: BASS_SetEAXParameters(EAX_PRESET_xxx)
#define EAX_PRESET_GENERIC         EAX_ENVIRONMENT_GENERIC,0.5F,1.493F,0.5F
#define EAX_PRESET_PADDEDCELL      EAX_ENVIRONMENT_PADDEDCELL,0.25F,0.1F,0.0F
#define EAX_PRESET_ROOM            EAX_ENVIRONMENT_ROOM,0.417F,0.4F,0.666F
#define EAX_PRESET_BATHROOM        EAX_ENVIRONMENT_BATHROOM,0.653F,1.499F,0.166F
#define EAX_PRESET_LIVINGROOM      EAX_ENVIRONMENT_LIVINGROOM,0.208F,0.478F,0.0F
#define EAX_PRESET_STONEROOM       EAX_ENVIRONMENT_STONEROOM,0.5F,2.309F,0.888F
#define EAX_PRESET_AUDITORIUM      EAX_ENVIRONMENT_AUDITORIUM,0.403F,4.279F,0.5F
#define EAX_PRESET_CONCERTHALL     EAX_ENVIRONMENT_CONCERTHALL,0.5F,3.961F,0.5F
#define EAX_PRESET_CAVE            EAX_ENVIRONMENT_CAVE,0.5F,2.886F,1.304F
#define EAX_PRESET_ARENA           EAX_ENVIRONMENT_ARENA,0.361F,7.284F,0.332F
#define EAX_PRESET_HANGAR          EAX_ENVIRONMENT_HANGAR,0.5F,10.0F,0.3F
#define EAX_PRESET_CARPETEDHALLWAY EAX_ENVIRONMENT_CARPETEDHALLWAY,0.153F,0.259F,2.0F
#define EAX_PRESET_HALLWAY         EAX_ENVIRONMENT_HALLWAY,0.361F,1.493F,0.0F
#define EAX_PRESET_STONECORRIDOR   EAX_ENVIRONMENT_STONECORRIDOR,0.444F,2.697F,0.638F
#define EAX_PRESET_ALLEY           EAX_ENVIRONMENT_ALLEY,0.25F,1.752F,0.776F
#define EAX_PRESET_FOREST          EAX_ENVIRONMENT_FOREST,0.111F,3.145F,0.472F
#define EAX_PRESET_CITY            EAX_ENVIRONMENT_CITY,0.111F,2.767F,0.224F
#define EAX_PRESET_MOUNTAINS       EAX_ENVIRONMENT_MOUNTAINS,0.194F,7.841F,0.472F
#define EAX_PRESET_QUARRY          EAX_ENVIRONMENT_QUARRY,1.0F,1.499F,0.5F
#define EAX_PRESET_PLAIN           EAX_ENVIRONMENT_PLAIN,0.097F,2.767F,0.224F
#define EAX_PRESET_PARKINGLOT      EAX_ENVIRONMENT_PARKINGLOT,0.208F,1.652F,1.5F
#define EAX_PRESET_SEWERPIPE       EAX_ENVIRONMENT_SEWERPIPE,0.652F,2.886F,0.25F
#define EAX_PRESET_UNDERWATER      EAX_ENVIRONMENT_UNDERWATER,1.0F,1.499F,0.0F
#define EAX_PRESET_DRUGGED         EAX_ENVIRONMENT_DRUGGED,0.875F,8.392F,1.388F
#define EAX_PRESET_DIZZY           EAX_ENVIRONMENT_DIZZY,0.139F,17.234F,0.666F
#define EAX_PRESET_PSYCHOTIC       EAX_ENVIRONMENT_PSYCHOTIC,0.486F,7.563F,0.806F
	*/

void BASS_SetEAX_NORMAL ( void )
{
	if (!EAX_SUPPORTED) return;

	BASS_SetEAXParameters(EAX_PRESET_GENERIC);
}

void BASS_SetEAX_UNDERWATER ( void )
{
	if (!EAX_SUPPORTED) return;

	BASS_SetEAXParameters(EAX_PRESET_UNDERWATER);
}

void BASS_SetRolloffFactor ( int factor )
{
	BASS_Set3DFactors(-1,pow(2,(factor-10)/5.0),-1);
}

void BASS_SetDopplerFactor ( int factor )
{
	BASS_Set3DFactors(-1,-1,pow(2,(factor-10)/5.0));
}

//
// Music Tracks...
//

void BASS_StopMusic( DWORD samplechan )
{
	// Free old samples...
	BASS_ChannelStop(MUSIC_CHANNEL.channel);
	BASS_SampleFree(MUSIC_CHANNEL.channel);
	BASS_MusicFree(MUSIC_CHANNEL.channel);
	BASS_StreamFree(MUSIC_CHANNEL.channel);
}

void BASS_StartMusic ( DWORD samplechan )
{
	//if (s_volumeMusic->value <= 0) return;

	// Set new samples...
	MUSIC_CHANNEL.originalChannel=MUSIC_CHANNEL.channel = samplechan;
	MUSIC_CHANNEL.entityNum = -1;
	MUSIC_CHANNEL.entityChannel = CHAN_MUSIC;
	MUSIC_CHANNEL.volume = 1.0;

	BASS_SampleGetChannel(samplechan,FALSE); // initialize sample channel
	BASS_ChannelSetAttribute(samplechan, BASS_ATTRIB_VOL, MUSIC_CHANNEL.volume*BASS_GetVolumeForChannel(CHAN_MUSIC));

	// Play
	BASS_ChannelPlay(samplechan,TRUE);

	// Apply the 3D settings (music is always local)...
	MUSIC_CHANNEL.vel.x = 0;
	MUSIC_CHANNEL.vel.y = 0;
	MUSIC_CHANNEL.vel.z = 0;

	// Set origin...
	MUSIC_CHANNEL.pos.x = 0;
	MUSIC_CHANNEL.pos.y = 0;
	MUSIC_CHANNEL.pos.z = 0;

	BASS_ChannelSet3DPosition(MUSIC_CHANNEL.channel, &MUSIC_CHANNEL.pos, NULL, &MUSIC_CHANNEL.vel);
	BASS_ChannelSet3DAttributes(MUSIC_CHANNEL.channel, SOUND_3D_METHOD, -1, -1, -1, -1, -1);
	BASS_Apply3D();
}

DWORD BASS_LoadMusicSample ( void *memory, int length )
{// Just load a sample into memory ready to play instantly...
	DWORD newchan;

	//if (s_volumeMusic->value <= 0) return -1;

	// Try to load the sample with the highest quality options we support...
	if (newchan=BASS_SampleLoad(TRUE,memory,0,(DWORD)length,1,BASS_SAMPLE_LOOP|BASS_SAMPLE_FLOAT))
	{
		return newchan;
	}
	else if (newchan=BASS_SampleLoad(TRUE,memory,0,(DWORD)length,1,BASS_SAMPLE_LOOP))
	{
		return newchan;
	}

	return -1;
}


//
// Sounds...
//

void BASS_AddStreamChannel ( char *file, int entityNum, int entityChannel, vec3_t origin )
{
#if 0
	DWORD newchan;
	int chan = BASS_FindFreeChannel();

	if (chan < 0)
	{// No channel left to play on...
		Com_Printf("BASS: No free sound channels.\n");
		return;
	}

	// Load a music or sample from "file"
	if (newchan=BASS_SampleLoad(FALSE,file,0,0,1,BASS_SAMPLE_3D|BASS_SAMPLE_MONO)) {
			Channel *c = SOUND_CHANNELS[chan];
			memset(c,0,sizeof(Channel));
			c->originalChannel=c->channel=newchan;
			c->entityNum = entityNum;
			c->entityChannel = entityChannel;
			c->isActive = qtrue;
			c->isLooping = qfalse;
			c->volume = 1.0;

			BASS_SampleGetChannel(newchan,FALSE); // initialize sample channel

			// Play
			BASS_ChannelPlay(newchan,FALSE);

			// Apply the 3D changes
			BASS_SetPosition( chan, origin );
			BASS_ChannelSet3DAttributes(newchan, SOUND_3D_METHOD, 256.0, -1, -1, -1, -1);//120, 120, 0.5);
			BASS_Apply3D();
	} else {
		Com_Printf("Can't load file (note samples must be mono)\n");
	}
#endif
}

void BASS_AddMemoryChannel ( DWORD samplechan, int entityNum, int entityChannel, vec3_t origin, float volume )
{
	int chan = BASS_FindFreeChannel();

	if (chan < 0)
	{// No channel left to play on...
		Com_Printf("BASS: No free sound channels.\n");
		return;
	}

	// Load a music or sample from "file" (memory)
	Channel *c = &SOUND_CHANNELS[chan];
	c->originalChannel=c->channel=samplechan;
	c->entityNum = entityNum;
	c->entityChannel = entityChannel;
	c->volume = volume;

	if (origin) VectorCopy(origin, c->origin);
	else VectorSet(c->origin, 0, 0, 0);

	c->isActive = qtrue;
	c->isLooping = qfalse;
	c->startRequest = qtrue;
}

void BASS_AddMemoryLoopChannel ( DWORD samplechan, int entityNum, int entityChannel, vec3_t origin, float volume )
{
	//
	// UQ1: Since it seems these also re-call this function to update positions, etc, run a check first...
	//
	if (origin)
	{// If there's no origin, surely this can't be an update...
		for (int ch = 0; ch < MAX_BASS_CHANNELS; ch++) 
		{
			if (SOUND_CHANNELS[ch].isActive && SOUND_CHANNELS[ch].isLooping)
			{// This is active and looping...
				if (SOUND_CHANNELS[ch].entityChannel == entityChannel 
					&& SOUND_CHANNELS[ch].entityNum == entityNum 
					&& SOUND_CHANNELS[ch].originalChannel == samplechan)
				{// This is our sound! Just update it (and then return)...
					Channel *c = &SOUND_CHANNELS[ch];
					VectorCopy(origin, c->origin);
					c->volume = volume;
					//Com_Printf("BASS DEBUG: Sound position (%f %f %f) and volume (%f) updated.\n", origin[0], origin[1], origin[2], volume);
					return;
				}
			}
		}
	}

	int chan = BASS_FindFreeChannel();

	if (chan < 0)
	{// No channel left to play on...
		Com_Printf("BASS: No free sound channels.\n");
		return;
	}

	// Load a music or sample from "file" (memory)
	Channel *c = &SOUND_CHANNELS[chan];
	c->originalChannel=c->channel=samplechan;
	c->entityNum = entityNum;
	c->entityChannel = entityChannel;
	c->volume = volume;

	if (origin) VectorCopy(origin, c->origin);
	else VectorSet(c->origin, 0, 0, 0);

	c->isActive = qtrue;
	c->isLooping = qtrue;
	c->startRequest = qtrue;
}

DWORD BASS_LoadMemorySample ( void *memory, int length )
{// Just load a sample into memory ready to play instantly...
	DWORD newchan;

	// Try to load the sample with the highest quality options we support...
	if ((newchan=BASS_SampleLoad(TRUE,memory,0,(DWORD)length,(DWORD)16,BASS_SAMPLE_3D|BASS_SAMPLE_MONO|BASS_SAMPLE_FLOAT|BASS_SAMPLE_VAM|BASS_SAMPLE_OVER_DIST)))
	{
		return newchan;
	}
	else if ((newchan=BASS_SampleLoad(TRUE,memory,0,(DWORD)length,(DWORD)16,BASS_SAMPLE_3D|BASS_SAMPLE_MONO|BASS_SAMPLE_VAM|BASS_SAMPLE_OVER_DIST)))
	{
		return newchan;
	}

	return -1;
}

#endif //__USE_BASS__
