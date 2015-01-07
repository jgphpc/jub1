#include <mpi.h>
#include "AllgatherBench.h"

#ifdef USE_SCOREP
#include <scorep/SCOREP_User.h>
#endif




//======================================================================

#ifdef USE_SCOREP
SCOREP_USER_METRIC_LOCAL (bench_AllgatherBench_metric);
#endif


CMSB::AllgatherBench::AllgatherBench (unsigned int messageSize) {

    _msgSize = messageSize;
}

CMSB::AllgatherBench::~AllgatherBench () {
}

void CMSB::AllgatherBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {

	CMSB::CollectivesBench::init (worldComm, benchInfo);
#ifdef USE_SCOREP
    SCOREP_USER_METRIC_INIT (bench_AllgatherBench_metric, "Allgather timing", "usec",
                             SCOREP_USER_METRIC_TYPE_DOUBLE, SCOREP_USER_METRIC_CONTEXT_GLOBAL);
#endif
}

const char* CMSB::AllgatherBench::getMicroBenchName () const {

    return "MPI_Allgather";
}

void CMSB::AllgatherBench::writeResultToProfile () const {

#ifdef USE_SCOREP
	if (_myRank == 0) {		// Only one rank should update SCORE-P's metrics
		SCOREP_USER_METRIC_DOUBLE (bench_AllgatherBench_metric, getMicroBenchResult ());
	}
#endif
}

//======================================================================
