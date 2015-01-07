#ifndef __Allgather_BENCH_H__
#define __Allgather_BENCH_H__


#include "CollectivesBench.h"


namespace CMSB {

	/**
	 * Represents Allgather benchmark.
	 */
	class AllgatherBench : public CMSB::CollectivesBench {

	public:

		AllgatherBench  (unsigned int messageSize);
		virtual ~AllgatherBench ();
		
		virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
        virtual const char* getMicroBenchName () const;
		virtual void writeResultToProfile () const;

	protected:
		virtual void performMPICollectiveFunc ();
	};
	
}


#endif   // __Allgather_BENCH_H__
