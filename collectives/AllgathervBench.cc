#include <mpi.h>
#include "AllgathervBench.h"

#ifdef USE_SCOREP
#include <scorep/SCOREP_User.h>
#endif




//======================================================================

#ifdef USE_SCOREP
SCOREP_USER_METRIC_LOCAL (bench_AllgathervBench_metric);
#endif


CMSB::AllgathervBench::AllgathervBench (unsigned int messageSize) {

    _msgSize = messageSize;
}

CMSB::AllgathervBench::~AllgathervBench () {
}

void CMSB::AllgathervBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {

	CMSB::CollectivesBench::init (worldComm, benchInfo);
#ifdef USE_SCOREP
    SCOREP_USER_METRIC_INIT (bench_AllgathervBench_metric, "Allgatherv timing", "usec",
                             SCOREP_USER_METRIC_TYPE_DOUBLE, SCOREP_USER_METRIC_CONTEXT_GLOBAL);
#endif
}

const char* CMSB::AllgathervBench::getMicroBenchName () const {

    return "MPI_Allgatherv";
}

void CMSB::AllgathervBench::writeResultToProfile () const {

#ifdef USE_SCOREP
	if (_myRank == 0) {		// Only one rank should update SCORE-P's metrics
		SCOREP_USER_METRIC_DOUBLE (bench_AllgathervBench_metric, getMicroBenchResult ());
	}
#endif
}

//======================================================================
