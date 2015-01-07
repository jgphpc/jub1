#include <mpi.h>
#include "AllreduceBench.h"

#ifdef USE_SCOREP
#include <scorep/SCOREP_User.h>
#endif




//======================================================================

#ifdef USE_SCOREP
SCOREP_USER_METRIC_LOCAL (bench_AllreduceBench_metric);
#endif


CMSB::AllreduceBench::AllreduceBench (unsigned int messageSize) {

    _msgSize = messageSize;
}

CMSB::AllreduceBench::~AllreduceBench () {
}

void CMSB::AllreduceBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {

	CMSB::CollectivesBench::init (worldComm, benchInfo);
#ifdef USE_SCOREP
    SCOREP_USER_METRIC_INIT (bench_AllreduceBench_metric, "Allreduce timing", "usec",
                             SCOREP_USER_METRIC_TYPE_DOUBLE, SCOREP_USER_METRIC_CONTEXT_GLOBAL);
#endif
}

const char* CMSB::AllreduceBench::getMicroBenchName () const {

    return "MPI_Allreduce";
}

void CMSB::AllreduceBench::writeResultToProfile () const {

#ifdef USE_SCOREP
	if (_myRank == 0) {		// Only one rank should update SCORE-P's metrics
		SCOREP_USER_METRIC_DOUBLE (bench_AllreduceBench_metric, getMicroBenchResult ());
	}
#endif
}

//======================================================================
