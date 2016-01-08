#include <stdio.h>
#include <stdlib.h>
#include <shlobj.h>
#include <string>

using namespace std;

string IniReadCPP(char *aFilespec, char *aSection, char *aKey, char *aDefault)
{
	if (!aDefault || !*aDefault)
		aDefault = "";

	if (!aKey || !*aKey || aKey == "")
		return ""; // Bug...

	char	szFileTemp[_MAX_PATH+1];
	char	*szFilePart;
	char	szBuffer[65535] = "";					// Max ini file size is 65535 under 95

	// Get the fullpathname (ini functions need a full path):
	GetFullPathName(aFilespec, _MAX_PATH, szFileTemp, &szFilePart);
	GetPrivateProfileString(aSection, aKey, aDefault, szBuffer, sizeof(szBuffer), szFileTemp);

	return szBuffer;
}

bool IniWriteCPP(char *aFilespec, char *aSection, char *aKey, char *aValue)
{
	char	szFileTemp[_MAX_PATH+1];
	char	*szFilePart;

	// Get the fullpathname (ini functions need a full path) 
	GetFullPathName(aFilespec, _MAX_PATH, szFileTemp, &szFilePart);
	BOOL result = WritePrivateProfileString(aSection, aKey, aValue, szFileTemp);  // Returns zero on failure.
	WritePrivateProfileString(NULL, NULL, NULL, szFileTemp);	// Flush
	
	return true;
}

extern "C"
{
/*
	// Example usage

	PLAYER_FACTION = IniRead("general.ini","MISC_SETTINGS","PLAYER_FACTION","federation");
	PLAYER_FACTION_AUTODETECT = atoi(IniRead("general.ini","MISC_SETTINGS","PLAYER_FACTION_AUTODETECT","1"));
*/
  const char *IniRead(char *aFilespec, char *aSection, char *aKey, char *aDefault)
  {
	  return IniReadCPP(aFilespec, aSection, aKey, aDefault).c_str();
  }
  
  bool IniWrite(char *aFilespec, char *aSection, char *aKey, char *aValue)
  {
    return IniWriteCPP(aFilespec, aSection, aKey, aValue);
  }
}
