#ifndef __Alltoallv_BENCH_H__
#define __Alltoallv_BENCH_H__


#include "CollectivesBench.h"


namespace CMSB {

	/**
	 * Represents Alltoallv benchmark.
	 */
	class AlltoallvBench : public CMSB::CollectivesBench {

	public:

		AlltoallvBench  (unsigned int messageSize);
		virtual ~AlltoallvBench ();
		
		virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
        virtual const char* getMicroBenchName () const;
		virtual void writeResultToProfile () const;

	protected:
		virtual void performMPICollectiveFunc ();
	};
	
}


#endif   // __Alltoallv_BENCH_H__
