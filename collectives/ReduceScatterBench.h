#ifndef __ReduceScatter_BENCH_H__
#define __ReduceScatter_BENCH_H__


#include "CollectivesBench.h"


namespace CMSB {

	/**
	 * Represents ReduceScatter benchmark.
	 */
	class ReduceScatterBench : public CMSB::CollectivesBench {

	public:

		ReduceScatterBench  (unsigned int messageSize);
		virtual ~ReduceScatterBench ();
		
		virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
        virtual const char* getMicroBenchName () const;
		virtual void writeResultToProfile () const;

	protected:
		virtual void performMPICollectiveFunc ();
	};
	
}


#endif   // __ReduceScatter_BENCH_H__
