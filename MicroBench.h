#ifndef __MICRO_BENCH_H__
#define __MICRO_BENCH_H__

#include <mpi.h>
#include "timing/ClockSync.h"

namespace CMSB {

	class MicroBench {

	public:    

		struct MicroBenchInfo {
			
			MicroBenchInfo () :
				_sendBuff   (NULL), 
				_recvBuff   (NULL), 
				_sendCounts (NULL),
				_sendDispls (NULL),
				_recvCounts (NULL),
				_recvDispls (NULL) {}
			
			double* _sendBuff;
            unsigned int _sBuffLen;     // In bytes
			double* _recvBuff;
            unsigned int _rBuffLen;     // In bytes
			int*	_sendCounts;
			int*	_sendDispls;
			int*	_recvCounts;
			int*	_recvDispls;
		};

		MicroBench  () : _worldComm (MPI_COMM_WORLD) {}
		virtual ~MicroBench ()	{}

		virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo)	
        { _worldComm = worldComm; _benchInfo = *benchInfo; }
        
		virtual void runMicroBench (CMSB::TimeSyncInfo* syncInfo) = 0;
		virtual const char* getMicroBenchName  () const = 0;
		virtual double getMicroBenchResult     () const = 0;
		virtual void writeResultToProfile      () const = 0;
		virtual unsigned int getMemConsumption () const = 0;
		
	protected:
		MPI_Comm        _worldComm;
        MicroBenchInfo  _benchInfo;
	};

}

#endif   // __MICRO_BENCH_H__
