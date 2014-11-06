// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_consolecmds.c -- text commands typed in at the local console, or
// executed by a key binding

#include "cg_local.h"
#include "game/bg_saga.h"
//#include "game/bg_class.h"

/*
=================
CG_TargetCommand_f

=================
*/
void CG_TargetCommand_f( void ) {
	int		targetNum;
	char	test[4];

	targetNum = CG_CrosshairPlayer();
	if ( targetNum == -1 ) {
		return;
	}

	trap->Cmd_Argv( 1, test, 4 );
	trap->SendClientCommand( va( "gc %i %i", targetNum, atoi( test ) ) );
}

/*
=================
CG_SizeUp_f

Keybinding command
=================
*/
static void CG_SizeUp_f (void) {
	trap->Cvar_Set( "cg_viewsize", va( "%i", cg_viewsize.integer + 10 ) );
}

/*
=================
CG_SizeDown_f

Keybinding command
=================
*/
static void CG_SizeDown_f (void) {
	trap->Cvar_Set( "cg_viewsize", va( "%i", cg_viewsize.integer - 10 ) );
}

/*
=============
CG_Viewpos_f

Debugging command to print the current position
=============
*/
static void CG_Viewpos_f (void) {
	trap->Print ("%s (%i %i %i) : %i\n", cgs.mapname, (int)cg.refdef.vieworg[0],
		(int)cg.refdef.vieworg[1], (int)cg.refdef.vieworg[2],
		(int)cg.refdef.viewangles[YAW]);
}

/*
=================
CG_ScoresDown_f

=================
*/
static void CG_ScoresDown_f( void ) {

	CG_BuildSpectatorString();
	if ( cg.scoresRequestTime + 2000 < cg.time ) {
		// the scores are more than two seconds out of data,
		// so request new ones
		cg.scoresRequestTime = cg.time;
		trap->SendClientCommand( "score" );

		// leave the current scores up if they were already
		// displayed, but if this is the first hit, clear them out
		if ( !cg.showScores ) {
			cg.showScores = qtrue;
			cg.numScores = 0;
		}
	} else {
		// show the cached contents even if they just pressed if it
		// is within two seconds
		cg.showScores = qtrue;
	}
}

/*
=================
CG_ScoresUp_f

=================
*/
static void CG_ScoresUp_f( void ) {
	if ( cg.showScores ) {
		cg.showScores = qfalse;
		cg.scoreFadeTime = cg.time;
	}
}

void CG_ClientList_f( void )
{
	clientInfo_t *ci;
	int i;
	int count = 0;

	for( i = 0; i < MAX_CLIENTS; i++ )
	{
		ci = &cgs.clientinfo[ i ];
		if( !ci->infoValid )
			continue;

		switch( ci->team )
		{
		case TEAM_FREE:
			Com_Printf( "%2d " S_COLOR_YELLOW "F   " S_COLOR_WHITE "%s" S_COLOR_WHITE "%s\n", i, ci->name, (ci->botSkill != -1) ? " (bot)" : "" );
			break;

		case TEAM_RED:
			Com_Printf( "%2d " S_COLOR_RED "R   " S_COLOR_WHITE "%s" S_COLOR_WHITE "%s\n", i,
				ci->name, (ci->botSkill != -1) ? " (bot)" : "" );
			break;

		case TEAM_BLUE:
			Com_Printf( "%2d " S_COLOR_BLUE "B   " S_COLOR_WHITE "%s" S_COLOR_WHITE "%s\n", i,
				ci->name, (ci->botSkill != -1) ? " (bot)" : "" );
			break;

		default:
		case TEAM_SPECTATOR:
			Com_Printf( "%2d " S_COLOR_YELLOW "S   " S_COLOR_WHITE "%s" S_COLOR_WHITE "%s\n", i, ci->name, (ci->botSkill != -1) ? " (bot)" : "" );
			break;
		}

		count++;
	}

	Com_Printf( "Listed %2d clients\n", count );
}


static void CG_TellTarget_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_CrosshairPlayer();
	if ( clientNum == -1 ) {
		return;
	}

	trap->Cmd_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap->SendClientCommand( command );
}

static void CG_TellAttacker_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_LastAttacker();
	if ( clientNum == -1 ) {
		return;
	}

	trap->Cmd_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap->SendClientCommand( command );
}

/*
==================
CG_StartOrbit_f
==================
*/

static void CG_StartOrbit_f( void ) {
	char var[MAX_TOKEN_CHARS];

	trap->Cvar_VariableStringBuffer( "developer", var, sizeof( var ) );
	if ( !atoi(var) ) {
		return;
	}
	if (cg_cameraOrbit.value != 0) {
		trap->Cvar_Set ("cg_cameraOrbit", "0");
		trap->Cvar_Set("cg_thirdPerson", "0");
	} else {
		trap->Cvar_Set("cg_cameraOrbit", "5");
		trap->Cvar_Set("cg_thirdPerson", "1");
		trap->Cvar_Set("cg_thirdPersonAngle", "0");
		trap->Cvar_Set("cg_thirdPersonRange", "100");
	}
}

void CG_SiegeBriefingDisplay(int team, int dontshow);
static void CG_SiegeBriefing_f(void)
{
	int team;

	if (cgs.gametype != GT_SIEGE)
	{ //Cannot be displayed unless in this gametype
		return;
	}

	team = cg.predictedPlayerState.persistant[PERS_TEAM];

	if (team != SIEGETEAM_TEAM1 &&
		team != SIEGETEAM_TEAM2)
	{ //cannot be displayed if not on a valid team
		return;
	}

	CG_SiegeBriefingDisplay(team, 0);
}

static void CG_SiegeCvarUpdate_f(void)
{
	int team;

	if (cgs.gametype != GT_SIEGE)
	{ //Cannot be displayed unless in this gametype
		return;
	}

	team = cg.predictedPlayerState.persistant[PERS_TEAM];

	if (team != SIEGETEAM_TEAM1 &&
		team != SIEGETEAM_TEAM2)
	{ //cannot be displayed if not on a valid team
		return;
	}

	CG_SiegeBriefingDisplay(team, 1);
}

static void CG_SiegeCompleteCvarUpdate_f(void)
{
	if (cgs.gametype != GT_SIEGE)
	{ //Cannot be displayed unless in this gametype
		return;
	}

	// Set up cvars for both teams
	CG_SiegeBriefingDisplay(SIEGETEAM_TEAM1, 1);
	CG_SiegeBriefingDisplay(SIEGETEAM_TEAM2, 1);
}

//[AUTOWAYPOINT]
extern void AIMod_AutoWaypoint ( void );
extern void AIMod_AutoWaypoint_Clean ( void );
extern void AIMod_MarkBadHeight ( void );
extern void AIMod_AddRemovalPoint ( void );
extern void AIMod_AddLiftPoint ( void );
extern void AIMod_AWC_MarkBadHeight ( void );
extern void AIMod_AddWayPoint ( void );
extern void CG_ShowSurface ( void );
extern void CG_ShowSlope ( void );

void CG_ShowLifts ( void )
{
	int i = 0;
	int count = 0;

	for (i = 0; i < MAX_GENTITIES; i++)
	{
		centity_t *cent = &cg_entities[i];

		if (!cent) continue;
		if (cent->currentState.eType != ET_MOVER_MARKER) continue;
		
		count++;
		trap->Print("Mover found at %f %f %f (top %f %f %f).\n", cent->currentState.origin[0], cent->currentState.origin[1], cent->currentState.origin[2]
		, cent->currentState.origin2[0], cent->currentState.origin2[1], cent->currentState.origin2[2]);
	}

	trap->Print("There are %i movers.\n", count);
}
//[/AUTOWAYPOINT]

#ifdef _WIN32
#define COBJMACROS
#include <sapi.h>
#include <ole2.h>

qboolean VOICE_INITIALIZED = qfalse;

void DoTextToSpeech (char* text)
{
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
}

void ShutdownTextToSpeechThread ( void )
{
	if (VOICE_INITIALIZED) CoUninitialize();
}

DWORD WINAPI ThreadFunc(void* text) {
	DoTextToSpeech((char *)text);
	return 1;
}

char PREVIOUS_TALK_TEXT[1024];
int  PREVIOUS_TALK_TIME = 0;

void TextToSpeech( char *text )
{
	if (cg.time != 0 && PREVIOUS_TALK_TIME >= cg.time - 1000) return;

	if (strcmp(text, PREVIOUS_TALK_TEXT))
	{// Never repeat...
		HANDLE thread;
		memset(PREVIOUS_TALK_TEXT, '\0', sizeof(char)*1024);
		strcpy(PREVIOUS_TALK_TEXT, text);
		PREVIOUS_TALK_TIME = cg.time;
		thread = CreateThread(NULL, 0, ThreadFunc, PREVIOUS_TALK_TEXT, 0, NULL);
	}
}
#endif //_WIN32

void CG_SaySillyTextTest ( void )
{
#ifdef _WIN32
	int choice = irand(0,10);

	switch (choice)
	{
	case 1:
		TextToSpeech("What the fuck are you doing???");
		break;
	case 2:
		TextToSpeech("Stop that!");
		break;
	case 3:
		TextToSpeech("Hay, stop it!");
		break;
	case 4:
		TextToSpeech("Get away from me!");
		break;
	case 5:
		TextToSpeech("How much wood wood a wood chuck chuck if a wood chuck could chuck wood?");
		break;
	case 6:
		TextToSpeech("What are you doing?");
		break;
	case 7:
		TextToSpeech("Don't talk to me.");
		break;
	case 8:
		TextToSpeech("Go away!");
		break;
	case 9:
		TextToSpeech("Ouch! That hurt!");
		break;
	default:
		TextToSpeech("Oh meye!");
		break;
	}
#endif //_WIN32
}

void TTS_SayText ( void )
{
	char	str[MAX_TOKEN_CHARS];
	
	if ( trap->Cmd_Argc() < 2 )
	{
		trap->Print( "^4*** ^3TTS^4: ^7Usage:\n" );
		trap->Print( "^4*** ^3TTS^4: ^3/tts \"text\"^5.\n" );
		trap->UpdateScreen();
		return;
	}

	trap->Cmd_Argv( 1, str, sizeof(str) );

	TextToSpeech(str);
}

typedef struct consoleCommand_s {
	const char	*cmd;
	void		(*func)(void);
} consoleCommand_t;

int cmdcmp( const void *a, const void *b ) {
	return Q_stricmp( (const char *)a, ((consoleCommand_t*)b)->cmd );
}

/* This array MUST be sorted correctly by alphabetical name field */
static consoleCommand_t	commands[] = {
	{ "+scores",					CG_ScoresDown_f },
	{ "-scores",					CG_ScoresUp_f },
	{ "adw",						AIMod_AddWayPoint },
	{ "autowaypoint",				AIMod_AutoWaypoint },
	{ "autowaypointclean",			AIMod_AutoWaypoint_Clean },
	{ "awc",						AIMod_AutoWaypoint_Clean },
	{ "awc_addbadheight",			AIMod_AWC_MarkBadHeight },
	{ "awc_addlift",				AIMod_AddLiftPoint },
	{ "awc_addremovalspot",			AIMod_AddRemovalPoint },
	{ "aw_badheight",				AIMod_MarkBadHeight },
	{ "aw_addwaypoint",				AIMod_AddWayPoint },
	{ "awp",						AIMod_AutoWaypoint },
	{ "briefing",					CG_SiegeBriefing_f },
	{ "clientlist",					CG_ClientList_f },
	{ "forcenext",					CG_NextForcePower_f },
	{ "forceprev",					CG_PrevForcePower_f },
	{ "invnext",					CG_NextInventory_f },
	{ "invprev",					CG_PrevInventory_f },
	{ "loaddeferred",				CG_LoadDeferredPlayers },
	{ "nextframe",					CG_TestModelNextFrame_f },
	{ "nextskin",					CG_TestModelNextSkin_f },
	{ "prevframe",					CG_TestModelPrevFrame_f },
	{ "prevskin",					CG_TestModelPrevSkin_f },
	{ "showlifts",					CG_ShowLifts },
	{ "showslope",					CG_ShowSlope },
	{ "showsurface",				CG_ShowSurface },
	{ "siegeCompleteCvarUpdate",	CG_SiegeCompleteCvarUpdate_f },
	{ "siegeCvarUpdate",			CG_SiegeCvarUpdate_f },
	{ "sizedown",					CG_SizeDown_f },
	{ "sizeup",						CG_SizeUp_f },
	{ "startOrbit",					CG_StartOrbit_f },
	{ "tcmd",						CG_TargetCommand_f },
	{ "tell_attacker",				CG_TellAttacker_f },
	{ "tell_target",				CG_TellTarget_f },
	{ "testgun",					CG_TestGun_f },
	{ "testmodel",					CG_TestModel_f },
	{ "tts",						TTS_SayText },
	{ "viewpos",					CG_Viewpos_f },
	{ "weapnext",					CG_NextWeapon_f },
	{ "weapon",						CG_Weapon_f },
	{ "weaponclean",				CG_WeaponClean_f },
	{ "weapprev",					CG_PrevWeapon_f },
	{ "zzz",						CG_SaySillyTextTest },
};

static const size_t numCommands = ARRAY_LEN( commands );

/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
qboolean CG_ConsoleCommand( void ) {
	consoleCommand_t	*command = NULL;

	command = (consoleCommand_t *)bsearch( CG_Argv( 0 ), commands, numCommands, sizeof( commands[0] ), cmdcmp );

	if ( !command )
		return qfalse;

	command->func();
	return qtrue;
}

static const char *gcmds[] = {
	"addbot",
	"callteamvote",
	"callvote",
	"duelteam",
	"follow",
	"follownext",
	"followprev",
	"forcechanged",
	"give",
	"god",
	"kill",
	"levelshot",
	"loaddefered",
	"noclip",
	"notarget",
	"NPC",
	"say",
	"say_team",
	"setviewpos",
	"siegeclass",
	"stats",
	//"stopfollow",
	"team",
	"teamtask",
	"teamvote",
	"tell",
	"voice_cmd",
	"vote",
	"where",
	"zoom"
};
static const size_t numgcmds = ARRAY_LEN( gcmds );

/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
*/
void CG_InitConsoleCommands( void ) {
	size_t i;

	for ( i = 0 ; i < numCommands ; i++ )
		trap->AddCommand( commands[i].cmd );

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	for( i = 0; i < numgcmds; i++ )
		trap->AddCommand( gcmds[i] );
}
