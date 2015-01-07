#include <mpi.h>
#include "AlltoallBench.h"

#ifdef USE_SCOREP
#include <scorep/SCOREP_User.h>
#endif




//======================================================================

#ifdef USE_SCOREP
SCOREP_USER_METRIC_LOCAL (bench_AlltoallBench_metric);
#endif


CMSB::AlltoallBench::AlltoallBench (unsigned int messageSize) {

    _msgSize = messageSize;
}

CMSB::AlltoallBench::~AlltoallBench () {
}

void CMSB::AlltoallBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {

	CMSB::CollectivesBench::init (worldComm, benchInfo);
#ifdef USE_SCOREP
    SCOREP_USER_METRIC_INIT (bench_AlltoallBench_metric, "Alltoall timing", "usec",
                             SCOREP_USER_METRIC_TYPE_DOUBLE, SCOREP_USER_METRIC_CONTEXT_GLOBAL);
#endif
}

const char* CMSB::AlltoallBench::getMicroBenchName () const {

    return "MPI_Alltoall";
}

void CMSB::AlltoallBench::writeResultToProfile () const {

#ifdef USE_SCOREP
	if (_myRank == 0) {		// Only one rank should update SCORE-P's metrics
		SCOREP_USER_METRIC_DOUBLE (bench_AlltoallBench_metric, getMicroBenchResult ());
	}
#endif
}

//======================================================================
