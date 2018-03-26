#include "tr_local.h"

#include <iostream>
using namespace std;
#include <cstdlib>
#include <sys/timeb.h>

#include "tr_debug.h"

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

// this allows for nested performance tracking
std::map<std::string, perfdata_t> performancelog;
std::list<std::string> perfNameStack;

void DEBUG_StartTimer(char *name, qboolean usePerfCvar)
{
#ifdef __PERFORMANCE_DEBUG__
	if (!usePerfCvar || r_perf->integer)
	{
		if (usePerfCvar)
			qglFinish();

		// then log the start time
		performancelog[name].starttime = getMilliCount(); //timeGetTime();
		perfNameStack.push_back(name);

		memset(DEBUG_PERFORMANCE_NAME, 0, sizeof(char) * 128);
		strcpy(DEBUG_PERFORMANCE_NAME, name);
		DEBUG_PERFORMANCE_TIME = getMilliCount();

		/*
		if (DEBUG_PERFORMANCE_NAME[0] != '\0' && strlen(DEBUG_PERFORMANCE_NAME) > 0)
		{
			ri->Printf(PRINT_WARNING, "%s begins.\n", DEBUG_PERFORMANCE_NAME);
		}
		*/
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
			
		// ...and log the end time, so we can calculate a real delta
		auto lastPerfName = perfNameStack.back();
		performancelog[lastPerfName].stoptime = getMilliCount();
		perfNameStack.pop_back();

		DEBUG_PERFORMANCE_TIME = getMilliSpan(DEBUG_PERFORMANCE_TIME);

		// you can see the results in Perf dock now --  UQ1: Gonna keep the messages for now so I can track down startup errors easier...
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
