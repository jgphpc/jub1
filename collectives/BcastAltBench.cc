#include <cmath>
#include <iostream>
#include <mpi.h>
#include "BcastAltBench.h"

#ifdef USE_SCOREP
#include <scorep/SCOREP_User.h>
#endif



//======================================================================

#ifdef USE_SCOREP
SCOREP_USER_METRIC_LOCAL (bench_BcastAltBench_metric);
#endif

CMSB::BcastAltBench::BcastAltBench (unsigned int messageSize) {

    _msgSize = messageSize;
}

CMSB::BcastAltBench::~BcastAltBench () {
}

void CMSB::BcastAltBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {

	CMSB::CollectivesBench::init (worldComm, benchInfo);
#ifdef USE_SCOREP
    SCOREP_USER_METRIC_INIT (bench_BcastAltBench_metric, "BcastAlt timing", "usec",
                             SCOREP_USER_METRIC_TYPE_DOUBLE, SCOREP_USER_METRIC_CONTEXT_GLOBAL);
#endif
}

const char* CMSB::BcastAltBench::getMicroBenchName () const {

    return "MPI_Bcast_alt";
}

void CMSB::BcastAltBench::writeResultToProfile () const {

#ifdef USE_SCOREP
	if (_myRank == 0) {		// Only one rank should update SCORE-P's metrics
		SCOREP_USER_METRIC_DOUBLE (bench_BcastAltBench_metric, getMicroBenchResult ());
	}
#endif
}

void CMSB::BcastAltBench::performMPICollectiveFunc () {
	
	// Alternative implementation of MPI_Bcast.
	// Invoked as: MPI_Bcast (_benchInfo._sendBuff, _msgSize, MPI_DOUBLE, 0, _worldComm)
	// The idea is to use MST implementation to get a clear expectation how much time
	// the bcast operation will take. The expected model is: T = log(P)*[alpha+N*beta],
	// where p - number of processes, N - message size in bytes, alpha - latency, and beta -
	// bandwidth.
	// The algorithm works by recursively subdiving the processes into two equal groups, picking
	// one process from the other group and sending it the broadcasted message. Since recursion
	// depth is log(P) and the sending occurs in parallel we get the aforementioned model.
	// This algorithm assumes the message size is relative small. For bigger messages better
	// algorithms exist.
	
	bcastAltImpl (0, 0, _numProcs-1);
	
	// Check for correctness of the operation - every rank should have the root's buffer, which
	// contains ones
	//for (int i = 0; i < _msgSize; i++) {
	//	if (std::fabs (_benchInfo._sendBuff[i] / 1.0 - 1.0) > 0.000001) {
	//		std::cout << "BcastAltBench - rank " << _myRank << ": ";
	//		for (int i = 0; i < _msgSize; i++)
	//			std::cout << _benchInfo._sendBuff[i] << "  ";
	//		std::cout << std::endl;
	//		std::cout << "BcastAltBench - rank " << _myRank << ": buffer verification failed." << std::endl;
	//		MPI_Abort (_worldComm, 1);
	//	}
	//}
}

void CMSB::BcastAltBench::bcastAltImpl (int root, int left, int right) {
											
	if (left == right)
		return;
		
	int mid = (left + right) / 2;
	int dest = left;
	if (root <= mid)
		dest = right;
		
	if (_myRank == root)
		MPI_Send (_benchInfo._sendBuff, _msgSize, MPI_DOUBLE, dest, 0, _worldComm);
	if (_myRank == dest) {
		MPI_Status status;
		MPI_Recv (_benchInfo._sendBuff, _msgSize, MPI_DOUBLE, root, 0, _worldComm, &status);
	}
	
	if (_myRank <= mid && root <= mid)
		bcastAltImpl (root, left, mid);
	else if (_myRank <= mid && root > mid)
		bcastAltImpl (dest, left, mid);
	else if (_myRank > mid && root <= mid)
		bcastAltImpl (dest, mid+1, right);
	else if (_myRank > mid && root > mid)
		bcastAltImpl (root, mid+1, right);
}

//======================================================================
