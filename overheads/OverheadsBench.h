#ifndef __OVERHEADS_BENCH_H__
#define __OVERHEADS_BENCH_H__


#include <mpi.h>
#include <MicroBench.h>
#include <vector>


namespace CMSB {

    class OverheadsBench : public CMSB::MicroBench {
    public:
        
        static const int NUM_WARMPUP_ITERS = 1;
        
        // Iterations are performed in these rounds to ensure a good
        // response in case the sync window has to increased
        static const int NUM_ITERS_ROUND = 5;        
        static const int NUM_ITERS_TOTAL = 10;
        
        static const int NUM_INTERNAL_ITERS = 1;
        
        OverheadsBench ();
        virtual ~OverheadsBench ();
        
        virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
        //cscs virtual void runMicroBench (CMSB::TimeSyncInfo* syncInfo);
        virtual void runMicroBench ();
		virtual const char* getMicroBenchName  () const = 0;
		virtual double getMicroBenchResult     () const { return _overheadSize; }
		virtual void writeResultToProfile      () const = 0;
		virtual unsigned int getMemConsumption () const { return sizeof (CMSB::OverheadsBench); }
    
    protected:
        virtual unsigned int runOverheadFunc () = 0;
        virtual void cleanupOverheadFunc () = 0;
        
        int 		_myRank;
		int 		_numProcs;
        MPI_Group   _worldGrp;
        double      _overheadSize;
    };

    void createOverheadsMicroBenches (std::vector<MicroBench*>& benchmarks);
}

#endif      // __OVERHEADS_BENCH_H__
