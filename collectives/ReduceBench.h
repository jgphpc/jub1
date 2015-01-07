#ifndef __Reduce_BENCH_H__
#define __Reduce_BENCH_H__


#include "CollectivesBench.h"


namespace CMSB {

	/**
	 * Represents Reduce benchmark.
	 */
	class ReduceBench : public CMSB::CollectivesBench {

	public:

		ReduceBench  (unsigned int messageSize);
		virtual ~ReduceBench ();
		
		virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
        virtual const char* getMicroBenchName () const;
		virtual void writeResultToProfile () const;

	protected:
		virtual void performMPICollectiveFunc ();
	};
	
}


#endif   // __Reduce_BENCH_H__
