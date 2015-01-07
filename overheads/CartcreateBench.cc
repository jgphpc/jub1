

#include <cmath>
#include <iostream>
#include "CartcreateBench.h"


CMSB::CartcreateBench::CartcreateBench (int num_dims) 
    : _numDims (num_dims), _dims (NULL), _periods (NULL) {
}


CMSB::CartcreateBench::~CartcreateBench () {
    
    delete[] _dims; 
    delete[] _periods;
}


void CMSB::CartcreateBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {

    CMSB::OverheadsBench::init (worldComm, benchInfo);
    
    delete[] _dims; 
    delete[] _periods;
    
    _dims = new int[_numDims]; 
    _periods = new int[_numDims];
    int num_procs_log = fastLog (_numProcs);
    switch (_numDims) {
        case 1:
            _dims[0] = _numProcs; 
            _periods[0] = 0;
            break;
        case 2:
            _dims[0] = fastPow (num_procs_log / 2);
            _dims[1] = _numProcs / _dims[0]; 
            _periods[0] = _periods[1] = 0;
            break;
        case 3:
            int exp = num_procs_log / 3;
            _dims[0] = _dims[1] = fastPow (exp);
            _dims[2] = fastPow (num_procs_log - (2 * exp));
            _periods[0] = _periods[1] = _periods[2] = 0;
            break;
    };
}


int CMSB::CartcreateBench::fastLog (int powerTwoVal) {
    
    int log = 0;
    while (powerTwoVal > 1) {
        log++;
        powerTwoVal >>= 1;
    }
    return log;
}


int CMSB::CartcreateBench::fastPow (int exp) {
    
    int result = 1;
    while (exp > 0) {
        result <<= 1;
        exp--;
    }
    return result;
}
