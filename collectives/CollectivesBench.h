#ifndef __COLLECTIVES_BENCH_H__
#define __COLLECTIVES_BENCH_H__

#include <mpi.h>
#include <MicroBench.h>
#include <vector>


namespace CMSB {

	class CollectivesBench : public CMSB::MicroBench {

	public:
    
        static const int NUM_WARMPUP_ITERS = 10;
        
        // Iterations are performed in these rounds to ensure a good
        // response in case the sync window has to increased
        static const int NUM_ITERS_ROUND = 20;	
												
        // Sufficient for quite accurate sample mean
        static const int NUM_ITERS_TOTAL = 400;	


		CollectivesBench  ();
		virtual ~CollectivesBench ();

		virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
        virtual void runMicroBench (CMSB::TimeSyncInfo* syncInfo);
		virtual const char* getMicroBenchName  () const = 0;
		virtual double getMicroBenchResult     () const { return _avgRunTime; }
		virtual void writeResultToProfile      () const = 0;
		virtual unsigned int getMemConsumption () const { return sizeof (CMSB::CollectivesBench); }
		
	protected:
		virtual void performMPICollectiveFunc () = 0;
    
		int 			_myRank;
		int 			_numProcs;
		double 			_avgRunTime;
		unsigned int	_msgSize;	// In number of doubles to send
	};
	
	void createCollectiveMicroBenches (std::vector<MicroBench*>& benchmarks, unsigned int messageSizePerProc);
	void createCollectiveMicroBenchesMinimalVer (std::vector<MicroBench*>& benchmarks, unsigned int messageSizePerProc);
    void createCollectiveExtraSizeMicroBenches (std::vector<MicroBench*>& benchmarks, unsigned int messageSizePerProc);
}


#endif   // __COLLECTIVES_BENCH_H__
