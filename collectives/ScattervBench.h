#ifndef __Scatterv_BENCH_H__
#define __Scatterv_BENCH_H__


#include "CollectivesBench.h"


namespace CMSB {

	/**
	 * Represents Scatterv benchmark.
	 */
	class ScattervBench : public CMSB::CollectivesBench {

	public:

		ScattervBench  (unsigned int messageSize);
		virtual ~ScattervBench ();
		
		virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
        virtual const char* getMicroBenchName () const;
		virtual void writeResultToProfile () const;

	protected:
		virtual void performMPICollectiveFunc ();
	};
	
}


#endif   // __Scatterv_BENCH_H__
