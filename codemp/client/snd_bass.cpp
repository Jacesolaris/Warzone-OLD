#include "client.h"
#include "snd_bass.h"

extern cvar_t	*s_khz;

qboolean EAX_SUPPORTED = qtrue;

// channel (sample/music) info structure
typedef struct {
	DWORD channel;			// the channel
	BASS_3DVECTOR pos,vel;	// position,velocity
} Channel;

Channel *chans=NULL;		// the channels
int chanc=0,chan=-1;		// number of channels, current channel

#define TIMERPERIOD	50		// timer period (ms)
#define MAXDIST		50		// maximum distance of the channels (m)
#define SPEED		12		// speed of the channels' movement (m/s)

void BASS_Update ( void )
{
	for (int c=0;c<chanc;c++) {
		// If the channel's playing then update it's position
		if (BASS_ChannelIsActive(chans[c].channel)==BASS_ACTIVE_PLAYING) {
			// Check if channel has reached the max distance
			if (chans[c].pos.z>=MAXDIST || chans[c].pos.z<=-MAXDIST)
				chans[c].vel.z=-chans[c].vel.z;
			if (chans[c].pos.x>=MAXDIST || chans[c].pos.x<=-MAXDIST)
				chans[c].vel.x=-chans[c].vel.x;
			// Update channel position
			chans[c].pos.z+=chans[c].vel.z*TIMERPERIOD/1000;
			chans[c].pos.x+=chans[c].vel.x*TIMERPERIOD/1000;
			BASS_ChannelSet3DPosition(chans[c].channel,&chans[c].pos,NULL,&chans[c].vel);
		}
	}
	// Apply the 3D changes
	BASS_Apply3D();
}

void BASS_AddStreamChanel ( char *file )
{
	DWORD newchan;

	// Load a music or sample from "file"
	if ((newchan=BASS_MusicLoad(FALSE,file,0,0,BASS_MUSIC_RAMP|BASS_SAMPLE_LOOP|BASS_SAMPLE_3D,0))
		|| (newchan=BASS_SampleLoad(FALSE,file,0,0,1,BASS_SAMPLE_LOOP|BASS_SAMPLE_3D|BASS_SAMPLE_MONO))) {
			Channel *c;
			chanc++;
			chans=(Channel*)realloc((void*)chans,chanc*sizeof(Channel));
			c=chans+chanc-1;
			memset(c,0,sizeof(Channel));
			c->channel=newchan;
			BASS_SampleGetChannel(newchan,FALSE); // initialize sample channel
			BASS_ChannelPlay(chans[chan].channel,FALSE);
	} else
		Com_Printf("Can't load file (note samples must be mono)\n");
}

void BASS_AddMemoryChanel ( char *memory, int length )
{
	DWORD newchan;

	// Load a music or sample from "file" (memory)
	if ((newchan=BASS_MusicLoad(TRUE,memory,0,(DWORD)length,BASS_MUSIC_RAMP|BASS_SAMPLE_3D,0))
		|| (newchan=BASS_SampleLoad(TRUE,memory,0,(DWORD)length,1,BASS_SAMPLE_3D|BASS_SAMPLE_MONO))) {
			Channel *c;
			chanc++;
			chans=(Channel*)realloc((void*)chans,chanc*sizeof(Channel));
			c=chans+chanc-1;
			memset(c,0,sizeof(Channel));
			c->channel=newchan;
			BASS_SampleGetChannel(newchan,FALSE); // initialize sample channel
			BASS_ChannelPlay(chans[chan].channel,FALSE);
	} else
		Com_Printf("Can't load sound memory (note samples must be mono)\n");
}

void BASS_RemoveChannel ( void )
{
	Channel *c=chans+chan;
	BASS_SampleFree(c->channel);
	BASS_MusicFree(c->channel);
	memmove(c,c+1,(chanc-chan-1)*sizeof(Channel));
	chanc--;
	chan=-1;
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

void BASS_Shutdown ( void )
{
	BASS_Free();
}

qboolean BASS_Init ( void )
{
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

	// Use meters as distance unit, real world rolloff, real doppler effect
	BASS_Set3DFactors(1,1,1);

	// Turn EAX off (volume=0), if error then EAX is not supported
	if (BASS_SetEAXParameters(-1,0,-1,-1))
		EAX_SUPPORTED = qfalse;

	Com_Printf("\n\n>>> BASS Sound System initialized OK! <<<\n\n\n");

	return qtrue;
}