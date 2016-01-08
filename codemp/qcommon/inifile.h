#include <stdio.h>
#include <stdlib.h>
#include <shlobj.h>
#include <string>

using namespace std;

const char *IniRead(char *aFilespec, char *aSection, char *aKey, char *aDefault);
bool IniWrite(char *aFilespec, char *aSection, char *aKey, char *aValue);