#include <sys/types.h>
#include <sys/stat.h>

#if defined(WIN32) || defined(WIN64)
#include <direct.h>
#include <windows.h>
#endif

#if defined (__linux__) || defined (__APPLE__)
#include <unistd.h>
#endif

#ifdef NeXT
#include <libc.h>
#endif

#include "qbsp.h"

#define bool boolean

typedef enum { qfalse, qtrue }	qboolean;

/*#define	MAX_VA_STRING	32000
#define MAX_VA_BUFFERS 4

char *va( const char *format, ... )
{
	va_list		argptr;
	static char	string[MAX_VA_BUFFERS][MAX_VA_STRING];	// in case va is called by nested functions
	static int	index = 0;
	char		*buf;

	va_start( argptr, format );
	buf = (char *)&string[index++ & 3];
	vsnprintf( buf, sizeof(*string), format, argptr );
	va_end( argptr );

	return buf;
}*/

#if defined(WIN32) || defined(WIN64)
bool textcolorprotect=true;
/*doesn't let textcolor be the same as backgroung color if true*/

inline void setcolor(int textcolor,int backcolor);
int textcolor();/*returns current text color*/
int backcolor();/*returns current background color*/

#define std_con_out GetStdHandle(STD_OUTPUT_HANDLE)

//-----------------------------------------------------------------------------

int textcolor()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(std_con_out,&csbi);
	int a=csbi.wAttributes;
	return a%16;
}

int backcolor()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(std_con_out,&csbi);
	int a=csbi.wAttributes;
	return (a/16)%16;
}

inline void setcolor(int textcol,int backcol)
{
	if(textcolorprotect)
	{if((textcol%16)==(backcol%16))textcol++;}
	textcol%=16;backcol%=16;
	unsigned short wAttributes= ((unsigned)backcol<<4)|(unsigned)textcol;
	SetConsoleTextAttribute(std_con_out, wAttributes);
}
#else //!defined(WIN32) || defined(WIN64)
inline void setcolor(concol_t textcol,concol_t backcol)
{

}

inline void setcolor(int textcol,int backcol)
{

}
#endif //defined(WIN32) || defined(WIN64)

void Sys_PrintHeading( char *heading )
{
	setcolor(white, black);
	printf(heading);
	setcolor(gray, black);
}

void Sys_PrintHeadingVerbose( char *heading )
{
	Sys_PrintHeading(heading );
	/*
	setcolor(white, black);
	printf(heading);
	setcolor(gray, black);
	*/
}


//time_t	CONSOLE_UPDATE_TIME = 0;
int		CONSOLE_WIDTH_CURRENT = 0;

int GetConsoleWidth( void )
{
	int columns;//, rows;

	if (CONSOLE_WIDTH_CURRENT == 0 /*|| time(NULL) >= CONSOLE_UPDATE_TIME*/)
	{// Only update once a second to stop is spazzing out with constant requests...
#if defined(WIN32) || defined(WIN64)
		CONSOLE_SCREEN_BUFFER_INFO csbi;

		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		//rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else // LINUX
		struct winsize max;
		ioctl(0, TIOCGWINSZ , &max);
		columns = max.ws_col;
		//rows = max.ws_row;
#endif

		CONSOLE_WIDTH_CURRENT = columns;
		//CONSOLE_UPDATE_TIME = time(NULL) + 1000;
	}

	return CONSOLE_WIDTH_CURRENT;
}

int previousPerc = -1;
char previousPercLabel[18] = { 0 };

void DoProgress( char inlabel[], int instep, int total, qboolean verbose )
{
	int step = instep + 1;
	char label[18] = { 0 };
	int width, pos;

	//progress width
    int pwidth = 72;

    int percent = ( step * 100 ) / total;

	if (percent > 100) percent = 100; // Just in case...

	if (percent == previousPerc)
	{// Don't waste CPU time...
		return;
	}

	strncpy(label, inlabel, 17);
	label[17] = '\0';

	pwidth = GetConsoleWidth() - 8;

    //minus label len
    width = pwidth - 18;//strlen( label );
    pos = ( step * width ) / total ;

	setcolor(white, black);

	if (verbose)
	{
		printf( "%-18s", label );
	}
	else
	{
		printf( "%-18s", label );
	}

	setcolor(dark_green, black);

	if (verbose)
	{
		printf( "[" );
	}
	else
	{
		printf( "[" );
	}


    //fill progress bar with =
    for ( int i = 0; i < pos; i++ )  printf( "%c", '=' );

    //fill progress bar with spaces
	if (verbose)
	{
		printf( "% *c", width - pos + 1, ']' );
		setcolor(yellow, black);
		printf( " %-3d\r", percent );
	}
	else
	{
		printf( "% *c", width - pos + 1, ']' );
		setcolor(yellow, black);
		printf( " %-3d\r", percent );
	}

	setcolor(gray, black);

	if (percent >= 100.0 && strcmp(previousPercLabel, label))
	{
		strcpy(previousPercLabel, label);

		if (verbose)
			printf( "\n" );
		else
			printf( "\n" );
	}

	previousPerc = percent;
}

void printLabelledProgress (char *label, double current, double max)
{
	if (max == 1)
		DoProgress( label, current*100, max*100, qfalse );
	else
		DoProgress( label, current, max, qfalse );
}

void printLabelledProgressVerbose (char *label, double current, double max)
{
	if (max == 1)
		DoProgress( label, current*100, max*100, qfalse );
	else
		DoProgress( label, current, max, qtrue );
}

void printProgress (double perc)
{// percentage is in 0 -> 9 range...
	DoProgress( "", perc, 9, qfalse );
}

void printProgressVerbose (double perc)
{// percentage is in 0 -> 9 range...
	DoProgress( "", perc, 9, qtrue );
}

void printDetailedProgress (double percentage)
{// percentage is in 0.0 -> 1.0 range...
    DoProgress( "", percentage, 1, qfalse );
}

void printDetailedProgressVerbose (double percentage)
{
	DoProgress( "", percentage, 1, qtrue );
}
