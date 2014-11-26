// snd_mem.c: sound caching

#include "snd_local.h"

#include "snd_mp3.h"
#include "snd_ambient.h"

#include <string>

/*
===============================================================================

WAV loading

===============================================================================
*/

byte	*data_p;
byte 	*iff_end;
byte 	*last_chunk;
byte 	*iff_data;
int 	iff_chunk_len;
extern sfx_t		s_knownSfx[];
extern	int			s_numSfx;

extern cvar_t		*s_lip_threshold_1;
extern cvar_t		*s_lip_threshold_2;
extern cvar_t		*s_lip_threshold_3;
extern cvar_t		*s_lip_threshold_4;

short GetLittleShort(void)
{
	short val = 0;
	val = *data_p;
	val = (short)(val + (*(data_p+1)<<8));
	data_p += 2;
	return val;
}

int GetLittleLong(void)
{
	int val = 0;
	val = *data_p;
	val = val + (*(data_p+1)<<8);
	val = val + (*(data_p+2)<<16);
	val = val + (*(data_p+3)<<24);
	data_p += 4;
	return val;
}

void FindNextChunk(char *name)
{
	while (1)
	{
		data_p=last_chunk;

		if (data_p >= iff_end)
		{	// didn't find the chunk
			data_p = NULL;
			return;
		}

		data_p += 4;
		iff_chunk_len = GetLittleLong();
		if (iff_chunk_len < 0)
		{
			data_p = NULL;
			return;
		}
		data_p -= 8;
		last_chunk = data_p + 8 + ( (iff_chunk_len + 1) & ~1 );
		if (!Q_strncmp((char *)data_p, name, 4))
			return;
	}
}

void FindChunk(char *name)
{
	last_chunk = iff_data;
	FindNextChunk (name);
}


void DumpChunks(void)
{
	char	str[5];

	str[4] = 0;
	data_p=iff_data;
	do
	{
		memcpy (str, data_p, 4);
		data_p += 4;
		iff_chunk_len = GetLittleLong();
		Com_Printf ("0x%x : %s (%d)\n", (intptr_t)(data_p - 4), str, iff_chunk_len);
		data_p += (iff_chunk_len + 1) & ~1;
	} while (data_p < iff_end);
}

/*
============
GetWavinfo
============
*/
wavinfo_t GetWavinfo (const char *name, byte *wav, int wavlength)
{
	wavinfo_t	info;
	int		samples;

	memset (&info, 0, sizeof(info));

	if (!wav)
		return info;

	iff_data = wav;
	iff_end = wav + wavlength;

// find "RIFF" chunk
	FindChunk("RIFF");
	if (!(data_p && !Q_strncmp((char *)data_p+8, "WAVE", 4)))
	{
		Com_Printf("Missing RIFF/WAVE chunks\n");
		return info;
	}

// get "fmt " chunk
	iff_data = data_p + 12;
// DumpChunks ();

	FindChunk("fmt ");
	if (!data_p)
	{
		Com_Printf("Missing fmt chunk\n");
		return info;
	}
	data_p += 8;
	info.format = GetLittleShort();
	info.channels = GetLittleShort();
	info.rate = GetLittleLong();
	data_p += 4+2;
	info.width = GetLittleShort() / 8;

	if (info.format != 1)
	{
		Com_Printf("Microsoft PCM format only\n");
		return info;
	}


// find data chunk
	FindChunk("data");
	if (!data_p)
	{
		Com_Printf("Missing data chunk\n");
		return info;
	}

	data_p += 4;
	samples = GetLittleLong () / info.width;

	if (info.samples)
	{
		if (samples < info.samples)
			Com_Error (ERR_DROP, "Sound %s has a bad loop length", name);
	}
	else
		info.samples = samples;

	info.dataofs = data_p - wav;


	return info;
}


/*
================
ResampleSfx

resample / decimate to the current source rate
================
*/
void ResampleSfx (sfx_t *sfx, int iInRate, int iInWidth, byte *pData)
{
	int		iOutCount;
	int		iSrcSample;
	float	fStepScale;
	int		i;
	int		iSample;
	unsigned int uiSampleFrac, uiFracStep;	// uiSampleFrac MUST be unsigned, or large samples (eg music tracks) crash

	fStepScale = (float)iInRate / dma.speed;	// this is usually 0.5, 1, or 2

	// When stepscale is > 1 (we're downsampling), we really ought to run a low pass filter on the samples

	iOutCount = (int)(sfx->iSoundLengthInSamples / fStepScale);
	sfx->iSoundLengthInSamples = iOutCount;

	sfx->pSoundData = (short *) SND_malloc( sfx->iSoundLengthInSamples*2 ,sfx );

	sfx->fVolRange	= 0;
	uiSampleFrac	= 0;
	uiFracStep		= (int)(fStepScale*256);

	for (i=0 ; i<sfx->iSoundLengthInSamples ; i++)
	{
		iSrcSample = uiSampleFrac >> 8;
		uiSampleFrac += uiFracStep;
		if (iInWidth == 2) {
			iSample = LittleShort ( ((short *)pData)[iSrcSample] );
		} else {
			iSample = (int)( (unsigned char)(pData[iSrcSample]) - 128) << 8;
		}

		sfx->pSoundData[i] = (short)iSample;

		// work out max vol for this sample...
		//
		if (iSample < 0)
			iSample = -iSample;
		if (sfx->fVolRange < (iSample >> 8) )
		{
			sfx->fVolRange =  iSample >> 8;
		}
	}
}


//=============================================================================


void S_LoadSound_Finalize(wavinfo_t	*info, sfx_t *sfx, byte *data)
{
	float	stepscale	= (float)info->rate / dma.speed;
	int		len			= (int)(info->samples / stepscale);

	len *= info->width;

	sfx->eSoundCompressionMethod = ct_16;
	sfx->iSoundLengthInSamples	 = info->samples;
	ResampleSfx( sfx, info->rate, info->width, data + info->dataofs );
}





// maybe I'm re-inventing the wheel, here, but I can't see any functions that already do this, so...
//
char *Filename_WithoutPath(const char *psFilename)
{
	static char sString[MAX_QPATH];	// !!
	const char *p = strrchr(psFilename,'\\');

  	if (!p++)
		p=psFilename;

	strcpy(sString,p);

	return sString;

}

// returns (eg) "\dir\name" for "\dir\name.bmp"
//
char *Filename_WithoutExt(const char *psFilename)
{
	static char sString[MAX_QPATH];	// !

	strcpy(sString,psFilename);

	char *p = strrchr(sString,'.');
	char *p2= strrchr(sString,'\\');

	// special check, make sure the first suffix we found from the end wasn't just a directory suffix (eg on a path'd filename with no extension anyway)
	//
	if (p && (p2==0 || (p2 && p>p2)))
		*p=0;

	return sString;
}


// adjust filename for foreign languages and WAV/MP3/OGG issues.
//
// returns qfalse if failed to load, else fills in *pData
//
extern	cvar_t	*com_buildScript;
static qboolean S_LoadSound_FileLoadAndNameAdjuster(char *psFilename, byte **pData, int *piSize, int iNameStrlen)
{
	char *psVoice = strstr(psFilename,"chars");
	if (psVoice)
	{
		// cache foreign voices...
		//
		if (com_buildScript->integer)
		{
			fileHandle_t hFile;
			//German
			strncpy(psVoice,"chr_d",5);	// same number of letters as "chars"
			FS_FOpenFileRead(psFilename, &hFile, qfalse);		//cache the wav
			if (!hFile)
			{
				strcpy(&psFilename[iNameStrlen-3],"mp3");		//not there try mp3
				FS_FOpenFileRead(psFilename, &hFile, qfalse);	//cache the mp3
			}
			if (!hFile)
			{
				strcpy(&psFilename[iNameStrlen-3],"ogg");		//not there try ogg
				FS_FOpenFileRead(psFilename, &hFile, qfalse);	//cache the ogg
			}
			if (hFile)
			{
				FS_FCloseFile(hFile);
			}
			strcpy(&psFilename[iNameStrlen-3],"wav");	//put it back to wav

			//French
			strncpy(psVoice,"chr_f",5);	// same number of letters as "chars"
			FS_FOpenFileRead(psFilename, &hFile, qfalse);		//cache the wav
			if (!hFile)
			{
				strcpy(&psFilename[iNameStrlen-3],"mp3");		//not there try mp3
				FS_FOpenFileRead(psFilename, &hFile, qfalse);	//cache the mp3
			}
			if (!hFile)
			{
				strcpy(&psFilename[iNameStrlen-3],"ogg");		//not there try ogg
				FS_FOpenFileRead(psFilename, &hFile, qfalse);	//cache the ogg
			}
			if (hFile)
			{
				FS_FCloseFile(hFile);
			}
			strcpy(&psFilename[iNameStrlen-3],"wav");	//put it back to wav

			//Spanish
			strncpy(psVoice,"chr_e",5);	// same number of letters as "chars"
			FS_FOpenFileRead(psFilename, &hFile, qfalse);		//cache the wav
			if (!hFile)
			{
				strcpy(&psFilename[iNameStrlen-3],"mp3");		//not there try mp3
				FS_FOpenFileRead(psFilename, &hFile, qfalse);	//cache the mp3
			}
			if (!hFile)
			{
				strcpy(&psFilename[iNameStrlen-3],"ogg");		//not there try ogg
				FS_FOpenFileRead(psFilename, &hFile, qfalse);	//cache the ogg
			}
			if (hFile)
			{
				FS_FCloseFile(hFile);
			}
			strcpy(&psFilename[iNameStrlen-3],"wav");	//put it back to wav

			strncpy(psVoice,"chars",5);	//put it back to chars
		}

		// account for foreign voices...
		//
		extern cvar_t* s_language;
		if ( s_language ) {
				 if ( !Q_stricmp( "DEUTSCH", s_language->string ) )
				strncpy( psVoice, "chr_d", 5 );	// same number of letters as "chars"
			else if ( !Q_stricmp( "FRANCAIS", s_language->string ) )
				strncpy( psVoice, "chr_f", 5 );	// same number of letters as "chars"
			else if ( !Q_stricmp( "ESPANOL", s_language->string ) )
				strncpy( psVoice, "chr_e", 5 );	// same number of letters as "chars"
			else
				psVoice = NULL;
		}
		else
		{
			psVoice = NULL;	// use this ptr as a flag as to whether or not we substituted with a foreign version
		}
	}

	//
	// WAV Support...
	//
	*piSize = FS_ReadFile( psFilename, (void **)pData );	// try WAV
	
	//
	// MP3 Support...
	//
	if ( !*pData ) {
		psFilename[iNameStrlen-3] = 'm';
		psFilename[iNameStrlen-2] = 'p';
		psFilename[iNameStrlen-1] = '3';
		*piSize = FS_ReadFile( psFilename, (void **)pData );	// try MP3

		if ( !*pData )
		{
			//hmmm, not found, ok, maybe we were trying a foreign noise ("arghhhhh.mp3" that doesn't matter?) but it
			// was missing?   Can't tell really, since both types are now in sound/chars. Oh well, fall back to English for now...

			if (psVoice)	// were we trying to load foreign?
			{
				// yep, so fallback to re-try the english...
				//
#ifndef FINAL_BUILD
				Com_Printf(S_COLOR_YELLOW "Foreign file missing: \"%s\"! (using English...)\n",psFilename);
#endif

				strncpy(psVoice,"chars",5);

				psFilename[iNameStrlen-3] = 'w';
				psFilename[iNameStrlen-2] = 'a';
				psFilename[iNameStrlen-1] = 'v';
				*piSize = FS_ReadFile( psFilename, (void **)pData );	// try English WAV
				if ( !*pData )
				{
					psFilename[iNameStrlen-3] = 'm';
					psFilename[iNameStrlen-2] = 'p';
					psFilename[iNameStrlen-1] = '3';
					*piSize = FS_ReadFile( psFilename, (void **)pData );	// try English MP3
				}
			}
		}
	}

	//
	// OGG Support...
	//
	if ( !*pData ) {
		psFilename[iNameStrlen-3] = 'o';
		psFilename[iNameStrlen-2] = 'g';
		psFilename[iNameStrlen-1] = 'g';
		*piSize = FS_ReadFile( psFilename, (void **)pData );	// try OGG

		if ( !*pData )
		{
			//hmmm, not found, ok, maybe we were trying a foreign noise ("arghhhhh.mp3" that doesn't matter?) but it
			// was missing?   Can't tell really, since both types are now in sound/chars. Oh well, fall back to English for now...

			if (psVoice)	// were we trying to load foreign?
			{
				// yep, so fallback to re-try the english...
				//
#ifndef FINAL_BUILD
				Com_Printf(S_COLOR_YELLOW "Foreign file missing: \"%s\"! (using English...)\n",psFilename);
#endif

				strncpy(psVoice,"chars",5);

				psFilename[iNameStrlen-3] = 'o';
				psFilename[iNameStrlen-2] = 'g';
				psFilename[iNameStrlen-1] = 'g';
				*piSize = FS_ReadFile( psFilename, (void **)pData );	// try English OGG
			}
		}
	}

	if (!*pData)
	{
		return qfalse;	// sod it, give up...
	}

	return qtrue;
}

// returns qtrue if this dir is allowed to keep loaded MP3s, else qfalse if they should be WAV'd instead...
//
// note that this is passed the original, un-language'd name
//
// (I was going to remove this, but on kejim_post I hit an assert because someone had got an ambient sound when the
//	perimter fence goes online that was an MP3, then it tried to get added as looping. Presumably it sounded ok or
//	they'd have noticed, but we therefore need to stop other levels using those. "sound/ambience" I can check for,
//	but doors etc could be anything. Sigh...)
//
#define SOUND_CHARS_DIR "sound/chars/"
#define SOUND_CHARS_DIR_LENGTH 12 // strlen( SOUND_CHARS_DIR )
static qboolean S_LoadSound_DirIsAllowedToKeepMP3s(const char *psFilename)
{
	if ( Q_stricmpn( psFilename, SOUND_CHARS_DIR, SOUND_CHARS_DIR_LENGTH ) == 0 )
		return qtrue;	// found a dir that's allowed to keep MP3s

	return qfalse;
}

/*
==============
S_LoadSound

The filename may be different than sfx->name in the case
of a forced fallback of a player specific sound	(or of a wav/mp3 substitution now -Ste)
==============
*/
qboolean gbInsideLoadSound = qfalse;
static qboolean S_LoadSound_Actual( sfx_t *sfx )
{
	char	*psExt;
	char	sLoadName[MAX_QPATH];

	int		len = strlen(sfx->sSoundName);
	if (len<5)
	{
		return qfalse;
	}

	// player specific sounds are never directly loaded...
	//
	if ( sfx->sSoundName[0] == '*') {
		return qfalse;
	}
	// make up a local filename to try wav/mp3 substitutes...
	//
	Q_strncpyz(sLoadName, sfx->sSoundName, sizeof(sLoadName));
	Q_strlwr( sLoadName );
	//
	// Ensure name has an extension (which it must have, but you never know), and get ptr to it...
	//
	psExt = &sLoadName[strlen(sLoadName)-4];
	if (*psExt != '.')
	{
		//Com_Printf( "WARNING: soundname '%s' does not have 3-letter extension\n",sLoadName);
		COM_DefaultExtension(sLoadName,sizeof(sLoadName),".wav");	// so psExt below is always valid
		psExt = &sLoadName[strlen(sLoadName)-4];
		len = strlen(sLoadName);
	}

	if (!S_LoadSound_FileLoadAndNameAdjuster(sLoadName, &sfx->indexData, &sfx->indexSize, len))
	{
		//Com_Printf("BASS: Failed to load sound [ID %i] %s from memory.\n", sfx->qhandle, sLoadName);
		sfx->bassSampleID = s_knownSfx[0].bassSampleID; // link to default sound bass ID...
		sfx->bDefaultSound = qtrue;
		return qfalse;
	}

	SND_TouchSFX(sfx);

	{// Load whole sound file into ram...
		qboolean		isMusic = qfalse;

		if (Q_stricmpn(psExt,".mp3",4) == 0 && !MP3_IsValid(sLoadName, sfx->indexData, sfx->indexSize, qfalse)) isMusic = qtrue;

		if (!isMusic)
			sfx->bassSampleID = BASS_LoadMemorySample( sfx->indexData, sfx->indexSize );
		else
			sfx->bassSampleID = BASS_LoadMusicSample( sfx->indexData, sfx->indexSize );

		sfx->qhandle = sfx - s_knownSfx;

		if (sfx->bassSampleID < 0) {
			Com_Printf("BASS: Failed to load sound [ID %i] %s from memory.\n", sfx->qhandle, sLoadName);
			sfx->bassSampleID = s_knownSfx[0].bassSampleID; // link to default sound bass ID...
			sfx->bDefaultSound = qtrue;
			FS_FreeFile( sfx->indexData );
			//free( sfx->indexData );
			return qfalse;
		}
	}

	//Com_Printf("BASS: Registered sound [ID %i] %s. Length %i.\n", sfx->qhandle, sLoadName, sfx->indexSize);

	FS_FreeFile( sfx->indexData );
	//free( sfx->indexData );
	return qtrue;
}


// wrapper function for above so I can guarantee that we don't attempt any audio-dumping during this call because
//	of a z_malloc() fail recovery...
//
qboolean S_LoadSound( sfx_t *sfx )
{
	gbInsideLoadSound = qtrue;	// !!!!!!!!!!!!!

	qboolean bReturn = S_LoadSound_Actual( sfx );

	gbInsideLoadSound = qfalse;	// !!!!!!!!!!!!!

	return bReturn;
}


