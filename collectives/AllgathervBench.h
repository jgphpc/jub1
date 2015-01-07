#ifndef __Allgatherv_BENCH_H__
#define __Allgatherv_BENCH_H__


#include "CollectivesBench.h"


namespace CMSB {

	/**
	 * Represents Allgatherv benchmark.
	 */
	class AllgathervBench : public CMSB::CollectivesBench {

	public:

		AllgathervBench  (unsigned int messageSize);
		virtual ~AllgathervBench ();
		
		virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
        virtual const char* getMicroBenchName () const;
		virtual void writeResultToProfile () const;

	protected:
		virtual void performMPICollectiveFunc ();
	};
	
}


#endif   // __Allgatherv_BENCH_H__
