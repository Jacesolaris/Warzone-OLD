#include "cg_local.h"

//
// TTS Voice Types Tracking Stuff...
//
typedef enum
{
	TTS_GENDER_NONE,
	TTS_GENDER_MALE,
	TTS_GENDER_FEMALE,
	TTS_GENDER_DROID,
	TTS_GENDER_YODA,
	TTS_GENDER_IMPERIAL_OFFICER,
	TTS_GENDER_BOUNTY_HUNTER_MALE,
	TTS_GENDER_BOUNTY_HUNTER_FEMALE,
	TTS_GENDER_EVIL_MALE,
	TTS_GENDER_EVIL_FEMALE,
	TTS_GENDER_MAX
} genders_t;

typedef enum
{
	TTS_AGE_NONE, // Only use to mark the 1st record in the list!
	TTS_AGE_ADULT,
	TTS_AGE_OLD,
	TTS_AGE_CHILD,
	TTS_AGE_MAX
} TTS_AGE_t;

typedef struct ttsVoiceData_s
{
	char	voicename[32];		// Voice Name
	char	description[128];	// Full Voice Description
	int		gender;
	int		age;
} ttsVoiceData_t;

//#define __OTHER_LANGUAGES_SUPPORT__ // UQ1: I can't be bothered with other languages. Too much work for now...

ttsVoiceData_t ttsVoiceData[] = {
	// char	voicename[32];	char description[128], char	gender[2], char	age[32]
	"unused", "unused voice", TTS_GENDER_NONE, TTS_AGE_NONE,
#ifdef __OTHER_LANGUAGES_SUPPORT__
	"leila22k", "Arabic (SA) - Leila", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"mehdi22k", "Arabic (SA) - Mehdi", TTS_GENDER_MALE, TTS_AGE_ADULT,
	"nizar22k", "Arabic (SA) - Nizar", TTS_GENDER_MALE, TTS_AGE_ADULT,
	"salma22k", "Arabic (SA) - Salma", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"laia22k", "Catalan - Laia", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"lulu22k", "Chinese (Mandarin) - Lulu", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"eliska22k", "Czech - Eliska", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"mette22k", "Danish - Mette", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"rasmus22k", "Danish - Rasmus", TTS_GENDER_MALE, TTS_AGE_ADULT,
	"jeroen22k", "Dutch (BE) - Jeroen", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"jeroenhappy22k", "Dutch (BE) - Jeroen (Happy)", TTS_GENDER_MALE, TTS_AGE_ADULT,
	"jeroensad22k", "Dutch (BE) - Jeroen (Sad)", TTS_GENDER_MALE, TTS_AGE_ADULT,
	"sofie22k", "Dutch (BE) - Sofie", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"zoe22k", "Dutch (BE) - Zoe", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"daan22k", "Dutch (NL) - Daan", TTS_GENDER_MALE, TTS_AGE_ADULT,
	"femke22k", "Dutch (NL) - Femke", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"jasmijn22k", "Dutch (NL) - Jasmijn", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"max22k", "Dutch (NL) - Max", TTS_GENDER_MALE, TTS_AGE_ADULT,
	"nizareng22k", "English (Arabic) - Nizar", TTS_GENDER_MALE, TTS_AGE_ADULT,
#endif //__OTHER_LANGUAGES_SUPPORT__
	"liam22k", "English (Australia) - Liam (Child) - Premium", TTS_GENDER_MALE, TTS_AGE_CHILD,
	"lisa22k", "English (Australia) - Lisa", TTS_GENDER_BOUNTY_HUNTER_FEMALE, TTS_AGE_ADULT,
	"olivia22k", "English (Australia) - Olivia (Child) - Premium", TTS_GENDER_FEMALE, TTS_AGE_CHILD,
	"tyler22k", "English (Australia) - Tyler", TTS_GENDER_BOUNTY_HUNTER_MALE, TTS_AGE_ADULT,
#ifdef __OTHER_LANGUAGES_SUPPORT__ // UQ1: Hmm... Might still use this one later...
	"deepa22k", "English (India) - Deepa", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
#endif //__OTHER_LANGUAGES_SUPPORT__
	"graham22k", "English (UK) - Graham", TTS_GENDER_IMPERIAL_OFFICER, TTS_AGE_ADULT,
	"harry22k", "English (UK) - Harry (Child) - Premium", TTS_GENDER_MALE, TTS_AGE_CHILD,
	"lucy22k", "English (UK) - Lucy", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"peter22k", "English (UK) - Peter", TTS_GENDER_IMPERIAL_OFFICER, TTS_AGE_ADULT,
	"peterhappy22k", "English (UK) - Peter (Happy)", TTS_GENDER_IMPERIAL_OFFICER, TTS_AGE_ADULT,
	"petersad22k", "English (UK) - Peter (Sad)", TTS_GENDER_IMPERIAL_OFFICER, TTS_AGE_ADULT,
	"queenelizabeth22k", "English (UK) - Queen Elizabeth", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"rachel22k", "English (UK) - Rachel", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"rosie22k", "English (UK) - Rosie (Child) - Premium", TTS_GENDER_FEMALE, TTS_AGE_CHILD,
	"ella22k", "English (US) - Ella (Child) - Premium", TTS_GENDER_FEMALE, TTS_AGE_CHILD,
	"heather22k", "English (US) - Heather", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"josh22k", "English (US) - Josh (Child) - Premium", TTS_GENDER_MALE, TTS_AGE_CHILD,
	"kenny22k", "English (US) - Kenny", TTS_GENDER_MALE, TTS_AGE_ADULT,
	"laura22k", "English (US) - Laura", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"micah22k", "English (US) - Micah", TTS_GENDER_MALE, TTS_AGE_ADULT,
	"nelly22k", "English (US) - Nelly", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"rod22k", "English (US) - Rod", TTS_GENDER_MALE, TTS_AGE_ADULT,
	"ryan22k", "English (US) - Ryan", TTS_GENDER_MALE, TTS_AGE_ADULT,
	"saul22k", "English (US) - Saul", TTS_GENDER_MALE, TTS_AGE_ADULT,
	"scott22k", "English (US) - Scott (Child) - Premium", TTS_GENDER_MALE, TTS_AGE_CHILD,
	"tracy22k", "English (US) - Tracy", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"will22k", "English (US) - Will", TTS_GENDER_MALE, TTS_AGE_ADULT,
	"willbadguy22k", "English (US) - Will (BadGuy)", TTS_GENDER_EVIL_MALE, TTS_AGE_ADULT,
	"willfromafar22k", "English (US) - Will (FromAfar)", TTS_GENDER_MALE, TTS_AGE_ADULT,
	"willhappy22k", "English (US) - Will (Happy)", TTS_GENDER_MALE, TTS_AGE_ADULT,
	"willlittlecreature22k", "English (US) - Will (LittleCreature)", TTS_GENDER_YODA, TTS_AGE_ADULT,
	"willoldman22k", "English (US) - Will (Old Man)", TTS_GENDER_MALE, TTS_AGE_OLD,
	"willsad22k", "English (US) - Will (Sad)", TTS_GENDER_MALE, TTS_AGE_ADULT,
	"willupclose22k", "English (US) - Will (UpClose)", TTS_GENDER_MALE, TTS_AGE_ADULT,
#ifdef __OTHER_LANGUAGES_SUPPORT__
	"sanna22k", "Finnish - Sanna", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"justine22k", "French (BE) - Justine", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"louise22k", "French (Canada) - Louise", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"alice22k", "French - Alice", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"antoine22k", "French - Antoine", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"antoinefromafar22k", "French - Antoine (FromAfar)", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"antoinehappy22k", "French - Antoine (Happy)", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"antoinesad22k", "French - Antoine (Sad)", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"antoineupclose22k", "French - Antoine (UpClose)", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"bruno22k", "French - Bruno", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"claire22k", "French - Claire", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"julie22k", "French - Julie", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"margaux22k", "French - Margaux", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"margauxhappy22k", "French - Margaux (Happy)", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"margauxsad22k", "French - Margaux (Sad)", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"andreas22k", "German - Andreas", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"jonas22k", "German - Jonas (Child) - Premium", TTS_GENDER_FEMALE, TTS_AGE_CHILD,
	"julia22k", "German - Julia", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"klaus22k", "German - Klaus", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"lea22k", "German - Lea (Child) - Premium", TTS_GENDER_FEMALE, TTS_AGE_CHILD,
	"sarah22k", "German - Sarah", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"dimitris22k", "Greek - Dimitris", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"dimitrishappy22k", "Greek - Dimitris (Happy)", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"dimitrissad22k", "Greek - Dimitris (Sad)", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"chiara22k", "Italian - Chiara", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"fabiana22k", "Italian - Fabiana", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"vittorio22k", "Italian - Vittorio", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"sakura22k", "Japanese - Sakura", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"minji22k", "Korean - Minji", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"bente22k", "Norwegian - Bente", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"kari22k", "Norwegian - Kari", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"olav22k", "Norwegian - Olav", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"ania22k", "Polish - Ania", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"monika22k", "Polish - Monika", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"marcia22k", "Portuguese (Brazil) - Marcia", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"celia22k", "Portuguese - Celia", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"alyona22k", "Russian - Alyona", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"rodrigo22k", "Spanish (US) - Rodrigo", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"rosa22k", "Spanish (US) - Rosa", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"antonio22k", "Spanish - Antonio", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"ines22k", "Spanish - Ines", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"maria22k", "Spanish - Maria", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"samuel22k", "Swedish (Finland) - Samuel", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"kal22k", "Swedish (Gothenburg) - Kal", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"mia22k", "Swedish (Scania) - Mia", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"elin22k", "Swedish - Elin", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"emil22k", "Swedish - Emil", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"emma22k", "Swedish - Emma", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"erik22k", "Swedish - Erik", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
	"ipek22k", "Turkish - Ipek", TTS_GENDER_FEMALE, TTS_AGE_ADULT,
#endif //__OTHER_LANGUAGES_SUPPORT__
	"", "", TTS_GENDER_MAX, TTS_AGE_MAX // Marks the end of the list...
};

int TTS_VOICES_MAX = -1;

int GetTTSVoicesMax()
{
	int max = 0;

	if (TTS_VOICES_MAX != -1) return TTS_VOICES_MAX; // already set up...

	// We need to count them...
	while (ttsVoiceData[max].age != TTS_AGE_MAX)
	{
		max++;
	}

	TTS_VOICES_MAX = max;
	return max;
}

//
// Utility functions...
//
clientInfo_t *CG_GetClientInfoForEnt(centity_t *ent)
{
	clientInfo_t	*ci = NULL;

	if (ent->currentState.number < MAX_CLIENTS)
	{
		ci = &cgs.clientinfo[ent->currentState.number];
	}
	else
	{
		ci = ent->npcClient;
	}
	
	return ci;
}

//
// Stuff to select the best possible TTS voice types...
//
qboolean CG_IsSith(centity_t *ent, team_t team)
{
	clientInfo_t	*ci = CG_GetClientInfoForEnt(ent);

	if (team == TEAM_RED && ent->currentState.primaryWeapon == WP_SABER) 
		return qtrue;

	if (!ci) return qfalse;

	if (ci->modelName && ci->modelName[0])
	{
		if (StringContainsWord(ci->modelName, "sith"))
			return qtrue;
	}

	return qfalse;
}

qboolean CG_IsPadawan(centity_t *ent)
{
	clientInfo_t	*ci = CG_GetClientInfoForEnt(ent);

	if (ent->currentState.eType == ET_NPC)
	{// NPC handling...
		if (ent->currentState.NPC_class == CLASS_PADAWAN)
			return qtrue;
	}

	if (!ci) return qfalse;

	if (ci->modelName && ci->modelName[0])
	{
		if (StringContainsWord(ci->modelName, "padawan"))
			return qtrue;
	}

	return qfalse;
}

qboolean CG_IsImperialOfficer(centity_t *ent)
{
	clientInfo_t	*ci = CG_GetClientInfoForEnt(ent);

	if (ent->currentState.eType == ET_NPC)
	{// NPC handling...
		switch (ent->currentState.NPC_class)
		{
			case CLASS_IMPERIAL:
				return qtrue;
				break;
			default:
				break;
		}
	}

	if (!ci) return qfalse;

	if (ci->modelName && ci->modelName[0])
	{
		if (StringContainsWord(ci->modelName, "officer"))
			return qtrue;
	}

	return qfalse;
}

qboolean CG_IsBountyHunter(centity_t *ent)
{
	clientInfo_t	*ci = CG_GetClientInfoForEnt(ent);

	if (ent->currentState.eType == ET_NPC)
	{// NPC handling...
		switch (ent->currentState.NPC_class)
		{
			case CLASS_BOBAFETT:
				return qtrue;
				break;
			default:
				break;
		}
	}

	if (!ci) return qfalse;

	if (ci->modelName && ci->modelName[0])
	{
		if (StringContainsWord(ci->modelName, "boba") || StringContainsWord(ci->modelName, "bounty"))
			return qtrue;
	}

	return qfalse;
}

char *CG_GetTextToSpeechVoiceForEntity(centity_t *ent)
{
	if (ent->selected_voice)
	{// Already have a voice... Use it...
		return ttsVoiceData[ent->selected_voice].voicename;
	}
	else if (!ent->selected_voice)
	{// Find a new voice for this character...
		clientInfo_t	*ci = CG_GetClientInfoForEnt(ent);
		int				SELECTED_GENDER = TTS_GENDER_NONE;
		int				SELECTED_AGE = TTS_AGE_NONE;
		int				BEST_VOICES_NUM = 0;
		int				BEST_VOICES[128];
		int				i;

		// Select best gender for this entity...
		if (CG_IsImperialOfficer(ent))
		{// Special case... Use brittish voice for all imperial officers (like the movies - lol)... :)
			SELECTED_GENDER = TTS_GENDER_IMPERIAL_OFFICER; // All imperial officers are male. We don't have a female model, and i've never seen one anyway...
		}
		else if (CG_IsBountyHunter(ent))
		{// Use australian accent - closest thing to the new zealand accent from the movies...
			if (ci->gender == GENDER_FEMALE)
			{
				SELECTED_GENDER = TTS_GENDER_BOUNTY_HUNTER_FEMALE;
			}
			else
			{
				SELECTED_GENDER = TTS_GENDER_BOUNTY_HUNTER_MALE;
			}
		}
		else if (!ci)
		{// Will assume male...
			SELECTED_GENDER = TTS_GENDER_MALE;
		}
		else if (ci->gender == GENDER_MALE) 
		{
			if (CG_IsSith(ent, ci->team))
			{
				SELECTED_GENDER = TTS_GENDER_EVIL_MALE;
			}
			else
			{
				SELECTED_GENDER = TTS_GENDER_MALE;
			}
		}
		else if (ci->gender == GENDER_FEMALE) 
		{
			if (CG_IsSith(ent, ci->team))
			{
				SELECTED_GENDER = TTS_GENDER_EVIL_MALE;
			}
			else
			{
				SELECTED_GENDER = TTS_GENDER_FEMALE;
			}
		}
		else if (ci->gender == GENDER_NEUTER) 
		{
			SELECTED_GENDER = TTS_GENDER_DROID; // assume droid...
		}

		// Select the best age group for this entity... -- TODO: Old people npc class check??? Non-human (monster) check???
		if (CG_IsPadawan(ent))
		{
			SELECTED_AGE = TTS_AGE_CHILD;
		}
		else
		{
			SELECTED_AGE = TTS_AGE_ADULT;
		}

		// Now that we have found a gender and age to use, go through the full voices list and make a short list to select a voice from...
		for (i = 0; i < GetTTSVoicesMax(); i++)
		{// Create BEST voices list...
			if (ttsVoiceData[i].gender == SELECTED_GENDER && ttsVoiceData[i].age == SELECTED_AGE)
			{// Perfect... Add to the BEST list...
				BEST_VOICES[BEST_VOICES_NUM] = i;
				BEST_VOICES_NUM++;
			}
		}

		if (BEST_VOICES_NUM > 0)
		{// Found some, select one at random...
			ent->selected_voice = BEST_VOICES[irand(0, BEST_VOICES_NUM-1)];
		}
		else
		{
			ent->selected_voice = 0;
		}

		/*
		if (ci && ent->selected_voice)
		{
			trap->Print("%s selected TTS voice %s.\n", ci->name, ttsVoiceData[ent->selected_voice].voicename);
		}
		else if (ent->selected_voice)
		{
			trap->Print("(unknown) selected TTS voice %s.\n", ttsVoiceData[ent->selected_voice].voicename);
		}
		else if (ci && !ent->selected_voice)
		{
			trap->Print("%s could not find a TTS voice for age %i gender %i.. Using default.\n", ci->name, SELECTED_AGE, SELECTED_GENDER);
			ent->selected_voice = 1;
		}
		else if (!ent->selected_voice)
		{
			trap->Print("(unknown) could not find a TTS voice for age %i gender %i. Using default.\n", ci->name, SELECTED_AGE, SELECTED_GENDER);
			ent->selected_voice = 1;
		}
		*/
	}

	// Failed to find a matching voice... No TTS support!
	if (!ent->selected_voice) return NULL;
	
	// All good... Return the voice...
	return ttsVoiceData[ent->selected_voice].voicename;
}

//
// TTS Generic functions...
//
void TextToSpeech( const char *text, const char *voice )
{// UQ1: Now uses a trap call to do all the good stuff in engine (client) code...
	trap->S_TextToSpeech(text, voice);
}

void CG_SaySillyTextTest ( void )
{
#ifdef _WIN32
	int choice = irand(0,10);

	switch (choice)
	{
	case 1:
		TextToSpeech("What the are you doing???", CG_GetTextToSpeechVoiceForEntity(&cg_entities[cg.clientNum]));
		break;
	case 2:
		TextToSpeech("Stop that!", CG_GetTextToSpeechVoiceForEntity(&cg_entities[cg.clientNum]));
		break;
	case 3:
		TextToSpeech("Hay, stop it!", CG_GetTextToSpeechVoiceForEntity(&cg_entities[cg.clientNum]));
		break;
	case 4:
		TextToSpeech("Get away from me!", CG_GetTextToSpeechVoiceForEntity(&cg_entities[cg.clientNum]));
		break;
	case 5:
		TextToSpeech("How much wood wood a wood chuck chuck if a wood chuck could chuck wood?", CG_GetTextToSpeechVoiceForEntity(&cg_entities[cg.clientNum]));
		break;
	case 6:
		TextToSpeech("What are you doing?", CG_GetTextToSpeechVoiceForEntity(&cg_entities[cg.clientNum]));
		break;
	case 7:
		TextToSpeech("Don't talk to me.", CG_GetTextToSpeechVoiceForEntity(&cg_entities[cg.clientNum]));
		break;
	case 8:
		TextToSpeech("Go away!", CG_GetTextToSpeechVoiceForEntity(&cg_entities[cg.clientNum]));
		break;
	case 9:
		TextToSpeech("Ouch! That hurt!", CG_GetTextToSpeechVoiceForEntity(&cg_entities[cg.clientNum]));
		break;
	default:
		TextToSpeech("Oh meye!", CG_GetTextToSpeechVoiceForEntity(&cg_entities[cg.clientNum]));
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
		return;
	}

	trap->Cmd_Argv( 1, str, sizeof(str) );

	TextToSpeech(str, CG_GetTextToSpeechVoiceForEntity(&cg_entities[cg.clientNum]));
}
