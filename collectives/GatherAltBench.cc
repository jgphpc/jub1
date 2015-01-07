#include <cmath>
#include <cstring>
#include <iostream>
#include <mpi.h>
#include "GatherAltBench.h"

#ifdef USE_SCOREP
#include <scorep/SCOREP_User.h>
#endif



//======================================================================

#ifdef USE_SCOREP
SCOREP_USER_METRIC_LOCAL (bench_GatherAltBench_metric);
#endif


CMSB::GatherAltBench::GatherAltBench (unsigned int messageSize) {

    _msgSize = messageSize;
}

CMSB::GatherAltBench::~GatherAltBench () {
}

void CMSB::GatherAltBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {

	CMSB::CollectivesBench::init (worldComm, benchInfo);
#ifdef USE_SCOREP
    SCOREP_USER_METRIC_INIT (bench_GatherAltBench_metric, "GatherAlt timing", "usec",
                             SCOREP_USER_METRIC_TYPE_DOUBLE, SCOREP_USER_METRIC_CONTEXT_GLOBAL);
#endif
}

const char* CMSB::GatherAltBench::getMicroBenchName () const {

    return "MPI_Gather_alt";
}

void CMSB::GatherAltBench::writeResultToProfile () const {

#ifdef USE_SCOREP
	if (_myRank == 0) {		// Only one rank should update SCORE-P's metrics
		SCOREP_USER_METRIC_DOUBLE (bench_GatherAltBench_metric, getMicroBenchResult ());
	}
#endif
}

void CMSB::GatherAltBench::performMPICollectiveFunc () {
	
	// Alternative implementation of MPI_Bcast.
	// MPI_Gather (_benchInfo._sendBuff, _msgSize, MPI_DOUBLE, _benchInfo._recvBuff, _msgSize, MPI_DOUBLE, 0, _worldComm);
	// The idea is to use MST implementation to get a clear expectation how much time
	// the bcast operation will take. The expected model is: T = log(P)*alpha+(p-1)/p*Nbeta,
	// where p - number of processes, N - message size in bytes, alpha - latency, and beta -
	// bandwidth.
	
	std::memcpy (_benchInfo._recvBuff + _msgSize*_myRank, _benchInfo._sendBuff, _msgSize*sizeof(double));
	//if (_myRank == 0) {
	//	std::cout << "GatherAltBench (before) - rank " << _myRank << ": ";
	//	for (int i = 0; i < _msgSize*_numProcs; i++) {
	//		std::cout << _benchInfo._recvBuff[i] << "  ";
	//	}
	//	std::cout << std::endl;
	//}
	gatherAltImpl (0, 0, _numProcs-1);
	
	// Check for correctness of the operation - every rank should have the root's buffer, which
	// contains ones
	//if (_myRank == 0) {
	//	std::cout << "GatherAltBench (after) - rank " << _myRank << ": ";
	//	for (int i = 0; i < _msgSize*_numProcs; i++) {
	//		std::cout << _benchInfo._recvBuff[i] << "  ";
	//	}
	//	std::cout << std::endl;
	//}
}

void CMSB::GatherAltBench::gatherAltImpl (int root, int left, int right) {
											
	if (left == right)
		return;
		
	int mid = (left + right) / 2;
	int src = left;
	if (root <= mid)
		src = right;
	
	if (_myRank <= mid && root <= mid)
		gatherAltImpl (root, left, mid);
	else if (_myRank <= mid && root > mid)
		gatherAltImpl (src, left, mid);
	else if (_myRank > mid && root <= mid)
		gatherAltImpl (src, mid+1, right);
	else if (_myRank > mid && root > mid)
		gatherAltImpl (root, mid+1, right);
		
	if (root <= mid) {
		int data_length = right - (mid+1) + 1;
		if (_myRank == src)
			MPI_Send (_benchInfo._recvBuff + _msgSize*(mid+1), data_length, MPI_DOUBLE, root, 0, _worldComm);
		if (_myRank == root) {
			MPI_Status status;
			MPI_Recv (_benchInfo._recvBuff + _msgSize*(mid+1), data_length, MPI_DOUBLE, src, 0, _worldComm, &status);
		}
	}
	else {
		int data_length = mid - left + 1;
		if (_myRank == src)
			MPI_Send (_benchInfo._recvBuff + _msgSize*left, data_length, MPI_DOUBLE, root, 0, _worldComm);
		if (_myRank == root) {
			MPI_Status status;
			MPI_Recv (_benchInfo._recvBuff + _msgSize*left, data_length, MPI_DOUBLE, src, 0, _worldComm, &status);
		}
	}
}

//======================================================================
