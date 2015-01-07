#ifndef __Gatherv_BENCH_H__
#define __Gatherv_BENCH_H__


#include "CollectivesBench.h"


namespace CMSB {

	/**
	 * Represents Gatherv benchmark.
	 */
	class GathervBench : public CMSB::CollectivesBench {

	public:

		GathervBench  (unsigned int messageSize);
		virtual ~GathervBench ();
		
		virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
        virtual const char* getMicroBenchName () const;
		virtual void writeResultToProfile () const;

	protected:
		virtual void performMPICollectiveFunc ();
	};
	
}


#endif   // __Gatherv_BENCH_H__
