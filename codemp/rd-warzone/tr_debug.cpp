#include "tr_local.h"

#include <iostream>
using namespace std;
#include <cstdlib>
#include <sys/timeb.h>

int getMilliCount() {
	timeb tb;
	ftime(&tb);
	int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
	return nCount;
}

int getMilliSpan(int nTimeStart) {
	int nSpan = getMilliCount() - nTimeStart;
	if (nSpan < 0)
		nSpan += 0x100000 * 1000;
	return nSpan;
}

#ifdef __PERFORMANCE_DEBUG__
int 	DEBUG_PERFORMANCE_TIME = 0;
char	DEBUG_PERFORMANCE_NAME[128] = { 0 };
#endif //__PERFORMANCE_DEBUG__

void DEBUG_StartTimer(char *name, qboolean usePerfCvar)
{
#ifdef __PERFORMANCE_DEBUG__
	if (!usePerfCvar || r_perf->integer)
	{
		if (usePerfCvar)
			qglFinish();

		memset(DEBUG_PERFORMANCE_NAME, 0, sizeof(char) * 128);
		strcpy(DEBUG_PERFORMANCE_NAME, name);
		DEBUG_PERFORMANCE_TIME = getMilliCount();
	}
#endif //__PERFORMANCE_DEBUG__
}

void DEBUG_EndTimer(qboolean usePerfCvar)
{
#ifdef __PERFORMANCE_DEBUG__
	if (!usePerfCvar || r_perf->integer)
	{
		if (usePerfCvar)
			qglFinish();

		DEBUG_PERFORMANCE_TIME = getMilliSpan(DEBUG_PERFORMANCE_TIME);

		if (DEBUG_PERFORMANCE_NAME[0] != '\0' && strlen(DEBUG_PERFORMANCE_NAME) > 0)
		{
			ri->Printf(PRINT_WARNING, "%s took %i ms to complete.\n", DEBUG_PERFORMANCE_NAME, DEBUG_PERFORMANCE_TIME);
		}
		else
		{
			ri->Printf(PRINT_WARNING, "%s took %i ms to complete.\n", "unknown", DEBUG_PERFORMANCE_TIME);
		}
	}
#endif //__PERFORMANCE_DEBUG__
}
