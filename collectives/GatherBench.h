#ifndef __Gather_BENCH_H__
#define __Gather_BENCH_H__


#include "CollectivesBench.h"


namespace CMSB {

	/**
	 * Represents Gather benchmark.
	 */
	class GatherBench : public CMSB::CollectivesBench {

	public:

		GatherBench  (unsigned int messageSize);
		virtual ~GatherBench ();
		
		virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
        virtual const char* getMicroBenchName () const;
		virtual void writeResultToProfile () const;

	protected:
		virtual void performMPICollectiveFunc ();
	};
	
}


#endif   // __Gather_BENCH_H__
