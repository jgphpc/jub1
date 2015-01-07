#ifndef __CommMem_BENCH_H__
#define __CommMem_BENCH_H__


#include <mpi.h>
#include <MicroBench.h>


namespace CMSB {

	/**
	 * A class for CommMem benchmark.
	 */
	class CommMemBench : public CMSB::MicroBench {

	public:

        // Number of communicators to create & duplicate
        static const int NUM_COMMS = 8;

#ifdef __bgq__        
        // Number of processes in each sub-group after communicator split
        static const int SUBGRP_SIZE = 128;
#else
		// Number of processes in each sub-group after communicator split
        static const int SUBGRP_SIZE = 8;
#endif


		CommMemBench  ();
		virtual ~CommMemBench ();
		
        virtual void init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo);
		virtual void runMicroBench (CMSB::TimeSyncInfo* syncInfo);
		virtual const char* getMicroBenchName  () const { return "CommMemBench"; }
		virtual double getMicroBenchResult     () const { return 0.0; }
		virtual void writeResultToProfile      () const {}
		virtual unsigned int getMemConsumption () const { return sizeof (CMSB::CommMemBench); }

	protected:
    
        int         _myRank;
		int         _numProcs;
        MPI_Group   _worldGrp;
        MPI_Comm _commsCreated[CMSB::CommMemBench::NUM_COMMS];
        MPI_Comm _commsDuped[CMSB::CommMemBench::NUM_COMMS];
        MPI_Comm _commAfterSplit;
	};
	
}


#endif   // __Barrier_BENCH_H__
