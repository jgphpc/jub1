#ifndef __Scatter_BENCH_H__
#define __Scatter_BENCH_H__


#include "CollectivesBench.h"


namespace CMSB {

	/**
	 * Represents Scatter benchmark.
	 */
	class ScatterBench : public CMSB::CollectivesBench {

	public:

		ScatterBench  (unsigned int messageSize);
		virtual ~ScatterBench ();
		
		virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
        virtual const char* getMicroBenchName () const;
		virtual void writeResultToProfile () const;

	protected:
		virtual void performMPICollectiveFunc ();
	};
	
}


#endif   // __Scatter_BENCH_H__
