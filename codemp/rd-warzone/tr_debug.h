#ifndef TR_DEBUG
#define TR_DEBUG

#include <map>
#include <list>

typedef struct perfdata_s {
	int starttime;
	int stoptime;
} perfdata_t;

// this allows for nested performance tracking
extern std::map<std::string, perfdata_t> performancelog;
extern std::list<std::string> perfNameStack;

#endif