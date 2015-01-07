#ifndef __Bcast_Alt_BENCH_H__
#define __Bcast_Alt_BENCH_H__


#include "CollectivesBench.h"


namespace CMSB {

	/**
	 * Represents Bcast benchmark.
	 */
	class BcastAltBench : public CMSB::CollectivesBench {

	public:

		BcastAltBench  (unsigned int messageSize);
		virtual ~BcastAltBench ();
		
		virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
        virtual const char* getMicroBenchName () const;
		virtual void writeResultToProfile () const;

	protected:
		virtual void performMPICollectiveFunc ();
		void bcastAltImpl (int root, int left, int right);
	};
	
}


#endif   // __Bcast_Alt_BENCH_H__
