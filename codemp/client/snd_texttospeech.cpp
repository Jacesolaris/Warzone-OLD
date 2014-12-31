#include "client.h"
#include "snd_local.h"

//#define __WIN_TTS__ // Disable this when I get acapela remote voice chat system working...

//
// TODO: NON-WINDOWS SUPPORT (threading is really all I need to do)...
//

#ifdef _WIN32
#ifdef __WIN_TTS__
#define COBJMACROS
#include <sapi.h>
#include <ole2.h>

qboolean VOICE_INITIALIZED = qfalse;

#else //!__WIN_TTS__
#include <ole2.h>
extern size_t GetHttpPostData(char *address, char *poststr, char *recvdata);
#endif //__WIN_TTS__

void DoTextToSpeech (char* text, char *voice)
{
#ifdef __WIN_TTS__
	ISpVoice * pVoice = NULL;
	HRESULT hr;

	if (!VOICE_INITIALIZED)
	{
		if (FAILED(CoInitialize(NULL))) 
		{
			VOICE_INITIALIZED = qfalse;
			return;
		}
	}

	VOICE_INITIALIZED = qtrue;

	/*
	//Enumerate voice tokens with attribute "Name=Microsoft Sam"
	if(SUCCEEDED(hr))
	{
		hr = SpEnumTokens(SPCAT_VOICES, L"Name=Microsoft Sam", NULL, &cpEnum);
	}
	*/

	hr = CoCreateInstance(&CLSID_SpVoice, NULL, CLSCTX_ALL, &IID_ISpVoice, (void **)&pVoice);

	if( SUCCEEDED( hr ) )
	{
		wchar_t wtext[1024];
		LPWSTR ptr;

		mbstowcs(wtext, text, strlen(text)+1);//Plus null
		ptr = wtext;

		ISpVoice_SetRate(pVoice, -3);

		//
		// With COBJMACROS defined, you can do this 
		//
		//ISpVoice_SetRate(pVoice, 1);
		hr = ISpVoice_Speak(pVoice, ptr, 0, NULL);
		//
		// Otherwise, you have to go through the vtable manually
		//
		//     hr = pVoice->Speak((LPCWSTR)text, 0, NULL);
		ISpVoice_Release(pVoice);
		pVoice = NULL;
	}

	//CoUninitialize();

#else //!__WIN_TTS__

	int			i = 0;
	size_t		size = 0;
	char		RESPONSE[4096+1];
	char		POST_DATA[4096+1];
	qboolean	FOUND_HTTPS = qfalse;
	char		MP3_ADDRESS[256];
	int			MP3_ADDRESS_LENGTH = 0;

	//Com_Printf("Voice: %s. Text: %s.\n", voice, text);

	// url: acapela_tts, snd_url: '', voice: options.avoice, listen: '1', format: 'MP3', codecMP3: '1', spd: '180', vct: '100', text: '\\vct=100\\ \\spd=180\\ ' + text
	sprintf(POST_DATA, "&voice=%s&listen=1&format=MP3&codecMP3=1&spd=180&vct=100&text=\\vct=100\\ \\spd=180\\  %s", voice, text);
	size = GetHttpPostData("https://acapela-box.com/AcaBox/dovaas.php", POST_DATA, RESPONSE);

	//Com_Printf("Response the was:\n%s.\n", RESPONSE);

	for (i = 0; i < size; i++)
	{
		if (FOUND_HTTPS)
		{// Adding characters to address...
			if (RESPONSE[i] == '"') break; // finished address string...

			if (RESPONSE[i] == '\\') continue; // never add a \

			MP3_ADDRESS[MP3_ADDRESS_LENGTH] = RESPONSE[i];
			MP3_ADDRESS_LENGTH++;
		}
		else
		{// Looking for start of the text we want...
			if (RESPONSE[i] == 'h' && RESPONSE[i+1] == 't' && RESPONSE[i+2] == 't' && RESPONSE[i+3] == 'p' && RESPONSE[i+4] == 's')
			{
				FOUND_HTTPS = qtrue;
				MP3_ADDRESS[MP3_ADDRESS_LENGTH] = RESPONSE[i];
				MP3_ADDRESS_LENGTH++;
			}
		}
	}

	//Com_Printf("MP3 https address is:\n%s.\n", MP3_ADDRESS);
	
	BASS_StartStreamingSound( MP3_ADDRESS, -1, CHAN_LOCAL, NULL );
#endif //__WIN_TTS__
}

void ShutdownTextToSpeechThread ( void )
{
#ifdef __WIN_TTS__
	if (VOICE_INITIALIZED) CoUninitialize();
#endif //__WIN_TTS__
}

typedef struct ttsData_s
{
	char	voicename[32];		// Voice Name
	char	text[1024];			// Text
} ttsData_t;

DWORD WINAPI ThreadFunc(void *ttsInfoVoid) 
{
	ttsData_t *ttsInfo = (ttsData_t*)ttsInfoVoid;
	DoTextToSpeech(ttsInfo->text, ttsInfo->voicename);
	free(ttsInfoVoid);
	return 1;
}

void RemoveColorEscapeSequences( char *text ) {
	int i, l;

	l = 0;
	for ( i = 0; text[i]; i++ ) {
		if (Q_IsColorStringExt(&text[i])) {
			i++;
			continue;
		}
		if (text[i] > 0x7E)
			continue;
		text[l++] = text[i];
	}
	text[l] = '\0';
}

char PREVIOUS_TALK_TEXT[1024];
int  PREVIOUS_TALK_TIME = 0;

void S_TextToSpeech( const char *text, const char *voice )
{
	ttsData_t *ttsInfo;

	if (cl.serverTime != 0 && PREVIOUS_TALK_TIME >= cl.serverTime - 1000) return;

	ttsInfo = (ttsData_t *)malloc(sizeof(ttsData_t));

	// Remove color codes...
	memset(ttsInfo->voicename, '\0', sizeof(char)*32);
	strcpy(ttsInfo->voicename, voice);

	memset(ttsInfo->text, '\0', sizeof(char)*1024);
	strcpy(ttsInfo->text, text);
	RemoveColorEscapeSequences(ttsInfo->text);

	if (strcmp(ttsInfo->text, PREVIOUS_TALK_TEXT))
	{// Never repeat...
		HANDLE thread;
		memset(PREVIOUS_TALK_TEXT, '\0', sizeof(char)*1024);
		strcpy(PREVIOUS_TALK_TEXT, ttsInfo->text);
		PREVIOUS_TALK_TIME = cl.serverTime;
		thread = CreateThread(NULL, 0, ThreadFunc, ttsInfo, 0, NULL);
	}
	else
	{
		free(ttsInfo);
	}
}
#endif //_WIN32
