#ifndef __COMM_DUP_BENCH_H__
#define __COMM_DUP_BENCH_H__


#include <mpi.h>
#include "OverheadsBench.h"


namespace CMSB {

    class CommdupBench : public CMSB::OverheadsBench {
    public:
        CommdupBench () {}
        virtual ~CommdupBench () {}
        
		virtual const char* getMicroBenchName  () const { return "MPI_Comm_dup"; }
		virtual void writeResultToProfile      () const { }
    
    protected:
        virtual unsigned int runOverheadFunc ()         {
            for (int i = 0; i < NUM_INTERNAL_ITERS; i++) 
                MPI_Comm_dup (_worldComm, &_newComm[i]); 
             return NUM_INTERNAL_ITERS;
        }
        virtual void cleanupOverheadFunc ()     {
            for (int i = 0; i < NUM_INTERNAL_ITERS; i++) 
                MPI_Comm_free (&_newComm[i]);
        }
        
        MPI_Comm _newComm[NUM_INTERNAL_ITERS];
    };

}

#endif      // __COMM_CREATE_BENCH_H__
