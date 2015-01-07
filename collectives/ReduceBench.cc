#include <mpi.h>
#include "ReduceBench.h"

#ifdef USE_SCOREP
#include <scorep/SCOREP_User.h>
#endif




//======================================================================

#ifdef USE_SCOREP
SCOREP_USER_METRIC_LOCAL (bench_ReduceBench_metric);
#endif


CMSB::ReduceBench::ReduceBench (unsigned int messageSize) {

    _msgSize = messageSize;
}

CMSB::ReduceBench::~ReduceBench () {
}

void CMSB::ReduceBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {

	CMSB::CollectivesBench::init (worldComm, benchInfo);
#ifdef USE_SCOREP
    SCOREP_USER_METRIC_INIT (bench_ReduceBench_metric, "Reduce timing", "usec",
                             SCOREP_USER_METRIC_TYPE_DOUBLE, SCOREP_USER_METRIC_CONTEXT_GLOBAL);
#endif
}

const char* CMSB::ReduceBench::getMicroBenchName () const {

    return "MPI_Reduce";
}

void CMSB::ReduceBench::writeResultToProfile () const {

#ifdef USE_SCOREP
	if (_myRank == 0) {		// Only one rank should update SCORE-P's metrics
		SCOREP_USER_METRIC_DOUBLE (bench_ReduceBench_metric, getMicroBenchResult ());
	}
#endif
}

//======================================================================
