#include <mpi.h>
#include "BcastBench.h"

#ifdef USE_SCOREP
#include <scorep/SCOREP_User.h>
#endif




//======================================================================

#ifdef USE_SCOREP
SCOREP_USER_METRIC_LOCAL (bench_BcastBench_metric);
#endif


CMSB::BcastBench::BcastBench (unsigned int messageSize) {

    _msgSize = messageSize;
}

CMSB::BcastBench::~BcastBench () {
}

void CMSB::BcastBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {

	CMSB::CollectivesBench::init (worldComm, benchInfo);
#ifdef USE_SCOREP
    SCOREP_USER_METRIC_INIT (bench_BcastBench_metric, "Bcast timing", "usec",
                             SCOREP_USER_METRIC_TYPE_DOUBLE, SCOREP_USER_METRIC_CONTEXT_GLOBAL);
#endif
}

const char* CMSB::BcastBench::getMicroBenchName () const {

    return "MPI_Bcast";
}

void CMSB::BcastBench::writeResultToProfile () const {

#ifdef USE_SCOREP
	if (_myRank == 0) {		// Only one rank should update SCORE-P's metrics
		SCOREP_USER_METRIC_DOUBLE (bench_BcastBench_metric, getMicroBenchResult ());
	}
#endif
}

//======================================================================
