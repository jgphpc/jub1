#include <mpi.h>
#include "GathervBench.h"

#ifdef USE_SCOREP
#include <scorep/SCOREP_User.h>
#endif




//======================================================================

#ifdef USE_SCOREP
SCOREP_USER_METRIC_LOCAL (bench_GathervBench_metric);
#endif


CMSB::GathervBench::GathervBench (unsigned int messageSize) {

    _msgSize = messageSize;
}

CMSB::GathervBench::~GathervBench () {
}

void CMSB::GathervBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {

	CMSB::CollectivesBench::init (worldComm, benchInfo);
#ifdef USE_SCOREP
    SCOREP_USER_METRIC_INIT (bench_GathervBench_metric, "Gatherv timing", "usec",
                             SCOREP_USER_METRIC_TYPE_DOUBLE, SCOREP_USER_METRIC_CONTEXT_GLOBAL);
#endif
}

const char* CMSB::GathervBench::getMicroBenchName () const {

    return "MPI_Gatherv";
}

void CMSB::GathervBench::writeResultToProfile () const {

#ifdef USE_SCOREP
	if (_myRank == 0) {		// Only one rank should update SCORE-P's metrics
		SCOREP_USER_METRIC_DOUBLE (bench_GathervBench_metric, getMicroBenchResult ());
	}
#endif
}

//======================================================================
