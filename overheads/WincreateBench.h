#ifndef __WIN_CREATE_BENCH_H__
#define __WIN_CREATE_BENCH_H__


#include <mpi.h>
#include "OverheadsBench.h"


namespace CMSB {

    class WincreateBench : public CMSB::OverheadsBench {
    public:
        WincreateBench () {}
        virtual ~WincreateBench () {}
        
		virtual const char* getMicroBenchName  () const { return "MPI_Win_create"; }
		virtual void writeResultToProfile      () const { }
    
    protected:
        virtual unsigned int runOverheadFunc () { 
            for (int i = 0; i < NUM_INTERNAL_ITERS; i++) 
                MPI_Win_create (_benchInfo._sendBuff, _benchInfo._sBuffLen, sizeof(double), 
                                MPI_INFO_NULL, _worldComm, &_onesidedWin[i]);
            return NUM_INTERNAL_ITERS;
        }
        virtual void cleanupOverheadFunc () { 
            for (int i = 0; i < NUM_INTERNAL_ITERS; i++)
                MPI_Win_free (&_onesidedWin[i]);
        }
        
        MPI_Win _onesidedWin[NUM_INTERNAL_ITERS];
    };

}

#endif      // __COMM_CREATE_BENCH_H__
