#include <mpi.h>
#include "ScanBench.h"

#ifdef USE_SCOREP
#include <scorep/SCOREP_User.h>
#endif




//======================================================================

#ifdef USE_SCOREP
SCOREP_USER_METRIC_LOCAL (bench_ScanBench_metric);
#endif


CMSB::ScanBench::ScanBench (unsigned int messageSize) {

    _msgSize = messageSize;
}

CMSB::ScanBench::~ScanBench () {
}

void CMSB::ScanBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {

	CMSB::CollectivesBench::init (worldComm, benchInfo);
#ifdef USE_SCOREP
    SCOREP_USER_METRIC_INIT (bench_ScanBench_metric, "Scan timing", "usec",
                             SCOREP_USER_METRIC_TYPE_DOUBLE, SCOREP_USER_METRIC_CONTEXT_GLOBAL);
#endif
}

const char* CMSB::ScanBench::getMicroBenchName () const {

    return "MPI_Scan";
}

void CMSB::ScanBench::writeResultToProfile () const {

#ifdef USE_SCOREP
	if (_myRank == 0) {		// Only one rank should update SCORE-P's metrics
		SCOREP_USER_METRIC_DOUBLE (bench_ScanBench_metric, getMicroBenchResult ());
	}
#endif
}

//======================================================================
