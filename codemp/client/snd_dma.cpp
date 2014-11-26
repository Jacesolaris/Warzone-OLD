/*****************************************************************************
 * name:		snd_dma.c
 *
 * desc:		main control for any streaming sound output device
 *
 *
 *****************************************************************************/
#include "snd_local.h"

#include "snd_mp3.h"
#include "snd_music.h"
#include "client.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

DWORD CURRENT_MUSIC_TRACK = 0;
sfx_t *CURRENT_MUSIC_SFX = NULL;

qboolean s_shutUp = qfalse;

static void S_Play_f(void);
static void S_SoundList_f(void);
static void S_Music_f(void);
static void S_SetDynamicMusic_f(void);

void S_Update_();
void S_StopAllSounds(void);
static void S_UpdateBackgroundTrack( void );
sfx_t *S_FindName( const char *name );
static int SND_FreeSFXMem(sfx_t *sfx);

extern qboolean Sys_LowPhysicalMemory();

//////////////////////////
//
// vars for bgrnd music track...
//
const int iMP3MusicStream_DiskBytesToRead = 10000;//4096;
const int iMP3MusicStream_DiskBufferSize = iMP3MusicStream_DiskBytesToRead*2; //*10;

typedef struct MusicInfo_s {
	qboolean	bIsMP3;
	//
	// MP3 specific...
	//
	sfx_t		sfxMP3_Bgrnd;
	MP3STREAM	streamMP3_Bgrnd;	// this one is pointed at by the sfx_t's ptr, and is NOT the one the decoder uses every cycle
	channel_t	chMP3_Bgrnd;		// ... but the one in this struct IS.
	//
	// MP3 disk streamer stuff... (if music is non-dynamic)
	//
	byte		byMP3MusicStream_DiskBuffer[iMP3MusicStream_DiskBufferSize];
	int			iMP3MusicStream_DiskReadPos;
	int			iMP3MusicStream_DiskWindowPos;
	//
	// MP3 disk-load stuff (for use during dynamic music, which is mem-resident)
	//
	byte		*pLoadedData;	// Z_Malloc, Z_Free	// these two MUST be kept as valid/invalid together
	char		sLoadedDataName[MAX_QPATH];			//  " " " " "
	int			iLoadedDataLen;
	//
	// remaining dynamic fields...
	//
	int			iXFadeVolumeSeekTime;
	int			iXFadeVolumeSeekTo;	// when changing this, set the above timer to Sys_Milliseconds().
									//	Note that this should be thought of more as an up/down bool rather than as a
									//	number now, in other words set it only to 0 or 255. I'll probably change this
									//	to actually be a bool later.
	int			iXFadeVolume;		// 0 = silent, 255 = max mixer vol, though still modulated via overall music_volume
	float		fSmoothedOutVolume;
	qboolean	bActive;			// whether playing or not
	qboolean	bExists;			// whether was even loaded for this level (ie don't try and start playing it)
	//
	// new dynamic fields...
	//
	qboolean	bTrackSwitchPending;
	MusicState_e eTS_NewState;
	float		 fTS_NewTime;
	//
	// Generic...
	//
	fileHandle_t s_backgroundFile;	// valid handle, else -1 if an MP3 (so that NZ compares still work)
	wavinfo_t	s_backgroundInfo;
	int			s_backgroundSamples;
} MusicInfo_t;

static void S_SetDynamicMusicState( MusicState_e musicState );

#define fDYNAMIC_XFADE_SECONDS (1.0f)

static MusicInfo_t	tMusic_Info[eBGRNDTRACK_NUMBEROF]	= {};
static qboolean		bMusic_IsDynamic					= qfalse;
static MusicState_e	eMusic_StateActual					= eBGRNDTRACK_EXPLORE;	// actual state, can be any enum
static MusicState_e	eMusic_StateRequest					= eBGRNDTRACK_EXPLORE;	// requested state, can only be explore, action, boss, or silence
static char			sMusic_BackgroundLoop[MAX_QPATH]	= {0};	// only valid for non-dynamic music
static char			sInfoOnly_CurrentDynamicMusicSet[64];	// any old reasonable size, only has to fit stuff like "kejim_post"
//
//////////////////////////


// =======================================================================
// Internal sound data & structures
// =======================================================================

// only begin attenuating sound volumes when outside the FULLVOLUME range
#define		SOUND_FULLVOLUME	256

#define		SOUND_ATTENUATE		0.0008f
#define		VOICE_ATTENUATE		0.004f

//const float	SOUND_FMAXVOL=0.75;//1.0;
const float	SOUND_FMAXVOL=1.0; // UQ1: Why was this lowered???
const int	SOUND_MAXVOL=255;

channel_t   s_channels[MAX_CHANNELS];

int			s_soundStarted;
qboolean	s_soundMuted;

dma_t		dma;

int			listener_number;
vec3_t		listener_origin;
matrix3_t	listener_axis;

int			s_soundtime;		// sample PAIRS
int   		s_paintedtime; 		// sample PAIRS

// MAX_SFX may be larger than MAX_SOUNDS because
// of custom player sounds
//#define		MAX_SFX			10000	//512 * 2
#define		MAX_SFX			14000	//512 * 2
sfx_t		s_knownSfx[MAX_SFX];
int			s_numSfx;

#define		LOOP_HASH		128
static	sfx_t		*sfxHash[LOOP_HASH];

cvar_t		*s_volume;
cvar_t		*s_volumeVoice;
cvar_t		*s_volumeEffects;
cvar_t		*s_volumeAmbient;
cvar_t		*s_volumeWeapon;
cvar_t		*s_volumeItem;
cvar_t		*s_volumeBody;
cvar_t		*s_volumeMusic;
cvar_t		*s_volumeLocal;
cvar_t		*s_testsound;
cvar_t		*s_khz;
cvar_t		*s_allowDynamicMusic;
cvar_t		*s_show;
cvar_t		*s_mixahead;
cvar_t		*s_mixPreStep;
cvar_t		*s_separation;
cvar_t		*s_lip_threshold_1;
cvar_t		*s_lip_threshold_2;
cvar_t		*s_lip_threshold_3;
cvar_t		*s_lip_threshold_4;
cvar_t		*s_language;	// note that this is distinct from "g_language"
cvar_t		*s_dynamix;
cvar_t		*s_debugdynamic;

cvar_t		*s_doppler;

typedef struct
{
	unsigned char	volume;
	vec3_t			origin;
	vec3_t			velocity;
	sfx_t		*sfx;
	int				mergeFrame;
	int			entnum;

	qboolean	doppler;
	float		dopplerScale;

	// For Open AL
	bool	bProcessed;
	bool	bRelative;
} loopSound_t;

#define	MAX_LOOP_SOUNDS		64

int			numLoopSounds;
loopSound_t	loopSounds[MAX_LOOP_SOUNDS];

int			s_rawend;
portable_samplepair_t	s_rawsamples[MAX_RAW_SAMPLES];
vec3_t		s_entityPosition[MAX_GENTITIES];
int			s_entityWavVol[MAX_GENTITIES];
int			s_entityWavVol_back[MAX_GENTITIES];

int			s_numChannels;			// Number of AL Sources == Num of Channels

bool					s_bInWater;				// Underwater effect currently active


// instead of clearing a whole channel_t struct, we're going to skip the MP3SlidingDecodeBuffer[] buffer in the middle...
//
#ifndef offsetof
#include <stddef.h>
#endif
static inline void Channel_Clear(channel_t *ch)
{
	// memset (ch, 0, sizeof(*ch));

	memset(ch,0,offsetof(channel_t,MP3SlidingDecodeBuffer));

	byte *const p = (byte *)ch + offsetof(channel_t,MP3SlidingDecodeBuffer) + sizeof(ch->MP3SlidingDecodeBuffer);

	memset(p,0,(sizeof(*ch) - offsetof(channel_t,MP3SlidingDecodeBuffer)) - sizeof(ch->MP3SlidingDecodeBuffer));
}

// ====================================================================
// User-setable variables
// ====================================================================
static void DynamicMusicInfoPrint(void)
{
	if (bMusic_IsDynamic)
	{
		// horribly lazy... ;-)
		//
		const char *psRequestMusicState	= Music_BaseStateToString( eMusic_StateRequest );
		const char *psActualMusicState	= Music_BaseStateToString( eMusic_StateActual, qtrue );
		if (psRequestMusicState == NULL)
		{
			psRequestMusicState = "<unknown>";
		}
		if (psActualMusicState	== NULL)
		{
			psActualMusicState	= "<unknown>";
		}

		Com_Printf("( Dynamic music ON, request state: '%s'(%d), actual: '%s' (%d) )\n", psRequestMusicState, eMusic_StateRequest, psActualMusicState, eMusic_StateActual);
	}
	else
	{
		Com_Printf("( Dynamic music OFF )\n");
	}
}

void S_SoundInfo_f(void) {
	Com_Printf("----- Sound Info -----\n" );

	if (!s_soundStarted) {
		Com_Printf ("sound system not started\n");
	} else {
		if ( s_soundMuted ) {
			Com_Printf ("sound system is muted\n");
		}

		Com_Printf("%5d stereo\n", dma.channels - 1);
		Com_Printf("%5d samples\n", dma.samples);
		Com_Printf("%5d samplebits\n", dma.samplebits);
		Com_Printf("%5d submission_chunk\n", dma.submission_chunk);
		Com_Printf("%5d speed\n", dma.speed);
		Com_Printf( "0x%" PRIxPTR " dma buffer\n", dma.buffer );

		if (bMusic_IsDynamic)
		{
			DynamicMusicInfoPrint();
			Com_Printf("( Dynamic music set name: \"%s\" )\n",sInfoOnly_CurrentDynamicMusicSet);
		}
		else
		{
			if (!s_allowDynamicMusic->integer)
			{
				Com_Printf("( Dynamic music inhibited (s_allowDynamicMusic == 0) )\n", sMusic_BackgroundLoop );
			}
			if ( tMusic_Info[eBGRNDTRACK_NONDYNAMIC].s_backgroundFile )
			{
				Com_Printf("Background file: %s\n", sMusic_BackgroundLoop );
			}
			else
			{
				Com_Printf("No background file.\n" );
			}
		}
	}
	S_DisplayFreeMemory();
	Com_Printf("----------------------\n" );
}



/*
================
S_Init
================
*/
void S_Init( void ) {
	cvar_t	*cv;

	Com_Printf("\n------- sound initialization -------\n");

	s_volume = Cvar_Get ("s_volume", "1.0", CVAR_ARCHIVE);
	s_volumeVoice= Cvar_Get ("s_volumeVoice", "1.0", CVAR_ARCHIVE);
	s_volumeEffects= Cvar_Get ("s_volumeEffects", "0.7", CVAR_ARCHIVE);
	s_volumeAmbient= Cvar_Get ("s_volumeAmbient", "0.5", CVAR_ARCHIVE);
	s_volumeWeapon= Cvar_Get ("s_volumeWeapon", "0.5", CVAR_ARCHIVE);
	s_volumeItem= Cvar_Get ("s_volumeItem", "0.5", CVAR_ARCHIVE);
	s_volumeBody= Cvar_Get ("s_volumeBody", "0.5", CVAR_ARCHIVE);
	s_volumeMusic = Cvar_Get ("s_volumeMusic", "0.25", CVAR_ARCHIVE);
	s_volumeLocal = Cvar_Get ("s_volumeLocal", "0.5", CVAR_ARCHIVE);
	s_separation = Cvar_Get ("s_separation", "0.5", CVAR_ARCHIVE);
	s_khz = Cvar_Get ("s_khz", "44", CVAR_ARCHIVE|CVAR_LATCH);
	s_allowDynamicMusic = Cvar_Get ("s_allowDynamicMusic", "1", CVAR_ARCHIVE);
	s_mixahead = Cvar_Get ("s_mixahead", "0.2", CVAR_ARCHIVE);

	s_mixPreStep = Cvar_Get ("s_mixPreStep", "0.05", CVAR_ARCHIVE);
	s_show = Cvar_Get ("s_show", "0", CVAR_CHEAT);
	s_testsound = Cvar_Get ("s_testsound", "0", CVAR_CHEAT);
	s_debugdynamic = Cvar_Get("s_debugdynamic","0", CVAR_CHEAT);
	s_lip_threshold_1 = Cvar_Get("s_threshold1" , "0.5",0);
	s_lip_threshold_2 = Cvar_Get("s_threshold2" , "4.0",0);
	s_lip_threshold_3 = Cvar_Get("s_threshold3" , "7.0",0);
	s_lip_threshold_4 = Cvar_Get("s_threshold4" , "8.0",0);

	s_language = Cvar_Get("s_language","english",CVAR_ARCHIVE | CVAR_NORESTART);

	s_doppler = Cvar_Get("s_doppler", "1", CVAR_ARCHIVE);

	cv = Cvar_Get ("s_initsound", "1", 0);
	if ( !cv->integer ) {
		s_soundStarted = 0;	// needed in case you set s_initsound to 0 midgame then snd_restart (div0 err otherwise later)
		Com_Printf ("not initializing.\n");
		Com_Printf("------------------------------------\n");
		return;
	}

	Cmd_AddCommand("play", S_Play_f);
	Cmd_AddCommand("music", S_Music_f);
	Cmd_AddCommand("soundlist", S_SoundList_f);
	Cmd_AddCommand("soundinfo", S_SoundInfo_f);
	Cmd_AddCommand("soundstop", S_StopAllSounds);

	BASS_Initialize();

	Com_Printf("------------------------------------\n");

	Com_Printf("\n--- ambient sound initialization ---\n");

	AS_Init();
}

// only called from snd_restart. QA request...
//
void S_ReloadAllUsedSounds(void)
{
	{
		// new bit, reload all soundsthat are used on the current level...
		//
		for (int i=1 ; i < s_numSfx ; i++)	// start @ 1 to skip freeing default sound
		{
			sfx_t *sfx = &s_knownSfx[i];

			if (!sfx->bDefaultSound && sfx->iLastLevelUsedOn == re->RegisterMedia_GetLevel()){
				S_memoryLoad(sfx);
			}
		}
	}
}

// =======================================================================
// Shutdown sound engine
// =======================================================================

void S_Shutdown( void )
{
	if ( !s_soundStarted ) {
		return;
	}

	S_FreeAllSFXMem();
	S_UnCacheDynamicMusic();

	BASS_Shutdown();

	SNDDMA_Shutdown();

	s_soundStarted = 0;

	Cmd_RemoveCommand("play");
	Cmd_RemoveCommand("music");
	Cmd_RemoveCommand("stopsound");
	Cmd_RemoveCommand("soundlist");
	Cmd_RemoveCommand("soundinfo");
	Cmd_RemoveCommand("soundstop");
	Cmd_RemoveCommand("mp3_calcvols");
	Cmd_RemoveCommand("s_dynamic");
	AS_Free();
}


// =======================================================================
// Load a sound
// =======================================================================
/*
================
return a hash value for the sfx name
================
*/
static long S_HashSFXName(const char *name) {
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (name[i] != '\0') {
		letter = tolower(name[i]);
		if (letter =='.') break;				// don't include extension
		if (letter =='\\') letter = '/';		// damn path names
		hash+=(long)(letter)*(i+119);
		i++;
	}
	hash &= (LOOP_HASH-1);
	return hash;
}

/*
==================
S_FindName

Will allocate a new sfx if it isn't found
==================
*/
sfx_t *S_FindName( const char *name ) {
	int		i;
	int		hash;

	sfx_t	*sfx;

	if (!name) {
		Com_Error (ERR_FATAL, "S_FindName: NULL");
	}
	if (!name[0]) {
		Com_Error (ERR_FATAL, "S_FindName: empty name");
	}

	if (strlen(name) >= MAX_QPATH) {
		Com_Error (ERR_FATAL, "Sound name too long: %s", name);
	}

	char sSoundNameNoExt[MAX_QPATH];
	COM_StripExtension(name,sSoundNameNoExt, sizeof( sSoundNameNoExt ));
	Q_strlwr(sSoundNameNoExt);//UQ1: force it down low before hashing too?!?!?!?!

	hash = S_HashSFXName(sSoundNameNoExt);

	sfx = sfxHash[hash];


	// see if already loaded
	while (sfx) {
		if (!Q_stricmp(sfx->sSoundName, sSoundNameNoExt) ) {
			return sfx;
		}
		sfx = sfx->next;
	}

/*
	// find a free sfx
	for (i=0 ; i < s_numSfx ; i++) {
		if (!s_knownSfx[i].soundName[0]) {
			break;
		}
	}
*/
	i = s_numSfx;	//we don't clear the soundName after failed loads any more, so it'll always be the last entry

	if (s_numSfx == MAX_SFX)
	{
		// ok, no sfx's free, but are there any with defaultSound set? (which the registering ent will never
		//	see because he gets zero returned if it's default...)
		//
		for (i=0 ; i < s_numSfx ; i++) {
			if (s_knownSfx[i].bDefaultSound) {
				break;
			}
		}

		if (i==s_numSfx)
		{
			// genuinely out of handles...

			// if we ever reach this, let me know and I'll either boost the array or put in a map-used-on
			//	reference to enable sfx_t recycling. TA codebase relies on being able to have structs for every sound
			//	used anywhere, ever, all at once (though audio bit-buffer gets recycled). SOF1 used about 1900 distinct
			//	events, so current MAX_SFX limit should do, or only need a small boost...	-ste
			//

			Com_Error (ERR_FATAL, "S_FindName: out of sfx_t");
		}
	}
	else
	{
		s_numSfx++;
	}

	sfx = &s_knownSfx[i];
	memset (sfx, 0, sizeof(*sfx));
	Q_strncpyz(sfx->sSoundName, sSoundNameNoExt, sizeof(sfx->sSoundName));
	Q_strlwr(sfx->sSoundName);//force it down low

	sfx->next = sfxHash[hash];
	sfxHash[hash] = sfx;

	return sfx;
}

/*
=================
S_DefaultSound
=================
*/
void S_DefaultSound( sfx_t *sfx ) {
	int		i;

	sfx->indexSize	= 512;								// #samples, ie shorts
	sfx->indexData				= (byte *)	Z_Malloc(512*2, TAG_SND_RAWDATA, qfalse);	// ... so *2 for alloc bytes
	sfx->bInMemory				= qtrue;

	for ( i=0 ; i < sfx->indexSize ; i++ )
	{
		sfx->indexData[i] = i;
	}

	sfx->bassSampleID = BASS_LoadMemorySample( sfx->indexData, sfx->indexSize );
	Z_Free(sfx->indexData);
}

/*
===================
S_DisableSounds

Disables sounds until the next S_BeginRegistration.
This is called when the hunk is cleared and the sounds
are no longer valid.
===================
*/
void S_DisableSounds( void ) {
	S_StopAllSounds();
	s_soundMuted = qtrue;
}

/*
=====================
S_BeginRegistration

=====================
*/
void S_BeginRegistration( void )
{
	s_soundMuted = qfalse;		// we can play again

	if (s_numSfx == 0) {
		SND_setup();

		Com_Printf("Registering default sound.\n");

		s_numSfx = 0;
		
		memset( s_knownSfx, 0, sizeof( s_knownSfx ) );
		memset(sfxHash, 0, sizeof(sfx_t *)*LOOP_HASH);

//#ifdef _DEBUG
		sfx_t *sfx = S_FindName( "***DEFAULT***" );
		S_DefaultSound( sfx );
/*#else
		S_RegisterSound("sound/null.wav");
#endif*/
	}
}

/*
==================
S_RegisterSound

Creates a default buzz sound if the file can't be loaded
==================
*/
sfxHandle_t	S_RegisterSound( const char *name)
{
	sfx_t	*sfx;

	if (!s_soundStarted) {
		return 0;
	}

	if ( strlen( name ) >= MAX_QPATH ) {
		Com_Printf( S_COLOR_RED"Sound name exceeds MAX_QPATH - %s\n", name );
		assert(0);
		return 0;
	}

	sfx = S_FindName( name );

	SND_TouchSFX(sfx);

	if ( sfx->bDefaultSound )
		return 0;

	if ( sfx->bassSampleID )
	{
		return sfx - s_knownSfx;
	}

	sfx->bInMemory = qfalse;

	S_memoryLoad(sfx);

	if ( sfx->bDefaultSound ) {
#ifndef FINAL_BUILD
		if (!s_shutUp)
		{
			Com_Printf( S_COLOR_YELLOW "WARNING: could not find %s - using default\n", sfx->sSoundName );
		}
#endif
		return 0;
	}

	return sfx - s_knownSfx;
}

void S_memoryLoad(sfx_t	*sfx)
{
	// load the sound file...
	//
	if ( !S_LoadSound( sfx ) )
	{
//		Com_Printf( S_COLOR_YELLOW "WARNING: couldn't load sound: %s\n", sfx->sSoundName );
		sfx->bDefaultSound = qtrue;
	}
	sfx->bInMemory = qtrue;
}

//=============================================================================
static qboolean S_CheckChannelStomp( int chan1, int chan2 )
{
	return qfalse;
}

/*
=================
S_PickChannel
=================
*/
channel_t *S_PickChannel(int entnum, int entchannel)
{
    int			ch_idx;
	channel_t	*ch, *firstToDie;
	qboolean	foundChan = qfalse;

	if ( entchannel<0 ) {
		Com_Error (ERR_DROP, "S_PickChannel: entchannel<0");
	}

	// Check for replacement sound, or find the best one to replace

    firstToDie = &s_channels[0];

	for ( int pass = 0; (pass < ((entchannel == CHAN_AUTO || entchannel == CHAN_LESS_ATTEN)?1:2)) && !foundChan; pass++ )
	{
		for (ch_idx = 0, ch = &s_channels[0]; ch_idx < MAX_CHANNELS ; ch_idx++, ch++ )
		{
			if ( entchannel == CHAN_AUTO || entchannel == CHAN_LESS_ATTEN || pass > 0 )
			{//if we're on the second pass, just find the first open chan
				if ( !ch->thesfx )
				{//grab the first open channel
					firstToDie = ch;
					break;
				}

			}
			else if ( ch->entnum == entnum && S_CheckChannelStomp( ch->entchannel, entchannel ) )
			{
				// always override sound from same entity
				if ( s_show->integer == 1 && ch->thesfx ) {
					Com_Printf( S_COLOR_YELLOW"...overrides %s\n", ch->thesfx->sSoundName );
					ch->thesfx = 0;	//just to clear the next error msg
				}
				firstToDie = ch;
				foundChan = qtrue;
				break;
			}

			// don't let anything else override local player sounds
			if ( ch->entnum == listener_number 	&& entnum != listener_number && ch->thesfx) {
				continue;
			}

			// don't override loop sounds
			if ( ch->loopSound ) {
				continue;
			}

			if ( ch->startSample < firstToDie->startSample ) {
				firstToDie = ch;
			}
		}
	}

	if ( s_show->integer == 1 && firstToDie->thesfx ) {
		Com_Printf( S_COLOR_RED"***kicking %s\n", firstToDie->thesfx->sSoundName );
	}

	Channel_Clear(firstToDie);	// memset(firstToDie, 0, sizeof(*firstToDie));

	return firstToDie;
}

/*
=================
S_SpatializeOrigin

Used for spatializing s_channels
=================
*/
void S_SpatializeOrigin (const vec3_t origin, float master_vol, int *left_vol, int *right_vol, int channel, int volumechannel)
{
}

// =======================================================================
// Start a sound effect
// =======================================================================

/*
====================
S_StartAmbientSound

Starts an ambient, 'one-shot" sound.
====================
*/

void S_StartAmbientSound( const vec3_t origin, int entityNum, unsigned char volume, sfxHandle_t sfxHandle )
{
	if (s_knownSfx[ sfxHandle ].bassSampleID < 0) return;
	/*
	if (origin)
		Com_Printf("BASS_DEBUG: Entity %i playing AMBIENT sound %s on channel %i at org %f %f %f.\n", entityNum, s_knownSfx[ sfxHandle ].sSoundName, CHAN_AMBIENT, origin[0], origin[1], origin[2]);
	else
		Com_Printf("BASS_DEBUG: Entity %i playing AMBIENT sound %s on channel %i at NULL org.\n", entityNum, s_knownSfx[ sfxHandle ].sSoundName, CHAN_AMBIENT);
	*/

	if (S_ShouldCull((float *)origin, qfalse, entityNum))
		BASS_AddMemoryChannel(s_knownSfx[ sfxHandle ].bassSampleID, entityNum, CHAN_AMBIENT, (float *)origin, (float)((float)(volume*0.25)/255.0));
	else
		BASS_AddMemoryChannel(s_knownSfx[ sfxHandle ].bassSampleID, entityNum, CHAN_AMBIENT, (float *)origin, (float)((float)volume/255.0));
	return;
}

/*
====================
S_MuteSound

Mutes sound on specified channel for specified entity.
====================
*/
void S_MuteSound(int entityNum, int entchannel)
{
	BASS_StopEntityChannel( entityNum, entchannel );
}

/*
====================
S_StartSound

Validates the parms and ques the sound up
if pos is NULL, the sound will be dynamically sourced from the entity
entchannel 0 will never override a playing sound
====================
*/
void S_StartSound(const vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfxHandle )
{
	if (s_knownSfx[ sfxHandle ].bassSampleID < 0) return;

	/*
	if (origin)
		Com_Printf("BASS_DEBUG: Entity %i playing sound %s (handle %i - bass id %ld) on channel %i at org %f %f %f.\n", entityNum, s_knownSfx[ sfxHandle ].sSoundName, sfxHandle, s_knownSfx[ sfxHandle ].bassSampleID, entchannel, origin[0], origin[1], origin[2]);
	else
		Com_Printf("BASS_DEBUG: Entity %i playing sound %s (handle %i - bass id %ld) on channel %i at NULL org.\n", entityNum, s_knownSfx[ sfxHandle ].sSoundName, sfxHandle, s_knownSfx[ sfxHandle ].bassSampleID, entchannel);
	*/

	if (S_ShouldCull((float *)origin, qfalse, entityNum))
		BASS_AddMemoryChannel(s_knownSfx[ sfxHandle ].bassSampleID, entityNum, entchannel, (float *)origin, 0.25);
	else
		BASS_AddMemoryChannel(s_knownSfx[ sfxHandle ].bassSampleID, entityNum, entchannel, (float *)origin, 1.0);
	return;
}

/*
==================
S_StartLocalSound
==================
*/
void S_StartLocalSound( sfxHandle_t sfxHandle, int channelNum ) {
	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	if ( sfxHandle < 0 || sfxHandle >= s_numSfx ) {
		Com_Error( ERR_DROP, "S_StartLocalSound: handle %i out of range", sfxHandle );
	}

	S_StartSound (NULL, listener_number, channelNum, sfxHandle );
}


/*
==================
S_StartLocalLoopingSound
==================
*/
void S_StartLocalLoopingSound( sfxHandle_t sfxHandle) {
	vec3_t nullVec = {0,0,0};

	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	if ( sfxHandle < 0 || sfxHandle >= s_numSfx ) {
		Com_Error( ERR_DROP, "S_StartLocalLoopingSound: handle %i out of range", sfxHandle );
	}

	S_AddLoopingSound( listener_number, nullVec, nullVec, sfxHandle );
}

// returns length in milliseconds of supplied sound effect...  (else 0 for bad handle now)
//
float S_GetSampleLengthInMilliSeconds( sfxHandle_t sfxHandle)
{
	return 512 * 1000;
}


/*
==================
S_ClearSoundBuffer

If we are about to perform file access, clear the buffer
so sound doesn't stutter.
==================
*/
void S_ClearSoundBuffer( void ) {
}


// kinda kludgy way to stop a special-use sfx_t playing...
//
void S_CIN_StopSound(sfxHandle_t sfxHandle)
{
	if ( sfxHandle < 0 || sfxHandle >= s_numSfx ) {
		Com_Error( ERR_DROP, "S_CIN_StopSound: handle %i out of range", sfxHandle );
	}

	if (s_knownSfx[ sfxHandle ].bassSampleID < 0) return;

	BASS_FindAndStopSound(s_knownSfx[ sfxHandle ].bassSampleID);
}


/*
==================
S_StopAllSounds
==================
*/
void S_StopSounds(void)
{
	BASS_StopAllChannels();
}

/*
==================
S_StopAllSounds
 and music
==================
*/
void S_StopAllSounds(void) {
	// stop the background music
	S_StopBackgroundTrack();

	S_StopSounds();
}

/*
==============================================================

continuous looping sounds are added each frame

==============================================================
*/

/*
==================
S_ClearLoopingSounds

==================
*/
void S_ClearLoopingSounds( void )
{
	//BASS_StopAllLoopChannels();
}

/*
==================
S_StopLoopingSound

Stops all active looping sounds on a specified entity.
Sort of a slow method though, isn't there some better way?
==================
*/
void S_StopLoopingSound( int entityNum )
{
	BASS_StopLoopChannel(entityNum);
	return;
}

#define MAX_DOPPLER_SCALE 50.0f //arbitrary

/*
==================
S_AddLoopingSound

Called during entity generation for a frame
Include velocity in case I get around to doing doppler...
==================
*/
void S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfxHandle ) {
	if (s_knownSfx[ sfxHandle ].bassSampleID < 0) return;
	
	if (entityNum >= 0 /*&& (cl.entityBaselines[entityNum].eType == ET_NPC || cl.entityBaselines[entityNum].eType == ET_PLAYER)*/)
	{
		/*
		if (origin)
			Com_Printf("BASS_DEBUG: Entity %i playing LOOPING sound %s on channel %i at org %f %f %f.\n", entityNum, s_knownSfx[ sfxHandle ].sSoundName, CHAN_BODY, origin[0], origin[1], origin[2]);
		else
			Com_Printf("BASS_DEBUG: Entity %i playing LOOPING sound %s on channel %i at NULL org.\n", entityNum, s_knownSfx[ sfxHandle ].sSoundName, CHAN_BODY);
		*/

		if (S_ShouldCull((float *)origin, qfalse, entityNum))
			BASS_AddMemoryLoopChannel(s_knownSfx[ sfxHandle ].bassSampleID, entityNum, CHAN_BODY, (float *)origin, 0.25);
		else
			BASS_AddMemoryLoopChannel(s_knownSfx[ sfxHandle ].bassSampleID, entityNum, CHAN_BODY, (float *)origin, 1.0);
	}
	else
	{
		/*
		if (origin)
			Com_Printf("BASS_DEBUG: Entity %i playing LOOPING sound %s on channel %i at org %f %f %f.\n", entityNum, s_knownSfx[ sfxHandle ].sSoundName, CHAN_AMBIENT, origin[0], origin[1], origin[2]);
		else
			Com_Printf("BASS_DEBUG: Entity %i playing LOOPING sound %s on channel %i at NULL org.\n", entityNum, s_knownSfx[ sfxHandle ].sSoundName, CHAN_AMBIENT);
		*/

		if (S_ShouldCull((float *)origin, qfalse, entityNum))
			BASS_AddMemoryLoopChannel(s_knownSfx[ sfxHandle ].bassSampleID, entityNum, CHAN_AMBIENT, (float *)origin, 0.25);
		else
			BASS_AddMemoryLoopChannel(s_knownSfx[ sfxHandle ].bassSampleID, entityNum, CHAN_AMBIENT, (float *)origin, 1.0);
	}
}


/*
==================
S_AddAmbientLoopingSound
==================
*/
void S_AddAmbientLoopingSound( const vec3_t origin, unsigned char volume, sfxHandle_t sfxHandle )
{
	if (s_knownSfx[ sfxHandle ].bassSampleID < 0) return;

	/*
	if (origin)
		Com_Printf("BASS_DEBUG: Entity %i playing AMBIENT LOOPING sound %s on channel %i at org %f %f %f.\n", -1, s_knownSfx[ sfxHandle ].sSoundName, CHAN_AMBIENT, origin[0], origin[1], origin[2]);
	else
		Com_Printf("BASS_DEBUG: Entity %i playing AMBIENT LOOPING sound %s on channel %i at NULL org.\n", -1, s_knownSfx[ sfxHandle ].sSoundName, CHAN_AMBIENT);
	*/

	if (S_ShouldCull((float *)origin, qfalse, -1))
		BASS_AddMemoryLoopChannel(s_knownSfx[ sfxHandle ].bassSampleID, -1, CHAN_AMBIENT, (float *)origin, (float)((float)(volume*0.25)/255.0));
	else
		BASS_AddMemoryLoopChannel(s_knownSfx[ sfxHandle ].bassSampleID, -1, CHAN_AMBIENT, (float *)origin, (float)((float)volume/255.0));
}



/*
==================
S_AddLoopSounds

Spatialize all of the looping sounds.
All sounds are on the same cycle, so any duplicates can just
sum up the channel multipliers.
==================
*/
void S_AddLoopSounds (void)
{
}

//=============================================================================

/*
=================
S_ByteSwapRawSamples

If raw data has been loaded in little endian binary form, this must be done.
If raw data was calculated, as with ADPCM, this should not be called.
=================
*/
void S_ByteSwapRawSamples( int samples, int width, int s_channels, const byte *data ) 
{
}


portable_samplepair_t *S_GetRawSamplePointer() {
	return s_rawsamples;
}


/*
============
S_RawSamples

Music streaming
============
*/
void S_RawSamples( int samples, int rate, int width, int s_channels, const byte *data, float volume, int bFirstOrOnlyUpdateThisFrame )
{
}

//=============================================================================

/*
=====================
S_UpdateEntityPosition

let the sound system know where an entity currently is
======================
*/
void S_UpdateEntityPosition( int entityNum, const vec3_t origin )
{
	if ( entityNum < 0 || entityNum >= MAX_GENTITIES ) {
		Com_Error( ERR_DROP, "S_UpdateEntityPosition: bad entitynum %i", entityNum );
	}

	VectorCopy( origin, s_entityPosition[entityNum] );
}


// Given a current wav we are playing, and our position within it, lets figure out its volume...
//
// (this is mostly Jake's code from EF1, which explains a lot...:-)
//
static int next_amplitude = 0;
static int S_CheckAmplitude(channel_t	*ch, const unsigned int s_oldpaintedtime )
{
	return 0; // UQ1: FIXME - lip sync...
}
/*
============
S_Respatialize

Change the volumes of all the playing sounds for changes in their positions
============
*/
void S_Respatialize( int entityNum, const vec3_t head, matrix3_t axis, int inwater )
{
	// Check if the Listener is underwater
	if (inwater)
	{
		// Check if we have already applied Underwater effect
		if (!s_bInWater)
		{
			s_bInWater = true;
			BASS_SetEAX_UNDERWATER();
		}
		else
		{
			if (s_bInWater)
			{
				s_bInWater = false;
				BASS_SetEAX_NORMAL();
			}
		}
	}
	else if (s_bInWater)
	{
		s_bInWater = false;
		BASS_SetEAX_NORMAL();
	}
}


/*
========================
S_ScanChannelStarts

Returns qtrue if any new sounds were started since the last mix
========================
*/
qboolean S_ScanChannelStarts( void ) 
{
	return qfalse;
}

// this is now called AFTER the DMA painting, since it's only the painter calls that cause the MP3s to be unpacked,
//	and therefore to have data readable by the lip-sync volume calc code.
//
void S_DoLipSynchs( const unsigned s_oldpaintedtime )
{
}

/*
============
S_Update

Called once each time through the main loop
============
*/
void S_Update( void ) 
{
	BASS_UpdateSounds();
}

void S_GetSoundtime(void)
{
}


void S_Update_(void) {
}

int S_MP3PreProcessLipSync(channel_t *ch, short *data)
{
	return 0;
}

void S_SetLipSyncs()
{
}

/*
===============================================================================

console functions

===============================================================================
*/

static void S_Play_f( void ) {
	int 	i;
	sfxHandle_t	h;
	char name[256];

	i = 1;
	while ( i<Cmd_Argc() ) {
		if ( !strrchr(Cmd_Argv(i), '.') ) {
			Com_sprintf( name, sizeof(name), "%s.wav", Cmd_Argv(1) );
		} else {
			Q_strncpyz( name, Cmd_Argv(i), sizeof(name) );
		}
		h = S_RegisterSound( name );
		if( h ) {
			S_StartLocalSound( h, CHAN_LOCAL_SOUND );
		}
		i++;
	}
}

static void S_Music_f( void ) {
	int		c;

	c = Cmd_Argc();

	if ( c == 2 ) {
		S_StartBackgroundTrack( Cmd_Argv(1), Cmd_Argv(1), qfalse );
	} else if ( c == 3 ) {
		S_StartBackgroundTrack( Cmd_Argv(1), Cmd_Argv(2), qfalse );
	} else {
		Com_Printf ("music <musicfile> [loopfile]\n");
		return;
	}
}

// a debug function, but no harm to leave in...
//
static void S_SetDynamicMusic_f(void)
{
}

// this table needs to be in-sync with the typedef'd enum "SoundCompressionMethod_t"...	-ste
//
static const char *sSoundCompressionMethodStrings[ct_NUMBEROF] =
{
	"16b",	// ct_16
	"mp3"	// ct_MP3
};
void S_SoundList_f( void ) 
{
}

/*
===============================================================================

background music functions

===============================================================================
*/

int	FGetLittleLong( fileHandle_t f ) {
	int		v;

	FS_Read( &v, sizeof(v), f );

	return LittleLong( v);
}

int	FGetLittleShort( fileHandle_t f ) {
	short	v;

	FS_Read( &v, sizeof(v), f );

	return LittleShort( v);
}

// returns the length of the data in the chunk, or 0 if not found
int S_FindWavChunk( fileHandle_t f, char *chunk ) {
	char	name[5];
	int		len;
	int		r;

	name[4] = 0;
	len = 0;
	r = FS_Read( name, 4, f );
	if ( r != 4 ) {
		return 0;
	}
	len = FGetLittleLong( f );
	if ( len < 0 || len > 0xfffffff ) {
		len = 0;
		return 0;
	}
	len = (len + 1 ) & ~1;		// pad to word boundary
//	s_nextWavChunk += len + 8;

	if ( strcmp( name, chunk ) ) {
		return 0;
	}
	return len;
}

// fixme: need to move this into qcommon sometime?, but too much stuff altered by other people and I won't be able
//	to compile again for ages if I check that out...
//
// DO NOT replace this with a call to FS_FileExists, that's for checking about writing out, and doesn't work for this.
//
qboolean S_FileExists( const char *psFilename )
{
	fileHandle_t fhTemp;

	FS_FOpenFileRead (psFilename, &fhTemp, qtrue);	// qtrue so I can fclose the handle without closing a PAK
	if (!fhTemp)
		return qfalse;

	FS_FCloseFile(fhTemp);
	return qtrue;
}

// some stuff for streaming MP3 files from disk (not pleasant, but nothing about MP3 is, other than compression ratios...)
//
static void MP3MusicStream_Reset(MusicInfo_t *pMusicInfo)
{
}

//
// return is where the decoder should read from...
//
static byte *MP3MusicStream_ReadFromDisk(MusicInfo_t *pMusicInfo, int iReadOffset, int iReadBytesNeeded)
{
	return NULL;
}

// does NOT set s_rawend!...
//
static void S_StopBackgroundTrack_Actual( MusicInfo_t *pMusicInfo )
{
	if (CURRENT_MUSIC_TRACK && CURRENT_MUSIC_SFX)
	{// Need to free the old sound!
		BASS_StopMusic(CURRENT_MUSIC_TRACK);
		memset (CURRENT_MUSIC_SFX, 0, sizeof(sfx_t));
	}
}

static void FreeMusic( MusicInfo_t *pMusicInfo )
{
}

// called only by snd_shutdown (from snd_restart or app exit)
//
void S_UnCacheDynamicMusic( void )
{
}

static qboolean S_StartBackgroundTrack_Actual( MusicInfo_t *pMusicInfo, qboolean qbDynamic, const char *intro, const char *loop )
{
	char	name[MAX_QPATH];

	Q_strncpyz( sMusic_BackgroundLoop, loop, sizeof( sMusic_BackgroundLoop ));

	Q_strncpyz( name, intro, sizeof( name ) - 4 );	// this seems to be so that if the filename hasn't got an extension
													//	but doesn't have the room to append on either then you'll just
													//	get the "soft" fopen() error, rather than the ERR_DROP you'd get
													//	if COM_DefaultExtension didn't have room to add it on.
	COM_DefaultExtension( name, sizeof( name ), ".mp3" );

	if (CURRENT_MUSIC_TRACK && CURRENT_MUSIC_SFX)
	{// Need to free the old sound!
		BASS_StopMusic(CURRENT_MUSIC_TRACK);
		memset (CURRENT_MUSIC_SFX, 0, sizeof(sfx_t));
	}

	sfxHandle_t	sfxHandle = S_RegisterSound( name );

	if (s_knownSfx[ sfxHandle ].bassSampleID < 0) return qfalse;
	CURRENT_MUSIC_TRACK = s_knownSfx[ sfxHandle ].bassSampleID;
	CURRENT_MUSIC_SFX = &s_knownSfx[ sfxHandle ];
	BASS_StartMusic(s_knownSfx[ sfxHandle ].bassSampleID);
	return qtrue;
}

static void S_SwitchDynamicTracks( MusicState_e eOldState, MusicState_e eNewState, qboolean bNewTrackStartsFullVolume )
{
}

// called by both the config-string parser and the console-command state-changer...
//
// This either changes the music right now (copying track structures etc), or leaves the new state as pending
//	so it gets picked up by the general music player if in a transition that can't be overridden...
//
static void S_SetDynamicMusicState( MusicState_e eNewState )
{
}

static void S_HandleDynamicMusicStateChange( void )
{
}

static char gsIntroMusic[MAX_QPATH]={0};
static char gsLoopMusic [MAX_QPATH]={0};

void S_RestartMusic( void )
{
	{
		//if (gsIntroMusic[0] || gsLoopMusic[0])	// dont test this anymore (but still *use* them), they're blank for JK2 dynamic-music levels anyway
		{
			MusicState_e ePrevState	= eMusic_StateRequest;
			S_StartBackgroundTrack( gsIntroMusic, gsLoopMusic, qfalse );	// ( default music start will set the state to EXPLORE )
			S_SetDynamicMusicState( ePrevState );					// restore to prev state
		}
	}
}

// Basic logic here is to see if the intro file specified actually exists, and if so, then it's not dynamic music,
//	When called by the cgame start it loads up, then stops the playback (because of stutter issues), so that when the
//	actual snapshot is received and the real play request is processed the data has already been loaded so will be quicker.
//
// to be honest, although the code still plays WAVs some of the file-check logic only works for MP3s, so if you ever want
//	to use WAV music you'll have to do some tweaking below (but I've got other things to do so it'll have to wait - Ste)
//
void S_StartBackgroundTrack( const char *intro, const char *loop, qboolean bCalledByCGameStart )
{
	bMusic_IsDynamic = qfalse;

	if (!s_soundStarted)
	{	//we have no sound, so don't even bother trying
		return;
	}

	if ( !intro ) {
		intro = "";
	}
	if ( !loop || !loop[0] ) {
		loop = intro;
	}

	Q_strncpyz(gsIntroMusic,intro, sizeof(gsIntroMusic));
	Q_strncpyz(gsLoopMusic, loop,  sizeof(gsLoopMusic));

	char sNameIntro[MAX_QPATH];
	char sNameLoop [MAX_QPATH];
	Q_strncpyz(sNameIntro,	intro,	sizeof(sNameIntro));
	Q_strncpyz(sNameLoop,	loop,	sizeof(sNameLoop));

	COM_DefaultExtension( sNameIntro, sizeof( sNameIntro ), ".mp3" );
	COM_DefaultExtension( sNameLoop,  sizeof( sNameLoop),	".mp3" );

	// if dynamic music not allowed, then just stream the explore music instead of playing dynamic...
	//
	if (!s_allowDynamicMusic->integer && Music_DynamicDataAvailable(intro))	// "intro", NOT "sName" (i.e. don't use version with ".mp3" extension)
	{
		const char *psMusicName = Music_GetFileNameForState( eBGRNDTRACK_DATABEGIN );
		if (psMusicName && S_FileExists( psMusicName ))
		{
			Q_strncpyz(sNameIntro,psMusicName,sizeof(sNameIntro));
			Q_strncpyz(sNameLoop, psMusicName,sizeof(sNameLoop ));
		}
	}

	// conceptually we always play the 'intro'[/sName] track, intro-to-loop transition is handled in UpdateBackGroundTrack().
	//
	if ( (strstr(sNameIntro,"/") && S_FileExists( sNameIntro )) )	// strstr() check avoids extra file-exists check at runtime if reverting from streamed music to dynamic since literal files all need at least one slash in their name (eg "music/blah")
	{
		const char *psLoopName = S_FileExists( sNameLoop ) ? sNameLoop : sNameIntro;
		Com_DPrintf("S_StartBackgroundTrack: Found/using non-dynamic music track '%s' (loop: '%s')\n", sNameIntro, psLoopName);
		S_StartBackgroundTrack_Actual( &tMusic_Info[eBGRNDTRACK_NONDYNAMIC], bMusic_IsDynamic, sNameIntro, psLoopName );
	}

	if (bCalledByCGameStart)
	{
		S_StopBackgroundTrack();
	}
}

void S_StopBackgroundTrack( void )
{
	if (CURRENT_MUSIC_TRACK && CURRENT_MUSIC_SFX)
	{// Need to free the old sound!
		BASS_StopMusic(CURRENT_MUSIC_TRACK);
		memset (CURRENT_MUSIC_SFX, 0, sizeof(sfx_t));
	}
}

// qboolean return is true only if we're changing from a streamed intro to a dynamic loop...
//
static qboolean S_UpdateBackgroundTrack_Actual( MusicInfo_t *pMusicInfo, qboolean bFirstOrOnlyMusicTrack, float fDefaultVolume)
{
	return qfalse;
}

// used to be just for dynamic, but now even non-dynamic music has to know whether it should be silent or not...
//
static const char *S_Music_GetRequestedState(void)
{
	return NULL;
}

// scan the configstring to see if there's been a state-change requested...
// (note that even if the state doesn't change it still gets here, so do a same-state check for applying)
//
// then go on to do transition handling etc...
//
static void S_CheckDynamicMusicState(void)
{
}

static void S_UpdateBackgroundTrack( void )
{
	// standard / non-dynamic one-track music...
	//
	const char *psCommand = S_Music_GetRequestedState();	// special check just for "silence" case...
	qboolean bShouldBeSilent = (qboolean)(psCommand && !Q_stricmp(psCommand,"silence"));
	float fDesiredVolume = bShouldBeSilent ? 0.0f : s_volumeMusic->value * s_volume->value;
	//
	// internal to this code is a volume-smoother...
	//
	qboolean bNewTrackDesired = S_UpdateBackgroundTrack_Actual(&tMusic_Info[eBGRNDTRACK_NONDYNAMIC], qtrue, fDesiredVolume);

	if (bNewTrackDesired)
	{
		S_StartBackgroundTrack( sMusic_BackgroundLoop, sMusic_BackgroundLoop, qfalse );
	}
}

cvar_t *s_soundpoolmegs = NULL;

// currently passing in sfx as a param in case I want to do something with it later.
//
byte *SND_malloc(int iSize, sfx_t *sfx)
{
	byte *pData = (byte *) Z_Malloc(iSize, TAG_SND_RAWDATA, qfalse);	// don't bother asking for zeroed mem

	// if "s_soundpoolmegs" is < 0, then the -ve of the value is the maximum amount of sounds we're allowed to have loaded...
	//
	if (s_soundpoolmegs && s_soundpoolmegs->integer < 0)
	{
		while ( (Z_MemSize(TAG_SND_RAWDATA) + Z_MemSize(TAG_SND_MP3STREAMHDR)) > ((-s_soundpoolmegs->integer) * 1024 * 1024))
		{
			int iBytesFreed = SND_FreeOldestSound(sfx);
			if (iBytesFreed == 0)
				break;	// sanity
		}
	}

	return pData;
}

// called once-only in EXE lifetime...
//
void SND_setup()
{
	s_soundpoolmegs = Cvar_Get("s_soundpoolmegs", "25", CVAR_ARCHIVE);
	if (Sys_LowPhysicalMemory() )
	{
		Cvar_Set("s_soundpoolmegs", "0");
	}

	Com_Printf("Sound memory manager started\n");
}

// ask how much mem an sfx has allocated...
//
static int SND_MemUsed(sfx_t *sfx)
{
	int iSize = 0;
	if (sfx->pSoundData){
		iSize += Z_Size(sfx->pSoundData);
	}

	if (sfx->pMP3StreamHeader) {
		iSize += Z_Size(sfx->pMP3StreamHeader);
	}

	return iSize;
}

// free any allocated sfx mem...
//
// now returns # bytes freed to help with z_malloc()-fail recovery
//
static int SND_FreeSFXMem(sfx_t *sfx)
{
	int iBytesFreed = 0;

	if (						sfx->pSoundData) {
		iBytesFreed +=	Z_Size(	sfx->pSoundData);
						Z_Free(	sfx->pSoundData );
								sfx->pSoundData = NULL;
	}

	sfx->bInMemory = qfalse;

	if (						sfx->pMP3StreamHeader) {
		iBytesFreed +=	Z_Size(	sfx->pMP3StreamHeader);
						Z_Free(	sfx->pMP3StreamHeader );
								sfx->pMP3StreamHeader = NULL;
	}

	return iBytesFreed;
}

void S_DisplayFreeMemory()
{
	int iSoundDataSize = Z_MemSize ( TAG_SND_RAWDATA ) + Z_MemSize( TAG_SND_MP3STREAMHDR );
	int iMusicDataSize = Z_MemSize ( TAG_SND_DYNAMICMUSIC );

	if (iSoundDataSize || iMusicDataSize)
	{
		Com_Printf("\n%.2fMB audio data:  ( %.2fMB WAV/MP3 ) + ( %.2fMB Music )\n",
					((float)(iSoundDataSize+iMusicDataSize))/1024.0f/1024.0f,
										((float)(iSoundDataSize))/1024.0f/1024.0f,
																((float)(iMusicDataSize))/1024.0f/1024.0f
					);

		// now count up amount used on this level...
		//
		iSoundDataSize = 0;
		for (int i=1; i<s_numSfx; i++)
		{
			sfx_t *sfx = &s_knownSfx[i];

			if (sfx->iLastLevelUsedOn == re->RegisterMedia_GetLevel()){
				iSoundDataSize += SND_MemUsed(sfx);
			}
		}

		Com_Printf("%.2fMB in sfx_t alloc data (WAV/MP3) loaded this level\n",(float)iSoundDataSize/1024.0f/1024.0f);
	}
}

void SND_TouchSFX(sfx_t *sfx)
{
	sfx->iLastTimeUsed		= Com_Milliseconds()+1;
	sfx->iLastLevelUsedOn	= re->RegisterMedia_GetLevel();
}

// currently this is only called during snd_shutdown or snd_restart
//
void S_FreeAllSFXMem(void)
{
	for (int i=1 ; i < s_numSfx ; i++)	// start @ 1 to skip freeing default sound
	{
		SND_FreeSFXMem(&s_knownSfx[i]);
	}
}

// returns number of bytes freed up...
//
// new param is so we can be usre of not freeing ourselves (without having to rely on possible uninitialised timers etc)
//
int SND_FreeOldestSound(sfx_t *pButNotThisOne /* = NULL */)
{
	return 0;
}

int SND_FreeOldestSound(void)
{
	return SND_FreeOldestSound(NULL);	// I had to add a void-arg version of this because of link issues, sigh
}

// just before we drop into a level, ensure the audio pool is under whatever the maximum
//	pool size is (but not by dropping out sounds used by the current level)...
//
// returns qtrue if at least one sound was dropped out, so z_malloc-fail recovery code knows if anything changed
//
extern qboolean gbInsideLoadSound;
qboolean SND_RegisterAudio_LevelLoadEnd(qboolean bDeleteEverythingNotUsedThisLevel /* 99% qfalse */)
{
	return qfalse;
}

