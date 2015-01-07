#ifndef __Alltoall_BENCH_H__
#define __Alltoall_BENCH_H__


#include "CollectivesBench.h"


namespace CMSB {

	/**
	 * Represents Alltoall benchmark.
	 */
	class AlltoallBench : public CMSB::CollectivesBench {

	public:

		AlltoallBench  (unsigned int messageSize);
		virtual ~AlltoallBench ();
		
		virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
        virtual const char* getMicroBenchName () const;
		virtual void writeResultToProfile () const;

	protected:
		virtual void performMPICollectiveFunc ();
	};
	
}


#endif   // __Alltoall_BENCH_H__
