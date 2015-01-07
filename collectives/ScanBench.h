#ifndef __Scan_BENCH_H__
#define __Scan_BENCH_H__


#include "CollectivesBench.h"


namespace CMSB {

	/**
	 * Represents Scan benchmark.
	 */
	class ScanBench : public CMSB::CollectivesBench {

	public:

		ScanBench  (unsigned int messageSize);
		virtual ~ScanBench ();
		
		virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
        virtual const char* getMicroBenchName () const;
		virtual void writeResultToProfile () const;

	protected:
		virtual void performMPICollectiveFunc ();
	};
	
}


#endif   // __Scan_BENCH_H__
