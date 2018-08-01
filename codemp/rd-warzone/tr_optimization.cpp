#include "tr_local.h"
#include "meshOptimization/triListOpt.h"


//#define __DEBUG_INDEXES_OPTIMIZATION__


extern int getMilliCount();
extern int getMilliSpan(int nTimeStart);

//
// Indexes optimization...
//
unsigned const VBUF_SZ = 32;

typedef struct
{
	unsigned ix;
	unsigned pos;
} vbuf_entry_t;

vbuf_entry_t vbuf[VBUF_SZ];

float calc_acmr(uint32_t numIndexes, uint32_t *indexes) 
{
	vbuf_entry_t vbuf[VBUF_SZ];
	unsigned num_cm(0); // cache misses

	for (unsigned i = 0; i < numIndexes; ++i) {
		bool found(0);
		unsigned best_entry(0), oldest_pos((unsigned)-1);

		for (unsigned n = 0; n < VBUF_SZ; ++n) {
			if (vbuf[n].ix == indexes[i]) { found = 1; break; }
			if (vbuf[n].pos < oldest_pos) { best_entry = n; oldest_pos = vbuf[n].pos; }
		}
		if (found) continue;
		vbuf[best_entry].ix = indexes[i];
		vbuf[best_entry].pos = i;
		++num_cm;
	}
	return float(num_cm) / float(numIndexes);
}

void R_VertexCacheOptimizeMeshIndexes(uint32_t numVerts, uint32_t numIndexes, uint32_t *indexes)
{
	if (numIndexes < 1.5*numVerts || numVerts < 2 * VBUF_SZ /*|| num_verts < 100000*/) return;
	float const orig_acmr(calc_acmr(numIndexes, indexes)), perfect_acmr(float(numVerts) / float(numIndexes));
	if (orig_acmr < 1.05*perfect_acmr) return;

#ifdef __DEBUG_INDEXES_OPTIMIZATION__
	int start_time = getMilliCount();
#endif //__DEBUG_INDEXES_OPTIMIZATION__

	vector<unsigned> out_indices(numIndexes);
	TriListOpt::OptimizeTriangleOrdering(numVerts, numIndexes, indexes, &out_indices.front());

	for (int i = 0; i < out_indices.size(); i++)
	{
		indexes[i] = out_indices[i];
	}

#ifdef __DEBUG_INDEXES_OPTIMIZATION__
	float optimized_acmr = calc_acmr(numIndexes, indexes);
	float optPercent = (1.0 - (optimized_acmr / orig_acmr)) * 100.0;
	int totalTime = getMilliSpan(start_time);
	ri->Printf(PRINT_WARNING, "R_VertexCacheOptimizeMeshIndexes: Original acmr: %f. Optimized acmr: %f. Perfect acmr: %f. Optimization Ratio: %f. Time: %i ms.\n", orig_acmr, optimized_acmr, perfect_acmr, optPercent, totalTime);
#endif //__DEBUG_INDEXES_OPTIMIZATION__
}
