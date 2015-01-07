#include <mpi.h>
#include "AlltoallBench.h"
#include "AllgatherBench.h"
#include "AllreduceBench.h"
#include "GatherBench.h"
#include "ReduceBench.h"
#include "BcastBench.h"
#include "ScatterBench.h"
#include "ScanBench.h"
#include "ReduceScatterBench.h"
#include "AlltoallvBench.h"
#include "AllgathervBench.h"
#include "GathervBench.h"
#include "ScattervBench.h"



//======================================================================


void CMSB::AlltoallBench::performMPICollectiveFunc () {

    MPI_Alltoall (_benchInfo._sendBuff, _msgSize, MPI_DOUBLE, _benchInfo._recvBuff, _msgSize, MPI_DOUBLE, _worldComm);
}


//======================================================================


void CMSB::AllgatherBench::performMPICollectiveFunc () {

    MPI_Allgather (_benchInfo._sendBuff, _msgSize, MPI_DOUBLE, _benchInfo._recvBuff, _msgSize, MPI_DOUBLE, _worldComm);
}


//======================================================================


void CMSB::AllreduceBench::performMPICollectiveFunc () {

    MPI_Allreduce (_benchInfo._sendBuff, _benchInfo._recvBuff, _msgSize, MPI_DOUBLE, MPI_SUM, _worldComm);
}


//======================================================================


void CMSB::GatherBench::performMPICollectiveFunc () {

    MPI_Gather (_benchInfo._sendBuff, _msgSize, MPI_DOUBLE, _benchInfo._recvBuff, _msgSize, MPI_DOUBLE, 0, _worldComm);
}


//======================================================================


void CMSB::ReduceBench::performMPICollectiveFunc () {

    MPI_Reduce (_benchInfo._sendBuff, _benchInfo._recvBuff, _msgSize, MPI_DOUBLE, MPI_SUM, 0, _worldComm);
}


//======================================================================


void CMSB::BcastBench::performMPICollectiveFunc () {

    MPI_Bcast (_benchInfo._sendBuff, _msgSize, MPI_DOUBLE, 0, _worldComm);
}


//======================================================================


void CMSB::ScatterBench::performMPICollectiveFunc () {

    MPI_Scatter (_benchInfo._sendBuff, _msgSize, MPI_DOUBLE, _benchInfo._recvBuff, _msgSize, MPI_DOUBLE, 0, _worldComm);
}


//======================================================================


void CMSB::ScanBench::performMPICollectiveFunc () {

    MPI_Scan (_benchInfo._sendBuff, _benchInfo._recvBuff, _msgSize, MPI_DOUBLE, MPI_SUM, _worldComm);
}


//======================================================================


void CMSB::ReduceScatterBench::performMPICollectiveFunc () {

    MPI_Reduce_scatter (_benchInfo._sendBuff, _benchInfo._recvBuff, _benchInfo._recvCounts, MPI_DOUBLE, MPI_SUM, _worldComm);
}


//======================================================================


void CMSB::AlltoallvBench::performMPICollectiveFunc () {

    MPI_Alltoallv (_benchInfo._sendBuff, _benchInfo._sendCounts, _benchInfo._sendDispls, MPI_DOUBLE,
                   _benchInfo._recvBuff, _benchInfo._recvCounts, _benchInfo._recvDispls, MPI_DOUBLE,
                   _worldComm);
}


//======================================================================


void CMSB::AllgathervBench::performMPICollectiveFunc () {

    MPI_Allgatherv (_benchInfo._sendBuff, _msgSize, MPI_DOUBLE, _benchInfo._recvBuff, _benchInfo._recvCounts,
                    _benchInfo._recvDispls, MPI_DOUBLE, _worldComm);
}


//======================================================================


void CMSB::GathervBench::performMPICollectiveFunc () {

    MPI_Gatherv (_benchInfo._sendBuff, _msgSize, MPI_DOUBLE, _benchInfo._recvBuff, _benchInfo._recvCounts,
                 _benchInfo._recvDispls, MPI_DOUBLE, 0, _worldComm);
}


//======================================================================


void CMSB::ScattervBench::performMPICollectiveFunc () {

    MPI_Scatterv (_benchInfo._sendBuff, _benchInfo._sendCounts, _benchInfo._sendDispls, MPI_DOUBLE,
                  _benchInfo._recvBuff, _msgSize, MPI_DOUBLE, 0, _worldComm);
}


//======================================================================


