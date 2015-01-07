#ifndef __Allreduce_BENCH_H__
#define __Allreduce_BENCH_H__


#include "CollectivesBench.h"


namespace CMSB {

	/**
	 * Represents Allreduce benchmark.
	 */
	class AllreduceBench : public CMSB::CollectivesBench {

	public:

		AllreduceBench  (unsigned int messageSize);
		virtual ~AllreduceBench ();
		
		virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
        virtual const char* getMicroBenchName () const;
		virtual void writeResultToProfile () const;

	protected:
		virtual void performMPICollectiveFunc ();
	};
	
}


#endif   // __Allreduce_BENCH_H__
