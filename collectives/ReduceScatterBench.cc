#include <mpi.h>
#include "ReduceScatterBench.h"

#ifdef USE_SCOREP
#include <scorep/SCOREP_User.h>
#endif




//======================================================================

#ifdef USE_SCOREP
SCOREP_USER_METRIC_LOCAL (bench_ReduceScatterBench_metric);
#endif


CMSB::ReduceScatterBench::ReduceScatterBench (unsigned int messageSize) {

    _msgSize = messageSize;
}

CMSB::ReduceScatterBench::~ReduceScatterBench () {
}

void CMSB::ReduceScatterBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {

	CMSB::CollectivesBench::init (worldComm, benchInfo);
#ifdef USE_SCOREP
    SCOREP_USER_METRIC_INIT (bench_ReduceScatterBench_metric, "ReduceScatter timing", "usec",
                             SCOREP_USER_METRIC_TYPE_DOUBLE, SCOREP_USER_METRIC_CONTEXT_GLOBAL);
#endif
}

const char* CMSB::ReduceScatterBench::getMicroBenchName () const {

    return "MPI_ReduceScatter";
}

void CMSB::ReduceScatterBench::writeResultToProfile () const {

#ifdef USE_SCOREP
	if (_myRank == 0) {		// Only one rank should update SCORE-P's metrics
		SCOREP_USER_METRIC_DOUBLE (bench_ReduceScatterBench_metric, getMicroBenchResult ());
	}
#endif
}

//======================================================================
