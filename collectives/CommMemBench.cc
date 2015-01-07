#include <mpi.h>
#include "CommMemBench.h"



//======================================================================

CMSB::CommMemBench::CommMemBench () {
}

CMSB::CommMemBench::~CommMemBench () {
    
    
    for (int i = 0; i < CMSB::CommMemBench::NUM_COMMS; i++) {
        MPI_Comm_free (&_commsCreated[i]);
        //MPI_Comm_free (&_commsDuped[i]);
    }
    MPI_Comm_free (&_commAfterSplit);
    
}

void CMSB::CommMemBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {
    
    CMSB::MicroBench::init (worldComm, benchInfo);

    MPI_Comm_rank  (_worldComm, &_myRank);
    MPI_Comm_size  (_worldComm, &_numProcs);
    MPI_Comm_group (_worldComm, &_worldGrp);
}

void CMSB::CommMemBench::runMicroBench (CMSB::TimeSyncInfo* syncInfo) {
    
    for (int i = 0; i < CMSB::CommMemBench::NUM_COMMS; i++) {
        MPI_Comm_create (_worldComm, _worldGrp, &_commsCreated[i]);
        //MPI_Comm_dup (_worldComm, &_commsDuped[i]);
    }
    MPI_Comm_split (_worldComm, _myRank % (_numProcs/CMSB::CommMemBench::SUBGRP_SIZE), _myRank, &_commAfterSplit);
}


//======================================================================
