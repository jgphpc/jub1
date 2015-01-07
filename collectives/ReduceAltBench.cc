#include <cmath>
#include <cstring>
#include <iostream>
#include <mpi.h>
#include "ReduceAltBench.h"

#ifdef USE_SCOREP
#include <scorep/SCOREP_User.h>
#endif




//======================================================================

#ifdef USE_SCOREP
SCOREP_USER_METRIC_LOCAL (bench_ReduceAltBench_metric);
#endif


CMSB::ReduceAltBench::ReduceAltBench (unsigned int messageSize) {

    _msgSize = messageSize;
    _tempBuff = new double[_msgSize];
}

CMSB::ReduceAltBench::~ReduceAltBench () {
	
	delete[] _tempBuff;
}

void CMSB::ReduceAltBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {

	CMSB::CollectivesBench::init (worldComm, benchInfo);
#ifdef USE_SCOREP
    SCOREP_USER_METRIC_INIT (bench_ReduceAltBench_metric, "ReduceAlt timing", "usec",
                             SCOREP_USER_METRIC_TYPE_DOUBLE, SCOREP_USER_METRIC_CONTEXT_GLOBAL);
#endif
}

const char* CMSB::ReduceAltBench::getMicroBenchName () const {

    return "MPI_Reduce_alt";
}

void CMSB::ReduceAltBench::writeResultToProfile () const {

#ifdef USE_SCOREP
	if (_myRank == 0) {		// Only one rank should update SCORE-P's metrics
		SCOREP_USER_METRIC_DOUBLE (bench_ReduceAltBench_metric, getMicroBenchResult ());
	}
#endif
}

void CMSB::ReduceAltBench::performMPICollectiveFunc () {
	
	// Alternative implementation of MPI_Reduce.
	// Invoked as: MPI_Reduce (_benchInfo._sendBuff, _benchInfo._recvBuff, _msgSize, MPI_DOUBLE, MPI_SUM, 0, _worldComm)
	// Uses MST implementation to get a clear expectation how much time reduce operation will take.
	// The expected model: log(P)*[alpha+N*beta+N*gamma],
	// where p - number of processes, N - message size in bytes, alpha - latency, beta -
	// bandwidth, and gamma - the cost of sum fn one element.
	// The algorithm works by recursively adding neighboring values to each other until everything ends up
	// summed at the root. The depth of this recursive tree is log(p).
	// This algorithm assumes the message size is relative small. For bigger messages better
	// algorithms exist.
	
	std::memcpy (_benchInfo._recvBuff, _benchInfo._sendBuff, _msgSize*sizeof(double));
	reduceAltImpl (0, 0, _numProcs-1);
	
	// Check for correctness of the operation - every rank should have the root's buffer, which
	// contains ones
	if (_myRank == 0) {
		std::cout << "ReduceAltBench - rank " << _myRank << ": ";
		for (int i = 0; i < _msgSize; i++) {
			std::cout << _benchInfo._recvBuff[i] << "  ";
		}
		std::cout << std::endl;
	}
}

void CMSB::ReduceAltBench::reduceAltImpl (int root, int left, int right) {
											
	if (left == right)
		return;
		
	int mid = (left + right) / 2;
	int src = left;
	if (root <= mid)
		src = right;
			
	if (_myRank <= mid && root <= mid)
		reduceAltImpl (root, left, mid);
	else if (_myRank <= mid && root > mid)
		reduceAltImpl (src, left, mid);
	else if (_myRank > mid && root <= mid)
		reduceAltImpl (src, mid+1, right);
	else if (_myRank > mid && root > mid)
		reduceAltImpl (root, mid+1, right);
		
	if (_myRank == src)
		MPI_Send (_benchInfo._recvBuff, _msgSize, MPI_DOUBLE, root, 0, _worldComm);
	if (_myRank == root) {
		MPI_Status status;
		MPI_Recv (_tempBuff, _msgSize, MPI_DOUBLE, src, 0, _worldComm, &status);
		for (int i = 0; i < _msgSize; i++) {
			_benchInfo._recvBuff[i] += _tempBuff[i];
		}
	}
}

//======================================================================
