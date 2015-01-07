#ifndef __Bcast_BENCH_H__
#define __Bcast_BENCH_H__


#include "CollectivesBench.h"


namespace CMSB {

	/**
	 * Represents Bcast benchmark.
	 */
	class BcastBench : public CMSB::CollectivesBench {

	public:

		BcastBench  (unsigned int messageSize);
		virtual ~BcastBench ();
		
		virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
        virtual const char* getMicroBenchName () const;
		virtual void writeResultToProfile () const;

	protected:
		virtual void performMPICollectiveFunc ();
	};
	
}


#endif   // __Bcast_BENCH_H__
