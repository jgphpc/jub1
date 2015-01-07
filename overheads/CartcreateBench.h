#ifndef __CART_CREATE_BENCH_H__
#define __CART_CREATE_BENCH_H__


#include <mpi.h>
#include "OverheadsBench.h"


namespace CMSB {

    class CartcreateBench : public CMSB::OverheadsBench {
    public:
        CartcreateBench (int num_dims = 3);
        virtual ~CartcreateBench ();
        
        virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
		virtual const char* getMicroBenchName  () const { return "MPI_Cart_create"; }
		virtual void writeResultToProfile      () const { }
    
    protected:
        virtual unsigned int runOverheadFunc () {
            for (int i = 0; i < NUM_INTERNAL_ITERS; i++) 
                MPI_Cart_create (_worldComm, _numDims, _dims, _periods, true, &_newComm[i]);
            return NUM_INTERNAL_ITERS;
        }
        virtual void cleanupOverheadFunc ()     { 
            for (int i = 0; i < NUM_INTERNAL_ITERS; i++) 
                MPI_Comm_free (&_newComm[i]); 
        }
        
        inline int fastLog (int powerTwoVal);
        inline int fastPow (int exp);
        
        int _numDims;
        int* _dims;
        int* _periods;
        MPI_Comm _newComm[NUM_INTERNAL_ITERS];
    };

}

#endif      // __CART_CREATE_BENCH_H__
