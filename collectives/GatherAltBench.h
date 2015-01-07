#ifndef __Gather_Alt_BENCH_H__
#define __Gather_Alt_BENCH_H__


#include "CollectivesBench.h"


namespace CMSB {

	/**
	 * Represents alternative gather benchmark.
	 */
	class GatherAltBench : public CMSB::CollectivesBench {

	public:

		GatherAltBench  (unsigned int messageSize);
		virtual ~GatherAltBench ();
		
		virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
        virtual const char* getMicroBenchName () const;
		virtual void writeResultToProfile () const;

	protected:
		virtual void performMPICollectiveFunc ();
		void gatherAltImpl (int root, int left, int right);
		
	};
	
}


#endif   // __Gather_Alt_BENCH_H__
