#include <mpi.h>
#include "ScattervBench.h"

#ifdef USE_SCOREP
#include <scorep/SCOREP_User.h>
#endif




//======================================================================

#ifdef USE_SCOREP
SCOREP_USER_METRIC_LOCAL (bench_ScattervBench_metric);
#endif


CMSB::ScattervBench::ScattervBench (unsigned int messageSize) {

    _msgSize = messageSize;
}

CMSB::ScattervBench::~ScattervBench () {
}

void CMSB::ScattervBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {

	CMSB::CollectivesBench::init (worldComm, benchInfo);
#ifdef USE_SCOREP
    SCOREP_USER_METRIC_INIT (bench_ScattervBench_metric, "Scatterv timing", "usec",
                             SCOREP_USER_METRIC_TYPE_DOUBLE, SCOREP_USER_METRIC_CONTEXT_GLOBAL);
#endif
}

const char* CMSB::ScattervBench::getMicroBenchName () const {

    return "MPI_Scatterv";
}

void CMSB::ScattervBench::writeResultToProfile () const {

#ifdef USE_SCOREP
	if (_myRank == 0) {		// Only one rank should update SCORE-P's metrics
		SCOREP_USER_METRIC_DOUBLE (bench_ScattervBench_metric, getMicroBenchResult ());
	}
#endif
}

//======================================================================
