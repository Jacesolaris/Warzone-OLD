#include "client.h"
#include "snd_local.h"

#ifdef __USE_BASS__

#include "snd_bass.h"

extern cvar_t		*s_khz;
extern vec3_t		s_entityPosition[MAX_GENTITIES];

extern float S_GetVolumeForChannel ( int entchannel );

qboolean EAX_SUPPORTED = qtrue;

// channel (sample/music) info structure
typedef struct {
	DWORD			channel;			// the channel
	BASS_3DVECTOR	pos, vel, ang;	// position,velocity,angles
	vec3_t			origin;
	int				entityNum;
	int				entityChannel;
	qboolean		isActive;
} Channel;

//Channel *chans=NULL;		// the channels
//int chanc=0,chan=-1;		// number of channels, current channel

qboolean	MUSIC_CHANNEL_INITIALIZED = qfalse;
Channel		*MUSIC_CHANNEL;

qboolean	SOUND_CHANNELS_INITIALIZED = qfalse;
Channel		*SOUND_CHANNELS[MAX_CHANNELS];

#define TIMERPERIOD	50		// timer period (ms)
#define MAXDIST		2048//50		// maximum distance of the channels (m)
#define SPEED		12		// speed of the channels' movement (m/s)

//
// Channel Utils...
//

void BASS_InitializeChannels ( void )
{
	if (!SOUND_CHANNELS_INITIALIZED)
	{
		for (int c = 0; c < MAX_CHANNELS; c++) 
		{// Set up this channel...
			SOUND_CHANNELS[c]=(Channel*)malloc(sizeof(Channel));
			memset(SOUND_CHANNELS[c],0,sizeof(Channel));
		}

		SOUND_CHANNELS_INITIALIZED = qtrue;
	}
}

void BASS_StopChannel ( int chanNum )
{
	if (BASS_ChannelIsActive(SOUND_CHANNELS[chanNum]->channel) == BASS_ACTIVE_PLAYING)
	{
		BASS_ChannelStop(SOUND_CHANNELS[chanNum]->channel);
	}

	//BASS_SampleFree(SOUND_CHANNELS[chanNum]->channel);
	//BASS_MusicFree(SOUND_CHANNELS[chanNum]->channel);
	//BASS_StreamFree(SOUND_CHANNELS[chanNum]->channel);
	SOUND_CHANNELS[chanNum]->isActive = qfalse;
}

int BASS_FindFreeChannel ( void )
{
	for (int c = 0; c < MAX_CHANNELS; c++) 
	{
		if (!SOUND_CHANNELS[c]->isActive)
		{
			return c;
		}
	}

	return -1;
}

void BASS_SetEAX ( void )
{
	/*
	EM(CB_ADDSTRING,0,"Off");
	EM(CB_ADDSTRING,0,"Generic");
	EM(CB_ADDSTRING,0,"Padded Cell");
	EM(CB_ADDSTRING,0,"Room");
	EM(CB_ADDSTRING,0,"Bathroom");
	EM(CB_ADDSTRING,0,"Living Room");
	EM(CB_ADDSTRING,0,"Stone Room");
	EM(CB_ADDSTRING,0,"Auditorium");
	EM(CB_ADDSTRING,0,"Concert Hall");
	EM(CB_ADDSTRING,0,"Cave");
	EM(CB_ADDSTRING,0,"Arena");
	EM(CB_ADDSTRING,0,"Hangar");
	EM(CB_ADDSTRING,0,"Carpeted Hallway");
	EM(CB_ADDSTRING,0,"Hallway");
	EM(CB_ADDSTRING,0,"Stone Corridor");
	EM(CB_ADDSTRING,0,"Alley");
	EM(CB_ADDSTRING,0,"Forest");
	EM(CB_ADDSTRING,0,"City");
	EM(CB_ADDSTRING,0,"Mountains");
	EM(CB_ADDSTRING,0,"Quarry");
	EM(CB_ADDSTRING,0,"Plain");
	EM(CB_ADDSTRING,0,"Parking Lot");
	EM(CB_ADDSTRING,0,"Sewer Pipe");
	EM(CB_ADDSTRING,0,"Under Water");
	EM(CB_ADDSTRING,0,"Drugged");
	EM(CB_ADDSTRING,0,"Dizzy");
	EM(CB_ADDSTRING,0,"Psychotic");
	*/

	//BASS_SetEAXParameters(-1,0,-1,-1); // off (volume=0)
	//BASS_SetEAXParameters(s-1,-1,-1,-1);
}

void BASS_SetRolloffFactor ( int factor )
{
	BASS_Set3DFactors(-1,pow(2,(factor-10)/5.0),-1);
}

void BASS_SetDopplerFactor ( int factor )
{
	BASS_Set3DFactors(-1,-1,pow(2,(factor-10)/5.0));
}

HINSTANCE			bass = 0;								// bass handle
char				tempfile[MAX_PATH];						// temporary BASS.DLL

void BASS_Shutdown ( void )
{
	if (SOUND_CHANNELS_INITIALIZED)
	{
		for (int c = 0; c < MAX_CHANNELS; c++) 
		{// Free channel...
			BASS_StopChannel(c);
			//BASS_SampleFree(SOUND_CHANNELS[c]->channel);
			//BASS_MusicFree(SOUND_CHANNELS[c]->channel);
			//BASS_StreamFree(SOUND_CHANNELS[c]->channel);
			free(SOUND_CHANNELS[c]);
		}

		SOUND_CHANNELS_INITIALIZED = qfalse;
	}

	BASS_Free();
}

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

qboolean BASS_Initialize ( void )
{
	//if (!LoadBASS()) Com_Error(ERR_FATAL, "Unable to load BASS sound library.\n");

	EAX_SUPPORTED = qtrue;

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

	// Initialize the default output device with 3D support
	if (!BASS_Init(-1, freq, BASS_DEVICE_3D, win, NULL)) {
		Com_Printf("Can't initialize output device");
		return qfalse;
	}

	BASS_DEVICEINFO info;
	BASS_GetDeviceInfo(BASS_GetDevice(), &info);
	Com_Printf("Initialized sound device %s.\n", info.name);

	// Use meters as distance unit, real world rolloff, real doppler effect
	//BASS_Set3DFactors(1,1,1);
	BASS_Set3DFactors(0.0,0.0,0.0);
	//BASS_SetRolloffFactor(0);

	// Turn EAX off (volume=0), if error then EAX is not supported
	//if (!BASS_SetEAXParameters(-1,/*0*/s_volume->value,-1,-1))
	if (!BASS_SetEAXParameters(-1,0,-1,-1))
	{
		Com_Printf("EAX Supported.\n");
		EAX_SUPPORTED = qtrue;
	}
	else
	{
		Com_Printf("EAX NOT Supported.\n");
		EAX_SUPPORTED = qfalse;
	}

	BASS_Start();

	BASS_SetConfig(BASS_CONFIG_GVOL_MUSIC, (DWORD)(float)(s_volume->value*10000.0));
	BASS_SetConfig(BASS_CONFIG_GVOL_SAMPLE, (DWORD)(float)(s_volume->value*10000.0));
	BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, (DWORD)(float)(s_volume->value*10000.0));

	//Com_Printf("Volume %f. Sample Volume %i. Stream Volume %i.\n", BASS_GetVolume(), (int)BASS_GetConfig(BASS_CONFIG_GVOL_SAMPLE), (int)BASS_GetConfig(BASS_CONFIG_GVOL_STREAM));

	// Initialize all the sound channels ready for use...
	BASS_InitializeChannels();

	Com_Printf("\n\n>>> BASS Sound System initialized OK! <<<\n\n\n");

	// UQ1: Play a BASS startup sound...
	//BASS_AddStreamChannel ( "OJK/sound/startup.wav", -1, s_volume->value, NULL );

	return qtrue;
}

//
// Position Utils...
//

void BASS_SetPosition ( int c, vec3_t origin )
{// If the channel's playing then update it's position
	vec3_t soundPos, soundAng, forward, right, up;
	BASS_3DVECTOR pos, ang, top, vel;

	pos.x = 0;//cl.snap.ps.origin[0];
	pos.y = 0;//cl.snap.ps.origin[1];
	pos.z = 0;//cl.snap.ps.origin[2];

	AngleVectors(cl.snap.ps.viewangles, forward, right, up);

	ang.x = forward[0];
	ang.y = forward[1];
	ang.z = forward[2];

	top.x = up[0];
	top.y = up[1];
	top.z = up[2];

	vel.x = 0;
	vel.y = 0;
	vel.z = 0;

	BASS_Set3DPosition(&pos, &vel, &ang, &top);

	if (origin)
		VectorCopy(origin, SOUND_CHANNELS[c]->origin);
	else
		VectorSet(SOUND_CHANNELS[c]->origin, 0, 0, 0);

	if (SOUND_CHANNELS[c]->entityNum == -1 && SOUND_CHANNELS[c]->origin[0] == 0 && SOUND_CHANNELS[c]->origin[1] == 0 && SOUND_CHANNELS[c]->origin[2] == 0)
	{// Local sound...
		soundPos[0] = 0.0;
		soundPos[1] = 0.0;
		soundPos[2] = 0.0;

		SOUND_CHANNELS[c]->ang.x = 0.0;
		SOUND_CHANNELS[c]->ang.y = 0.0;
		SOUND_CHANNELS[c]->ang.z = 0.0;
	}
	else if (SOUND_CHANNELS[c]->entityNum == -1 && !(SOUND_CHANNELS[c]->origin[0] == 0 && SOUND_CHANNELS[c]->origin[1] == 0 && SOUND_CHANNELS[c]->origin[2] == 0))
	{// No entity, but we have an origin...
		VectorSubtract(cl.snap.ps.origin, SOUND_CHANNELS[c]->origin, soundAng);

		AngleVectors(soundAng, forward, right, up);
		SOUND_CHANNELS[c]->ang.x = forward[0];
		SOUND_CHANNELS[c]->ang.y = forward[1];
		SOUND_CHANNELS[c]->ang.z = forward[2];

		//soundPos[0] = SOUND_CHANNELS[c]->origin[0];
		//soundPos[1] = SOUND_CHANNELS[c]->origin[1];
		//soundPos[2] = SOUND_CHANNELS[c]->origin[2];

		soundPos[0] = soundAng[0];
		soundPos[1] = soundAng[1];
		soundPos[2] = soundAng[2];
	}
	else
	{
		VectorSubtract(cl.snap.ps.origin, s_entityPosition[SOUND_CHANNELS[c]->entityNum], soundAng);

		AngleVectors(soundAng, forward, right, up);
		SOUND_CHANNELS[c]->ang.x = forward[0];
		SOUND_CHANNELS[c]->ang.y = forward[1];
		SOUND_CHANNELS[c]->ang.z = forward[2];

		//soundPos[0] = s_entityPosition[SOUND_CHANNELS[c]->entityNum][0];
		//soundPos[1] = s_entityPosition[SOUND_CHANNELS[c]->entityNum][1];
		//soundPos[2] = s_entityPosition[SOUND_CHANNELS[c]->entityNum][2];

		soundPos[0] = soundAng[0];//s_entityPosition[SOUND_CHANNELS[c]->entityNum][0];
		soundPos[1] = soundAng[1];//s_entityPosition[SOUND_CHANNELS[c]->entityNum][1];
		soundPos[2] = soundAng[2];//s_entityPosition[SOUND_CHANNELS[c]->entityNum][2];
	}

	SOUND_CHANNELS[c]->vel.x = soundPos[0] - SOUND_CHANNELS[c]->pos.x;
	SOUND_CHANNELS[c]->vel.y = soundPos[1] - SOUND_CHANNELS[c]->pos.y;
	SOUND_CHANNELS[c]->vel.z = soundPos[2] - SOUND_CHANNELS[c]->pos.z;

	// Set origin...
	SOUND_CHANNELS[c]->pos.x = soundPos[0];
	SOUND_CHANNELS[c]->pos.y = soundPos[1];
	SOUND_CHANNELS[c]->pos.z = soundPos[2];

	BASS_ChannelSetAttribute(SOUND_CHANNELS[c]->channel, BASS_ATTRIB_VOL, S_GetVolumeForChannel(SOUND_CHANNELS[c]->entityChannel));
	BASS_ChannelSet3DPosition(SOUND_CHANNELS[c]->channel,&SOUND_CHANNELS[c]->pos,&SOUND_CHANNELS[c]->ang,&SOUND_CHANNELS[c]->vel);
	BASS_ChannelSet3DAttributes(SOUND_CHANNELS[c]->channel, /*BASS_3DMODE_RELATIVE*/BASS_3DMODE_NORMAL, 10.0, 2048.0, -1, -1, -1);
}


int BASS_UPDATE_TIMER = 0;
int BASS_TIME_PERIOD = 50;

void BASS_UpdatePosition ( int c )
{// Update this channel's position, etc...
	//if (BASS_UPDATE_TIMER > cls.realtime) return; // wait...

	//BASS_UPDATE_TIMER = cls.realtime + BASS_TIME_PERIOD;

	vec3_t soundPos, soundAng, forward, right, up;
	BASS_3DVECTOR pos, ang, top, vel;

	if (SOUND_CHANNELS[c]->entityNum == -1 && SOUND_CHANNELS[c]->origin[0] == 0 && SOUND_CHANNELS[c]->origin[1] == 0 && SOUND_CHANNELS[c]->origin[2] == 0)
	{// Local sound...
		soundPos[0] = 0.0;
		soundPos[1] = 0.0;
		soundPos[2] = 0.0;

		SOUND_CHANNELS[c]->ang.x = 0.0;
		SOUND_CHANNELS[c]->ang.y = 0.0;
		SOUND_CHANNELS[c]->ang.z = 0.0;
	}
	else if (SOUND_CHANNELS[c]->entityNum == -1 && !(SOUND_CHANNELS[c]->origin[0] == 0 && SOUND_CHANNELS[c]->origin[1] == 0 && SOUND_CHANNELS[c]->origin[2] == 0))
	{// No entity, but we have an origin...
		VectorSubtract(cl.snap.ps.origin, SOUND_CHANNELS[c]->origin, soundAng);

		AngleVectors(soundAng, forward, right, up);
		SOUND_CHANNELS[c]->ang.x = forward[0];
		SOUND_CHANNELS[c]->ang.y = forward[1];
		SOUND_CHANNELS[c]->ang.z = forward[2];

		//soundPos[0] = SOUND_CHANNELS[c]->origin[0];
		//soundPos[1] = SOUND_CHANNELS[c]->origin[1];
		//soundPos[2] = SOUND_CHANNELS[c]->origin[2];

		soundPos[0] = soundAng[0];
		soundPos[1] = soundAng[1];
		soundPos[2] = soundAng[2];
	}
	else
	{
		VectorSubtract(cl.snap.ps.origin, s_entityPosition[SOUND_CHANNELS[c]->entityNum], soundAng);

		AngleVectors(soundAng, forward, right, up);
		SOUND_CHANNELS[c]->ang.x = forward[0];
		SOUND_CHANNELS[c]->ang.y = forward[1];
		SOUND_CHANNELS[c]->ang.z = forward[2];

		//soundPos[0] = s_entityPosition[SOUND_CHANNELS[c]->entityNum][0];
		//soundPos[1] = s_entityPosition[SOUND_CHANNELS[c]->entityNum][1];
		//soundPos[2] = s_entityPosition[SOUND_CHANNELS[c]->entityNum][2];

		soundPos[0] = soundAng[0];//s_entityPosition[SOUND_CHANNELS[c]->entityNum][0];
		soundPos[1] = soundAng[1];//s_entityPosition[SOUND_CHANNELS[c]->entityNum][1];
		soundPos[2] = soundAng[2];//s_entityPosition[SOUND_CHANNELS[c]->entityNum][2];
	}

	SOUND_CHANNELS[c]->vel.x = soundPos[0] - SOUND_CHANNELS[c]->pos.x;
	SOUND_CHANNELS[c]->vel.y = soundPos[1] - SOUND_CHANNELS[c]->pos.y;
	SOUND_CHANNELS[c]->vel.z = soundPos[2] - SOUND_CHANNELS[c]->pos.z;

	// Set origin...
	SOUND_CHANNELS[c]->pos.x = soundPos[0];
	SOUND_CHANNELS[c]->pos.y = soundPos[1];
	SOUND_CHANNELS[c]->pos.z = soundPos[2];

	BASS_ChannelSetAttribute(SOUND_CHANNELS[c]->channel, BASS_ATTRIB_VOL, S_GetVolumeForChannel(SOUND_CHANNELS[c]->entityChannel));
	BASS_ChannelSet3DPosition(SOUND_CHANNELS[c]->channel,&SOUND_CHANNELS[c]->pos,&SOUND_CHANNELS[c]->ang,&SOUND_CHANNELS[c]->vel);
	BASS_ChannelSet3DAttributes(SOUND_CHANNELS[c]->channel, /*BASS_3DMODE_RELATIVE*/BASS_3DMODE_NORMAL, 10.0, 2048.0, -1, -1, -1);
}

void BASS_Update ( void )
{
	int NUM_ACTIVE = 0;
	//int NUM_FREE = 0;

	BASS_SetConfig(BASS_CONFIG_GVOL_MUSIC, (DWORD)(float)(s_volume->value*10000.0));
	BASS_SetConfig(BASS_CONFIG_GVOL_SAMPLE, (DWORD)(float)(s_volume->value*10000.0));
	BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, (DWORD)(float)(s_volume->value*10000.0));

	vec3_t forward, right, up;
	BASS_3DVECTOR pos, ang, top, vel;

	pos.x = 0;//cl.snap.ps.origin[0];
	pos.y = 0;//cl.snap.ps.origin[1];
	pos.z = 0;//cl.snap.ps.origin[2];

	AngleVectors(cl.snap.ps.viewangles, forward, right, up);

	ang.x = forward[0];
	ang.y = forward[1];
	ang.z = forward[2];

	top.x = up[0];
	top.y = up[1];
	top.z = up[2];

	vel.x = 0;
	vel.y = 0;
	vel.z = 0;

	BASS_Set3DPosition(&pos, &vel, &ang, &top);

	if (MUSIC_CHANNEL_INITIALIZED)
	{
		BASS_ChannelSetAttribute(MUSIC_CHANNEL->channel, BASS_ATTRIB_VOL, S_GetVolumeForChannel(CHAN_MUSIC));
	}

	for (int c = 0; c < MAX_CHANNELS; c++) 
	{
		if (!SOUND_CHANNELS[c]) continue;

		if (SOUND_CHANNELS[c]->isActive && BASS_ChannelIsActive(SOUND_CHANNELS[c]->channel) == BASS_ACTIVE_PLAYING) 
		{// If the channel's playing then update it's position
			BASS_UpdatePosition(c);
		}
		else
		{// Finished. Remove the channel...
			if (SOUND_CHANNELS[c]->isActive)
			{// Still marked as active. Stop the channel and reset it...
				//Com_Printf("Removing inactive channel %i.\n", c);
				BASS_StopChannel(c);
			}
		}

		if (SOUND_CHANNELS[c]->isActive)
			NUM_ACTIVE++;
		//else
		//	NUM_FREE++;
	}

	//if (NUM_ACTIVE > 0)
	{// Apply the 3D changes. No point if we are not playing anything though...
		BASS_Apply3D();
	}

	//Com_Printf("There are currently %i active and %i free channels.\n", NUM_ACTIVE, NUM_FREE);
}

//
// Music Tracks...
//

void BASS_StopMusic( void )
{
	if (MUSIC_CHANNEL_INITIALIZED)
	{// Free old samples...
		if (BASS_ChannelIsActive(MUSIC_CHANNEL->channel) == BASS_ACTIVE_PLAYING)
		{
			BASS_ChannelStop(MUSIC_CHANNEL->channel);
		}

		//BASS_StreamFree(MUSIC_CHANNEL->channel);
		//BASS_SampleFree(MUSIC_CHANNEL->channel);
		//BASS_MusicFree(MUSIC_CHANNEL->channel);
	}
}

void BASS_StartMusic ( DWORD samplechan )
{
	if (!MUSIC_CHANNEL_INITIALIZED)
	{// Set up music channel...
		MUSIC_CHANNEL=(Channel*)malloc(sizeof(Channel));
		memset(MUSIC_CHANNEL,0,sizeof(Channel));
		MUSIC_CHANNEL->channel=samplechan;
		MUSIC_CHANNEL->entityNum = -1;
		MUSIC_CHANNEL->entityChannel = CHAN_MUSIC;
		MUSIC_CHANNEL_INITIALIZED = qtrue;
	}
	else
	{// Replace old music channel...
		BASS_StopMusic();

		// Set new samples...
		MUSIC_CHANNEL->channel = samplechan;
		MUSIC_CHANNEL->entityNum = -1;
		MUSIC_CHANNEL->entityChannel = CHAN_MUSIC;
	}

	BASS_SampleGetChannel(samplechan,FALSE); // initialize sample channel
	BASS_ChannelSetAttribute(samplechan, BASS_ATTRIB_VOL, S_GetVolumeForChannel(CHAN_MUSIC));

	// Play
	BASS_ChannelPlay(samplechan,TRUE);

	// Apply the 3D settings (music is always local)...
	MUSIC_CHANNEL->vel.x = 0;
	MUSIC_CHANNEL->vel.y = 0;
	MUSIC_CHANNEL->vel.z = 0;

	// Set origin...
	MUSIC_CHANNEL->pos.x = 0;
	MUSIC_CHANNEL->pos.y = 0;
	MUSIC_CHANNEL->pos.z = 0;

	BASS_ChannelSet3DPosition(MUSIC_CHANNEL->channel, &MUSIC_CHANNEL->pos, NULL, &MUSIC_CHANNEL->vel);
	BASS_ChannelSet3DAttributes(MUSIC_CHANNEL->channel, /*BASS_3DMODE_RELATIVE*/BASS_3DMODE_NORMAL, 10.0, 2048.0, -1, -1, -1);
	BASS_Apply3D();
}

DWORD BASS_LoadMusicSample ( void *memory, int length )
{// Just load a sample into memory ready to play instantly...
	DWORD newchan;

	if (newchan=BASS_StreamCreateFile(TRUE,memory,0,(DWORD)length,BASS_SAMPLE_LOOP/*0*/))
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
			c->channel=newchan;
			c->entityNum = entityNum;
			c->entityChannel = entityChannel;
			c->isActive = qtrue;

			if (entityNum == -1 || !origin || (origin[0] == 0 && origin[1] == 0 && origin[2] == 0))
			{
				c->entityNum = -1;
			}

			BASS_SampleGetChannel(newchan,FALSE); // initialize sample channel
			BASS_ChannelSetAttribute(newchan, BASS_ATTRIB_VOL, S_GetVolumeForChannel(entityChannel));

			// Play
			BASS_ChannelPlay(newchan,FALSE);

			// Apply the 3D changes
			BASS_SetPosition( chan, origin );
			BASS_ChannelSet3DAttributes(newchan, /*BASS_3DMODE_RELATIVE*/BASS_3DMODE_NORMAL, 10.0, 2048.0, -1, -1, -1);
			BASS_Apply3D();
	} else {
		Com_Printf("Can't load file (note samples must be mono)\n");
	}
}

void BASS_AddMemoryChannel ( DWORD samplechan, int entityNum, int entityChannel, vec3_t origin )
{
	int chan = BASS_FindFreeChannel();

	if (chan < 0)
	{// No channel left to play on...
		Com_Printf("BASS: No free sound channels.\n");
		return;
	}
	/*else
	{
		Com_Printf("BASS: Selected channel %i.\n", chan);
	}*/

	// Load a music or sample from "file" (memory)
	Channel *c = SOUND_CHANNELS[chan];
	memset(c,0,sizeof(Channel));
	c->channel=samplechan;
	c->entityNum = entityNum;
	c->entityChannel = entityChannel;
	c->isActive = qtrue;

	if (entityNum == -1 || !origin || (origin[0] == 0 && origin[1] == 0 && origin[2] == 0))
	{
		c->entityNum = -1;
	}

	samplechan = BASS_SampleGetChannel(samplechan,FALSE); // initialize sample channel
	c->channel=samplechan;
	BASS_ChannelSetAttribute(samplechan, BASS_ATTRIB_VOL, S_GetVolumeForChannel(entityChannel));

	// Play
	BASS_ChannelPlay(samplechan,FALSE);

	// Apply the 3D changes
	BASS_SetPosition( chan, origin );
	BASS_ChannelSet3DAttributes(samplechan, /*BASS_3DMODE_RELATIVE*/BASS_3DMODE_NORMAL, 10.0, 2048.0, -1, -1, -1);
	BASS_Apply3D();
}

void BASS_AddMemoryLoopChannel ( DWORD samplechan, int entityNum, int entityChannel, vec3_t origin )
{
	int chan = BASS_FindFreeChannel();

	if (chan < 0)
	{// No channel left to play on...
		Com_Printf("BASS: No free sound channels.\n");
		return;
	}
	/*else
	{
		Com_Printf("BASS: Selected channel %i.\n", chan);
	}*/

	// Load a music or sample from "file" (memory)
	Channel *c = SOUND_CHANNELS[chan];
	memset(c,0,sizeof(Channel));
	c->channel=samplechan;
	c->entityNum = entityNum;
	c->entityChannel = entityChannel;
	c->isActive = qtrue;

	if (entityNum == -1 || !origin || (origin[0] == 0 && origin[1] == 0 && origin[2] == 0))
	{
		c->entityNum = -1;
	}

	samplechan = BASS_SampleGetChannel(samplechan,FALSE); // initialize sample channel
	c->channel=samplechan;
	BASS_ChannelSetAttribute(samplechan, BASS_ATTRIB_VOL, S_GetVolumeForChannel(entityChannel));

	// Play
	BASS_ChannelPlay(samplechan,TRUE);

	// Apply the 3D changes
	BASS_SetPosition( chan, origin );
	BASS_ChannelSet3DAttributes(samplechan, /*BASS_3DMODE_RELATIVE*/BASS_3DMODE_NORMAL, 10.0, 2048.0, -1, -1, -1);
	BASS_Apply3D();
}

DWORD BASS_LoadMemorySample ( void *memory, int length )
{// Just load a sample into memory ready to play instantly...
	DWORD newchan;

	if ((newchan=BASS_SampleLoad(TRUE,memory,0,(DWORD)length,16,BASS_SAMPLE_3D|BASS_SAMPLE_SOFTWARE|BASS_SAMPLE_MONO))
		/*|| (newchan=BASS_StreamCreateFile(TRUE,memory,0,(DWORD)length,BASS_SAMPLE_3D|BASS_SAMPLE_SOFTWARE))*/)
	{
		return newchan;
	}

	return -1;
}

#endif //__USE_BASS__
