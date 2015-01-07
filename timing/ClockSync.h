/*
 * Copyright (c) 2009 The Trustees of Indiana University and Indiana
 *                    University Research and Technology
 *                    Corporation.  All rights reserved.
 *
 * Author(s): Torsten Hoefler <htor@cs.indiana.edu>
 *
 */


#ifndef __CLOCK_SYNC_H__
#define __CLOCK_SYNC_H__


#include <mpi.h>

/* define synchronization method here */
//#define SYNC_WINDOW
//#define SYNC_BARRIER


namespace CMSB {
    
    struct TimeSyncInfo {

        TimeSyncInfo () :
            _comm    (MPI_COMM_WORLD),
            _esttime (0.0),
            _window  (0.0) {
        }
        
        MPI_Comm _comm;
        double _esttime; /* estimated maximum single step time (=max(estnbctime, estmpitime)) in usec */
        double _window; 	/* window to perform operation */
    };

    void sync_init_stage1 (CMSB::TimeSyncInfo* syncInfo);
    void sync_init_stage2 (CMSB::TimeSyncInfo* syncInfo);
    double nbcb_sync (CMSB::TimeSyncInfo* syncInfo);
}

#endif
