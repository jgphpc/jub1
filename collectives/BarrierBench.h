#ifndef __Barrier_BENCH_H__
#define __Barrier_BENCH_H__


#include "CollectivesBench.h"


namespace CMSB {

	/**
	 * Base class for Barrier benchmark.
	 */
	class BarrierBench : public CMSB::CollectivesBench {

	public:

		BarrierBench  ();
		virtual ~BarrierBench ();
		
		virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
        virtual const char* getMicroBenchName () const;
		virtual void writeResultToProfile () const;

	protected:
		virtual void performMPICollectiveFunc ();
	};
	
}


#endif   // __Barrier_BENCH_H__
