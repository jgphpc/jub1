#include <mpi.h>
#include "GatherBench.h"

#ifdef USE_SCOREP
#include <scorep/SCOREP_User.h>
#endif




//======================================================================

#ifdef USE_SCOREP
SCOREP_USER_METRIC_LOCAL (bench_GatherBench_metric);
#endif


CMSB::GatherBench::GatherBench (unsigned int messageSize) {

    _msgSize = messageSize;
}

CMSB::GatherBench::~GatherBench () {
}

void CMSB::GatherBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {

	CMSB::CollectivesBench::init (worldComm, benchInfo);
#ifdef USE_SCOREP
    SCOREP_USER_METRIC_INIT (bench_GatherBench_metric, "Gather timing", "usec",
                             SCOREP_USER_METRIC_TYPE_DOUBLE, SCOREP_USER_METRIC_CONTEXT_GLOBAL);
#endif
}

const char* CMSB::GatherBench::getMicroBenchName () const {

    return "MPI_Gather";
}

void CMSB::GatherBench::writeResultToProfile () const {

#ifdef USE_SCOREP
	if (_myRank == 0) {		// Only one rank should update SCORE-P's metrics
		SCOREP_USER_METRIC_DOUBLE (bench_GatherBench_metric, getMicroBenchResult ());
	}
#endif
}

//======================================================================
