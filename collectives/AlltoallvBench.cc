#include <mpi.h>
#include "AlltoallvBench.h"

#ifdef USE_SCOREP
#include <scorep/SCOREP_User.h>
#endif




//======================================================================

#ifdef USE_SCOREP
SCOREP_USER_METRIC_LOCAL (bench_AlltoallvBench_metric);
#endif


CMSB::AlltoallvBench::AlltoallvBench (unsigned int messageSize) {

    _msgSize = messageSize;
}

CMSB::AlltoallvBench::~AlltoallvBench () {
}

void CMSB::AlltoallvBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {

	CMSB::CollectivesBench::init (worldComm, benchInfo);
#ifdef USE_SCOREP
    SCOREP_USER_METRIC_INIT (bench_AlltoallvBench_metric, "Alltoallv timing", "usec",
                             SCOREP_USER_METRIC_TYPE_DOUBLE, SCOREP_USER_METRIC_CONTEXT_GLOBAL);
#endif
}

const char* CMSB::AlltoallvBench::getMicroBenchName () const {

    return "MPI_Alltoallv";
}

void CMSB::AlltoallvBench::writeResultToProfile () const {

#ifdef USE_SCOREP
	if (_myRank == 0) {		// Only one rank should update SCORE-P's metrics
		SCOREP_USER_METRIC_DOUBLE (bench_AlltoallvBench_metric, getMicroBenchResult ());
	}
#endif
}

//======================================================================
