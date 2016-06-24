#if defined(rd_warzone_x86_EXPORTS) || defined(DEDICATED) || defined(jampgamex86_EXPORTS) || defined(cgamex86_EXPORTS) || defined(uix86_EXPORTS)

//
// Mixing C and C++ in one codebase is soooooo annoying!!!!
//
#if defined(rd_warzone_x86_EXPORTS)
#include "inifile.h"   /* function prototypes in here */
#else //!defined(rd_warzone_x86_EXPORTS)
extern "C"
{
	#include "inifile.h"   /* function prototypes in here */
}
#endif //defined(rd_warzone_x86_EXPORTS)

#ifdef _WIN32
#define unlink _unlink
#endif

#if defined(rd_warzone_x86_EXPORTS)
#include "tr_local.h"
#define FS_FOpenFileByMode ri->FS_FOpenFileByMode
#define FS_Read ri->FS_Read
#define FS_Write ri->FS_Write
#define FS_FCloseFile ri->FS_FCloseFile
#elif defined(jampgamex86_EXPORTS)
extern "C" {
#include "game/g_local.h"
#define FS_FOpenFileByMode trap->FS_Open
#define FS_Read trap->FS_Read
#define FS_Write trap->FS_Write
#define FS_FCloseFile trap->FS_Close
}
#elif defined(cgamex86_EXPORTS)
extern "C" {
#include "cgame/cg_local.h"
#define FS_FOpenFileByMode trap->FS_Open
#define FS_Read trap->FS_Read
#define FS_Write trap->FS_Write
#define FS_FCloseFile trap->FS_Close
}
#elif defined(uix86_EXPORTS)
extern "C" {
#include "ui/ui_local.h"
#define FS_FOpenFileByMode trap->FS_Open
#define FS_Read trap->FS_Read
#define FS_Write trap->FS_Write
#define FS_FCloseFile trap->FS_Close
}
#else
#include "q_shared.h"
extern int FS_FOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode );
extern int FS_Read( void *buffer, int len, fileHandle_t f );
extern int FS_Write( const void *buffer, int len, fileHandle_t h );
extern void FS_FCloseFile( fileHandle_t f );
#endif


char FS_GetC(fileHandle_t fp)
{
	char c = '\0';
	FS_Read(&c, sizeof(char), fp);
	return c;
}

/*****************************************************************
* Function:     read_line()
* Arguments:    <fileHandle_t> fp - a pointer to the file to be read from
*               <char *> bp - a pointer to the copy buffer
* Returns:      TRUE if successful FALSE otherwise
******************************************************************/
int read_line(fileHandle_t fp, char *bp)
{   
	char c = '\0';
    int i = 0;

    /* Read one line from the source fileHandle_t */
    while( (c = FS_GetC(fp)) != '\n' )
    {
		if( c == EOF || c == '\0' )         /* return FALSE on unexpected EOF */
            return(0);

        bp[i] = c;
		i++;
    }

    bp[i] = '\0';

/*#ifdef _GAME
	trap->Print("%s\n", bp);
#endif*/

    return(1);
}

/**************************************************************************
* Function:     get_private_profile_string()
* Arguments:    <char *> section - the name of the section to search for
*               <char *> entry - the name of the entry to find the value of
*               <char *> def - default string in the event of a failed read
*               <char *> buffer - a pointer to the buffer to copy into
*               <int> buffer_len - the max number of characters to copy
*               <char *> file_name - the name of the .ini file to read from
* Returns:      the number of characters copied into the supplied buffer
***************************************************************************/
int get_private_profile_string(char *section, char *entry, char *def, char *buffer, int buffer_len, char *file_name)
{
	fileHandle_t fp = NULL;
    char buff[MAX_LINE_LENGTH] = { 0 };
    char *ep;
    char t_section[MAX_LINE_LENGTH] = { 0 };
    int len = strlen(entry);

	int fpLen = FS_FOpenFileByMode(file_name, &fp, FS_READ);

    if( !fp || !fpLen ) return(0);

    sprintf(t_section,"[%s]",section);    /* Format the section name */

    /*  Move through file 1 line at a time until a section is matched or EOF */
    do
    {   
		if( !read_line(fp,buff) )
        {   
			FS_FCloseFile(fp);
            strncpy(buffer,def,buffer_len);
            return(strlen(buffer));
        }
    }
    while( strncmp(buff,t_section,strlen(t_section)) );

    /* Now that the section has been found, find the entry.
     * Stop searching upon leaving the section's area. */
    do
    {   
		if( !read_line(fp,buff) || buff[0] == '\0' )
        {   
			FS_FCloseFile(fp);
            strncpy(buffer,def,buffer_len);
            return(strlen(buffer));
        }
    }  while( strncmp(buff,entry,len) );

    ep = strrchr(buff,'=');    /* Parse out the equal sign */
    ep++;

    /* Copy up to buffer_len chars to buffer */
    strncpy(buffer,ep,strlen(ep)-1); // -1 to remove the trailing \n

    FS_FCloseFile(fp);               /* Clean up and return the amount copied */

    return(strlen(buffer));
}

/*************************************************************************
 * Function:    write_private_profile_string()
 * Arguments:   <char *> section - the name of the section to search for
 *              <char *> entry - the name of the entry to find the value of
 *              <char *> buffer - pointer to the buffer that holds the string
 *              <char *> file_name - the name of the .ini file to read from
 * Returns:     TRUE if successful, otherwise FALSE
 *************************************************************************/
int write_private_profile_string(char *section, char *entry, char *buffer, char *file_name)

{   
	fileHandle_t rfp = NULL, wfp = NULL;
    char tmp_name[15];
    char buff[MAX_LINE_LENGTH] = { 0 };
    char t_section[MAX_LINE_LENGTH] = { 0 };
    int len = strlen(entry);
	int rfpLen, wfpLen;

    tmpnam(tmp_name); /* Get a temporary file name to copy to */
    sprintf(t_section,"[%s]",section);/* Format the section name */
    
	rfpLen = FS_FOpenFileByMode(file_name, &rfp, FS_READ);

	if( !rfp || !rfpLen )  /* If the .ini file doesn't exist */
    {   
		char sectionText[81] = { 0 };
		char entryText[1024] = { 0 };

		wfpLen = FS_FOpenFileByMode(file_name, &wfp, FS_WRITE); /*  then make one */

		if( !wfp || !wfpLen ) 
        {   
			return(0);   
		}
		
		sprintf(sectionText,"%s\n",t_section);
		FS_Write(sectionText, strlen(sectionText), wfp);

		sprintf(entryText,"%s=%s\n",entry,buffer);
		FS_Write(entryText, strlen(entryText), wfp);

        FS_FCloseFile(wfp);
        return(1);
    }

	wfpLen = FS_FOpenFileByMode(tmp_name, &wfp, FS_WRITE);

    if( !wfp || !wfpLen )
    {   
		FS_FCloseFile(rfp);
        return(0);
    }

    /* Move through the file one line at a time until a section is
     * matched or until EOF. Copy to temp file as it is read. */

    do
    {   
		char buffText[81] = { 0 };

		if( !read_line(rfp, buff) )
        {   
			char sectionText[81] = { 0 };
			char entryText[1024] = { 0 };

			/* Failed to find section, so add one to the end */
			sprintf(sectionText,"%s\n",t_section);
			FS_Write(sectionText, strlen(sectionText), wfp);

			sprintf(entryText,"%s=%s\n",entry,buffer);
			FS_Write(entryText, strlen(entryText), wfp);

            /* Clean up and rename */
            FS_FCloseFile(rfp);
            FS_FCloseFile(wfp);
            unlink(file_name);
            rename(tmp_name,file_name);
            return(1);
        }

        sprintf(buffText,"%s\n",buff);
		FS_Write(buffText, strlen(buffText), wfp);

    } while( strncmp(buff,t_section,strlen(t_section)) );

    /* Now that the section has been found, find the entry. Stop searching
     * upon leaving the section's area. Copy the file as it is read
     * and create an entry if one is not found.  */
    while( 1 )
    {   
		char buffText[81] = { 0 };

		if( !read_line(rfp, buff) )
        {   
			char entryText[1024] = { 0 };

			/* EOF without an entry so make one */
            sprintf(entryText,"%s=%s\n",entry,buffer);
			FS_Write(entryText, strlen(entryText), wfp);

            /* Clean up and rename */
            FS_FCloseFile(rfp);
            FS_FCloseFile(wfp);
            unlink(file_name);
            rename(tmp_name,file_name);
            return(1);
        }

        if( !strncmp(buff, entry, len) || buff[0] == '\0' )
            break;

		sprintf(buffText,"%s\n",buff);
		FS_Write(buffText, strlen(buffText), wfp);
    }

    if( buff[0] == '\0' )
    {   
		char entryText[1024] = { 0 };

		sprintf(entryText,"%s=%s\n",entry,buffer);
		FS_Write(entryText, strlen(entryText), wfp);

        do
        {
			char buffText[81] = { 0 };
			sprintf(buffText,"%s\n",buff);
			FS_Write(buffText, strlen(buffText), wfp);
        } while( read_line(rfp, buff) );
    }
    else
    {   
		char entryText[1024] = { 0 };

		sprintf(entryText,"%s=%s\n",entry,buffer);
		FS_Write(entryText, strlen(entryText), wfp);

        while( read_line(rfp, buff) )
        {
			char buffText[81] = { 0 };
			sprintf(buffText,"%s\n",buff);
			FS_Write(buffText, strlen(buffText), wfp);
        }
    }

    /* Clean up and rename */
    FS_FCloseFile(wfp);
    FS_FCloseFile(rfp);
    unlink(file_name);
    rename(tmp_name,file_name);
    return(1);
}


// ==============================================================================================================
// C++ Interface Layer Functions - For simple usage from C++ code...
// ==============================================================================================================

const char *IniReadCPP(char *aFilespec, char *aSection, char *aKey, char *aDefault)
{
	if (!aDefault || !*aDefault)
		aDefault = "";

	if (!aKey || !aKey[0])
		return "";

	char	szBuffer[65535] = { 0 };					// Max ini file size is 65535 under 95

	get_private_profile_string(aSection, aKey, aDefault, szBuffer, sizeof(szBuffer), aFilespec);

	return va(szBuffer);
}

void IniWriteCPP(char *aFilespec, char *aSection, char *aKey, char *aValue)
{
	write_private_profile_string(aSection, aKey, aValue, aFilespec);
}

// ==============================================================================================================
// C Interface Layer Functions - For simple access from Q3 code...
// ==============================================================================================================

/*
// Example usage
PLAYER_FACTION = IniRead("general.ini","PLAYER_SETTINGS","PLAYER_FACTION","imperial");
PLAYER_FACTION_AUTODETECT = atoi(IniRead("general.ini","PLAYER_SETTINGS","PLAYER_HEALTH","100"));
*/

#if defined(rd_warzone_x86_EXPORTS)
const char *IniRead(char *aFilespec, char *aSection, char *aKey, char *aDefault)
{
	const char *value = IniReadCPP(aFilespec, aSection, aKey, aDefault);
/*#ifdef _GAME
	trap->Print("[file] %s [section] %s [key] %s [value] %s\n", aFilespec, aSection, aKey, value);
#endif*/
	return value;
}

void IniWrite(char *aFilespec, char *aSection, char *aKey, char *aValue)
{
	IniWriteCPP(aFilespec, aSection, aKey, aValue);
}
#else //!defined(rd_warzone_x86_EXPORTS)
extern "C"
{
	const char *IniRead(char *aFilespec, char *aSection, char *aKey, char *aDefault)
	{
		const char *value = IniReadCPP(aFilespec, aSection, aKey, aDefault);
/*#ifdef _GAME
		trap->Print("[file] %s [section] %s [key] %s [value] %s\n", aFilespec, aSection, aKey, value);
#endif*/
		return value;
	}

	void IniWrite(char *aFilespec, char *aSection, char *aKey, char *aValue)
	{
		IniWriteCPP(aFilespec, aSection, aKey, aValue);
	}
}
#endif //!defined(rd_warzone_x86_EXPORTS)

#endif //defined(rd_warzone_x86_EXPORTS) || defined(DEDICATED) || defined(jampgamex86_EXPORTS) || defined(cgamex86_EXPORTS) || defined(uix86_EXPORTS)
