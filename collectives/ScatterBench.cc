#include <mpi.h>
#include "ScatterBench.h"

#ifdef USE_SCOREP
#include <scorep/SCOREP_User.h>
#endif




//======================================================================

#ifdef USE_SCOREP
SCOREP_USER_METRIC_LOCAL (bench_ScatterBench_metric);
#endif


CMSB::ScatterBench::ScatterBench (unsigned int messageSize) {

    _msgSize = messageSize;
}

CMSB::ScatterBench::~ScatterBench () {
}

void CMSB::ScatterBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {

	CMSB::CollectivesBench::init (worldComm, benchInfo);
#ifdef USE_SCOREP
    SCOREP_USER_METRIC_INIT (bench_ScatterBench_metric, "Scatter timing", "usec",
                             SCOREP_USER_METRIC_TYPE_DOUBLE, SCOREP_USER_METRIC_CONTEXT_GLOBAL);
#endif
}

const char* CMSB::ScatterBench::getMicroBenchName () const {

    return "MPI_Scatter";
}

void CMSB::ScatterBench::writeResultToProfile () const {

#ifdef USE_SCOREP
	if (_myRank == 0) {		// Only one rank should update SCORE-P's metrics
		SCOREP_USER_METRIC_DOUBLE (bench_ScatterBench_metric, getMicroBenchResult ());
	}
#endif
}

//======================================================================
