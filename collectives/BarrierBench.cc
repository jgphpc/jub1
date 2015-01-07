#include <mpi.h>
#include "BarrierBench.h"

#ifdef USE_SCOREP
#include <scorep/SCOREP_User.h>
#endif


#ifdef USE_SCOREP
SCOREP_USER_METRIC_LOCAL (bench_BarrierBench_metric);
#endif

//======================================================================

CMSB::BarrierBench::BarrierBench () {
}

CMSB::BarrierBench::~BarrierBench () {
}

void CMSB::BarrierBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {
	
	CMSB::CollectivesBench::init (worldComm, benchInfo);

#ifdef USE_SCOREP
    SCOREP_USER_METRIC_INIT (bench_BarrierBench_metric, "Barrier timing", "usec",
                             SCOREP_USER_METRIC_TYPE_DOUBLE, SCOREP_USER_METRIC_CONTEXT_GLOBAL);
#endif
}

void CMSB::BarrierBench::performMPICollectiveFunc () {

    MPI_Barrier (_worldComm);
}

const char* CMSB::BarrierBench::getMicroBenchName () const {

    return "MPI_Barrier";
}

void CMSB::BarrierBench::writeResultToProfile () const {

#ifdef USE_SCOREP
	if (_myRank == 0) {		// Only one rank should update SCORE-P's metrics
		SCOREP_USER_METRIC_DOUBLE (bench_BarrierBench_metric, getMicroBenchResult ());
	}
#endif
}


//======================================================================
