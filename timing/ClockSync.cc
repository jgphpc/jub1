/*
 * Copyright (c) 2009 The Trustees of Indiana University and Indiana
 *                    University Research and Technology
 *                    Corporation.  All rights reserved.
 *
 * Author(s): Torsten Hoefler <htor@cs.indiana.edu>
 *
 */
 
#include <iostream>
#include "elg_pform_defs.h"
#include "ClockSync.h"

#ifdef SYNC_BARRIER
// Simple MPI_Barrier synchronization mechanism
void CMSB::sync_init_stage1 (CMSB::TimeSyncInfo* syncInfo) {

}

void CMSB::sync_init_stage2 (CMSB::TimeSyncInfo* syncInfo) {

}

double CMSB::nbcb_sync (CMSB::TimeSyncInfo* syncInfo) {
	
	MPI_Barrier (syncInfo->_comm);
	return 0;
}
#endif

#ifdef SYNC_DISSEMINATION
// Internal dissemination algorithm
void CMSB::sync_init_stage1 (CMSB::TimeSyncInfo* syncInfo) {

}

void CMSB::sync_init_stage2 (CMSB::TimeSyncInfo* syncInfo) {

}

// Done at the beginning of every new nprocs
double CMSB::nbcb_sync (CMSB::TimeSyncInfo* syncInfo) {
    
    MPI_Comm comm = syncInfo->_comm;
    int round = -1, src, dst, maxround, size, i, rank;
    char buf;

    MPI_Comm_size (comm, &size);
    MPI_Comm_rank (comm, &rank);

    // round starts with 0 -> round - 1
    maxround = (int)ceil((double)log(size)) - 1;
            
    do {
        round++;
        dst = (rank + round) % size;
        MPI_Send (&buf, 1, MPI_BYTE, dst, 0, comm);
        // + size*size that the modulo does never get negative !
        src = ((rank - round) + size*size) % size;
        MPI_Recv (&buf, 1, MPI_BYTE, src, 0, comm, MPI_STATUS_IGNORE);
    } while (round < maxround);     

    return 0;
}
#endif

#ifdef SYNC_WINDOW
static double *diffs = NULL;    // global array of all diffs to all ranks - only
                                // completely valid on rank 0
static double gdiff = 0;    // the is the final time diff to rank 0 :-)
static double gnext = 0;    // start-time for next round - synchronized with rank 0

#define NUMBER_SMALLER 100  // do RTT measurement until n successive
                            // messages are *not* smaller than the current
                            // smallest one
                           
#define MAX_DOUBLE 1e99

// time-window based synchronization mechanism 
void CMSB::sync_init_stage1 (CMSB::TimeSyncInfo* syncInfo) {
    
    int p, r, res, dist, round;
    MPI_Comm comm = syncInfo->_comm;

    res = MPI_Comm_rank (comm, &r);
    res = MPI_Comm_size (comm, &p);
  
    // reallocate the diffs array with the right size
    if (diffs != NULL) delete[] diffs;
    diffs = new double[p]();    // initialize all values to 0
  
    // check if p is power of 2
    { 
        int i=1;
        while((i = i << 1) < p) {};
        if(i != p) {
            std::cerr << "Communicator size" << p << " must be power of 2!" << std::endl;
            MPI_Abort (MPI_COMM_WORLD, 1);
        }
    }

    dist = 1;   // this gets left-shifted (<<) every round and is after 
                // $\lceil log_2(p) \rceil$ rounds >= p */
    round = 1;  // fun and printf round counter - not really needed
    do {
        int peer;   // synchronization peer
        int client, server;
        double tstart,  // local start time
               tend,    // local end time
               trem,    // remote time
               tmpdiff, // temporary difference to remote clock
               diff;    // difference to remote clock
               
        client = 0; server = 0;
        client = ((r % (dist << 1)) == 0);
        server = ((r % (dist << 1)) == dist);
    
        if (server) {
            peer = r - dist;
            if (peer < 0) server = 0;
        }
        if (client) {
            peer = r + dist;
            if (peer >= p) client = 0;
        }
        if(!client && !server) break;   // TODO: leave loop if no peer left -
                                        // works only for power of two process
                                        // groups

        // synchronize clocks with peer
        {
            int notsmaller = 0; // count number of RTTs that are *not* smaller than
                                // the current smallest one
            double smallest = MAX_DOUBLE;   // the current smallest time
            do {
                /* the client sends a ping to the server and waits for a pong (and
                 * takes the RTT time). It repeats this procedure until the last
                 * NUMBER_SMALLER RTTs have not been smaller than the smallest
                 * (tries to find the smallest RTT). When the smallest RTT is
                 * found, it sends a special flag (0d) to the server that it knows
                 * that the benchmark is finished. The client computes the diff
                 * with this smallest RTT with the scheme described in the paper.
                 * */
                if (client) {
                    tstart = elg_pform_wtime ();
                    res = MPI_Send (&tstart, 1, MPI_DOUBLE, peer, 0, comm);
                    res = MPI_Recv (&trem, 1, MPI_DOUBLE, peer, 0, comm, MPI_STATUS_IGNORE);
                    tend = elg_pform_wtime ();
                    tmpdiff = tstart + (tend-tstart)/2 - trem;
        
                    if (tend-tstart < smallest) {
                        smallest = tend-tstart;
                        notsmaller = 0;
                        diff = tmpdiff; // save new smallest diff-time
                    } else {
                        if (++notsmaller == NUMBER_SMALLER) {
                            // send abort flag to client
                            trem = 0;
                            res = MPI_Send (&trem, 1, MPI_DOUBLE, peer, 0, comm);
                            break;
                        }
                    }
                }

                /* The server just replies with the local time to the client
                 * requests and aborts the benchmark if the abort flag (0d) is
                 * received in any of the requests. */
                if (server) {
         
                    res = MPI_Recv (&tstart, 1, MPI_DOUBLE, peer, 0, comm, MPI_STATUS_IGNORE);
                    if(tstart == 0) break;  // this is the signal from the client to stop
                    trem = elg_pform_wtime ();     // fill in local time on server
                    res = MPI_Send (&trem, 1, MPI_DOUBLE, peer, 0, comm);
                }
                // this loop is only left with a break
            } while (1);
        }
    
        /* the client measured the time difference to his peer-server of the
         * current round. Since rank 0 is the global synchronization point,
         * rank 0's array has to be up to date and the other clients have to
         * communicate all their knowledge to rank 0 as described in the
         * paper. */
    
        if (client) {
            // all clients just measured the time difference to node r + diff (=peer)
            diffs[peer] = diff;

            /* we are a client - we need to receive all the knowledge
             * (differences) that the server we just synchronized with holds!
             * Our server has been "round-1" times client and measures
             * "round-1" diffs */
            if(round > 1) {
                double *recvbuf;    // receive the server's data
                int items, i;

                items = (1 << (round-1))-1;
                recvbuf = new double[items];
        
                res = MPI_Recv (recvbuf, items, MPI_DOUBLE, peer, 0, comm, MPI_STATUS_IGNORE);
        
                // merge data into my own field
                for(i=0; i<items; i++) {
                    diffs[peer+i+1] =  diffs[peer] /* diff to server */ + 
                            recvbuf[i] /* received time */; // TODO: + or - ???
                }
                delete[] recvbuf;
            }
        }

        if(server) {
            /* we are a server, we need to send all our knowledge (time
             * differences to our client */
    
            /* we have measured "round-1" nodes at the end of round "round"
             * and hold $2^(round-1)-1$ diffs at this time*/
            if(round > 1) {
                int i, tmpdist, tmppeer, items;
                double *sendbuf;
        
                items = (1 << (round-1))-1;
                sendbuf = new double[items];

                // fill buffer - every server holds the $2^(round-1)-1$ next diffs
                for(i=0; i<items; i++) {
                    sendbuf[i] = diffs[r+i+1];
                }
                res = MPI_Send (sendbuf, items, MPI_DOUBLE, peer, 0, comm);
                delete[] sendbuf;
            }
        }
    
        dist = dist << 1;
        round++;
        
    } while (dist < p);


    // scatter all the time diffs to the processes
    MPI_Scatter (diffs, 1, MPI_DOUBLE, &gdiff, 1, MPI_DOUBLE, 0, comm);

    // initialize window to 0
    syncInfo->_window = 0;
}

// done at the beginning of every new size
void CMSB::sync_init_stage2 (CMSB::TimeSyncInfo* syncInfo) {
    
    double bcasttime, win;    // measure MPI_Bcast() time :-/
    int i;
    MPI_Comm comm = syncInfo->_comm;

#if 0
    // we increased the window size? - we need to re-synch
    if(syncInfo->_window != 0) {
        /* save increased window size */
        win = syncInfo->_window;
        sync_init_stage1 (syncInfo);
        syncInfo->_window = win;
    }
#endif

    //if(!mpiargs->rank) printf("estimated time: %lf\n", mpiargs->esttime);
    win = syncInfo->_esttime/1e6 * 1.25;    // esttime is in usec - window
                                            // length = 125% of the
                                            // estimated MPI time

    if (win > syncInfo->_window) syncInfo->_window = win;
    MPI_Bcast (&syncInfo->_window, 1, MPI_DOUBLE, 0, comm);
  
    // it has been used before - so it is "warm" :)
    bcasttime = -elg_pform_wtime ();
    for(i=0; i<10; i++) {
        MPI_Bcast(&gnext, 1, MPI_DOUBLE, 0, comm);
    }
    bcasttime += elg_pform_wtime();
    gnext = bcasttime;  // dummy buffer
    // get maximum bcasttime
    MPI_Reduce (&gnext, &bcasttime, 1, MPI_DOUBLE, MPI_MAX, 0, comm);

    // rank 0 sets base-time to a time when the bcast is expected to be finished
    gnext = elg_pform_wtime () + bcasttime;
    MPI_Bcast (&gnext, 1, MPI_DOUBLE, 0, comm);

    gnext= gnext - gdiff;   // adjust rank 0's time to local time
}


static volatile int NBC_Dummy_var=0; /* avoid optimizations */

// for every single measurement
double CMSB::nbcb_sync (CMSB::TimeSyncInfo* syncInfo) {
  
    double err = 0;
 
    // OMPI does not send messages immediately!!!! -> drain messages
    MPI_Barrier(syncInfo->_comm);

    if (elg_pform_wtime () > gnext) {
        err = elg_pform_wtime ()-gnext;
    } else {
        // wait
        while (elg_pform_wtime () < gnext) {NBC_Dummy_var++;};
    }
  
    gnext = gnext + syncInfo->_window;
    
    return err;
}
#endif
