#ifndef __Reduce_Alt_BENCH_H__
#define __Reduce_Alt_BENCH_H__


#include "CollectivesBench.h"


namespace CMSB {

	/**
	 * Represents alternative reduce benchmark.
	 */
	class ReduceAltBench : public CMSB::CollectivesBench {

	public:

		ReduceAltBench  (unsigned int messageSize);
		virtual ~ReduceAltBench ();
		
		virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
        virtual const char* getMicroBenchName () const;
		virtual void writeResultToProfile () const;

	protected:
		virtual void performMPICollectiveFunc ();
		void reduceAltImpl (int root, int left, int right);
		
		double* _tempBuff;
	};
	
}


#endif   // __Reduce_Alt_BENCH_H__
