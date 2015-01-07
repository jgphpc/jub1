#include <mpi.h>
#ifdef __bgq__
#include <mpix.h>
#endif
#include <algorithm>
#include <cmath>
#include <iostream>
#include <ios>
#include <iomanip>
#include <timing/elg_pform_defs.h>
#include "AlltoallBench.h"
#include "AllgatherBench.h"
#include "AllreduceBench.h"
#include "GatherBench.h"
#include "GatherAltBench.h"
#include "ReduceBench.h"
#include "ReduceAltBench.h"
#include "BcastBench.h"
#include "BcastAltBench.h"
#include "ScatterBench.h"
#include "ScanBench.h"
#include "ReduceScatterBench.h"
#include "AlltoallvBench.h"
#include "AllgathervBench.h"
#include "GathervBench.h"
#include "ScattervBench.h"
#include "BarrierBench.h"



CMSB::CollectivesBench::CollectivesBench () :
    _myRank     (0),
    _numProcs   (0),
    _avgRunTime (0.0),
    _msgSize    (0) {
		
}

CMSB::CollectivesBench::~CollectivesBench () {

}

void CMSB::CollectivesBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {
    
    CMSB::MicroBench::init (worldComm, benchInfo);

    MPI_Comm_rank (_worldComm, &_myRank);
    MPI_Comm_size (_worldComm, &_numProcs);
    
    for (int i = 0; i < _numProcs; i++) {
		_benchInfo._sendCounts[i] = _msgSize;
		_benchInfo._sendDispls[i] = i*_msgSize;
		_benchInfo._recvCounts[i] = _msgSize;
		_benchInfo._recvDispls[i] = i*_msgSize;
    }
}

void CMSB::CollectivesBench::runMicroBench (CMSB::TimeSyncInfo* syncInfo) {

	double start_time, end_time;
	std::string mpi_collective_name (getMicroBenchName ());

	// First run the warmpup runs - no need to measure times
	double warmup_times[NUM_WARMPUP_ITERS];
	double avg_warmup_time = 0.0;
	double max_warmup_time = 0.0;
	
	for (int i = 0; i < NUM_WARMPUP_ITERS; i++) {
		MPI_Barrier (_worldComm);
		start_time = CMSB::elg_pform_wtime ();
		performMPICollectiveFunc ();
		end_time = CMSB::elg_pform_wtime ();
		warmup_times[i] = (end_time - start_time) * 1e6;	// Convert to usec
	}
	for (int i = 0; i < NUM_WARMPUP_ITERS; i++) avg_warmup_time += warmup_times[i];
	avg_warmup_time /= NUM_WARMPUP_ITERS;
	MPI_Allreduce (&avg_warmup_time, &max_warmup_time, 1, MPI_DOUBLE, MPI_MAX, _worldComm);
	syncInfo->_esttime = max_warmup_time;
	
#ifdef __bgq__
	// On JUQUEEN it's possible to query the protocol that was used in
	// the collective function
	if (_myRank == 0) {
		char collective_alg_name[100];
		MPIX_Get_last_algorithm_name (_worldComm, collective_alg_name, 100);
		std::cout << "COLLECTIVE: " << mpi_collective_name << ", ALGORITHM USED: "
				  << collective_alg_name << std::endl;
	}
#endif

	double run_times[NUM_ITERS_ROUND];
	double max_run_times[NUM_ITERS_TOTAL];
	double errors[NUM_ITERS_ROUND];
	double max_errors[NUM_ITERS_ROUND];
	int total_num_valid_runs = 0;
		
	do {
		if (_myRank == 0) {
			std::cout << mpi_collective_name << ": starting new round; valid runs = "
					  << total_num_valid_runs
					  << ", window = " << std::setprecision(6) << std::fixed << syncInfo->_window
					  << ", est. time = " << std::setprecision(6) << std::fixed << syncInfo->_esttime
					  << std::endl;
		}
		int error_count = 0;
		CMSB::sync_init_stage2 (syncInfo);
		for (int i = 0; i < NUM_ITERS_ROUND; i++) {
			double err = 0;
   
			errors[i] = CMSB::nbcb_sync (syncInfo);
        
			start_time = CMSB::elg_pform_wtime ();
			performMPICollectiveFunc ();
			end_time = CMSB::elg_pform_wtime ();
		
			run_times[i] = (end_time - start_time) * 1e6;	// Convert to usec
		}
		
		// Check for errors in the synchronization process
		MPI_Allreduce (errors, max_errors, NUM_ITERS_ROUND, MPI_DOUBLE, MPI_MAX, _worldComm);
		for (int i = 0; i < NUM_ITERS_ROUND; i++) {
			if (max_errors[i] > 0.0) {
				error_count++;
				run_times[i] = run_times[NUM_ITERS_ROUND-error_count];
			}
		}
		// If more than 25% erros occured or there are less than 4 valid
		// measurements, we should double the window size and re-run the
		// round
		int valid_runs_count = NUM_ITERS_ROUND-error_count;
		if (error_count > NUM_ITERS_ROUND*0.25 || valid_runs_count < 4) {
			syncInfo->_window *= 2.0;
			continue;
		}
		
		// Use only the amount of runs needed to complete the NUM_ITERS_TOTAL
		// requirement
		if (total_num_valid_runs + valid_runs_count > NUM_ITERS_TOTAL) {
			valid_runs_count = NUM_ITERS_TOTAL - total_num_valid_runs;
		}
		MPI_Reduce (run_times, max_run_times+total_num_valid_runs, valid_runs_count, MPI_DOUBLE, MPI_MAX, 0, _worldComm);
		total_num_valid_runs += valid_runs_count;
		
		// Iterate until sufficient statistical confidence is reached
	} while (total_num_valid_runs < NUM_ITERS_TOTAL);
	
	// Calculate average
	if (_myRank == 0) {
		_avgRunTime = 0.0;
		for (int i = 0; i < NUM_ITERS_TOTAL; i++) _avgRunTime += max_run_times[i];
		_avgRunTime /= NUM_ITERS_TOTAL;
		std::cout << mpi_collective_name << ": total runs = " << NUM_ITERS_TOTAL << std::endl;
		std::cout << mpi_collective_name << ": average runtime = " << std::setprecision(6)
				  << std::fixed << _avgRunTime << std::endl;
		//////////////
		double median = 0.0;
		std::sort (max_run_times, max_run_times + NUM_ITERS_TOTAL);
		if (NUM_ITERS_TOTAL % 2 > 0) {
			median = max_run_times[int (floor (NUM_ITERS_TOTAL/2.0))];
		}
		else {
			double val_hi = max_run_times[int (NUM_ITERS_TOTAL/2.0)];
			double val_lo = max_run_times[int (NUM_ITERS_TOTAL/2.0) - 1];
			median = (val_hi + val_lo) / 2;
		}
		_avgRunTime = median;
		std::cout << mpi_collective_name << ": median = " << std::setprecision(6)
				  << std::fixed << median << std::endl;
		//////////////
		double sum = 0.0, sum_of_sqrs = 0.0;
		for (int i = 0; i < NUM_ITERS_TOTAL; i++) {
			sum += max_run_times[i];
			sum_of_sqrs += (max_run_times[i]*max_run_times[i]);
		}
		std::cout << mpi_collective_name << ": sum = " << std::setprecision(6)
				  << std::fixed << sum << std::endl;
		std::cout << mpi_collective_name << ": sum of squares = " << std::setprecision(6)
				  << std::fixed << sum_of_sqrs << std::endl;
		for (int i = 0; i < NUM_ITERS_TOTAL; i++) {
			std::cout << mpi_collective_name << ": v: " << std::setprecision(6)
					  << std::fixed << max_run_times[i] << std::endl;
		}
		//////////////
	}

}

void CMSB::createCollectiveMicroBenches (std::vector<MicroBench*>& benchmarks, unsigned int messageSizePerProc) {
	
	benchmarks.push_back (new CMSB::BcastBench(messageSizePerProc));
	benchmarks.push_back (new CMSB::GatherBench(messageSizePerProc));
	benchmarks.push_back (new CMSB::AllgatherBench(messageSizePerProc));
	// Latency-oriented benchmarks
//cscs	benchmarks.push_back (new CMSB::AlltoallBench	  (messageSizePerProc));
//cscs	benchmarks.push_back (new CMSB::AllgatherBench	  (messageSizePerProc));
//cscs	benchmarks.push_back (new CMSB::AllreduceBench	  (messageSizePerProc));
//cscs	benchmarks.push_back (new CMSB::GatherBench		  (messageSizePerProc));
//cscs	benchmarks.push_back (new CMSB::ReduceBench		  (messageSizePerProc));
//cscs	benchmarks.push_back (new CMSB::BcastBench		  (messageSizePerProc));
//cscs	benchmarks.push_back (new CMSB::ScatterBench	  (messageSizePerProc));
//cscs	benchmarks.push_back (new CMSB::ScanBench		  (messageSizePerProc));
//cscs	benchmarks.push_back (new CMSB::ReduceScatterBench(messageSizePerProc));
//cscs	benchmarks.push_back (new CMSB::AlltoallvBench	  (messageSizePerProc));
//cscs	benchmarks.push_back (new CMSB::AllgathervBench	  (messageSizePerProc));
//cscs	benchmarks.push_back (new CMSB::GathervBench	  (messageSizePerProc));
//cscs	benchmarks.push_back (new CMSB::ScattervBench	  (messageSizePerProc));
//cscs	benchmarks.push_back (new CMSB::BarrierBench   	  ());
}

void CMSB::createCollectiveMicroBenchesMinimalVer (std::vector<MicroBench*>& benchmarks, unsigned int messageSizePerProc) {
	
	//cscs benchmarks.push_back (new CMSB::BarrierBench   	  ());

	benchmarks.push_back (new CMSB::BcastBench		  (messageSizePerProc));
	//cscs benchmarks.push_back (new CMSB::ReduceBench		  (messageSizePerProc));
	//cscs benchmarks.push_back (new CMSB::AllreduceBench	  (messageSizePerProc));
	//cscs benchmarks.push_back (new CMSB::GatherBench		  (messageSizePerProc));

	benchmarks.push_back (new CMSB::AllgatherBench	  (messageSizePerProc));
	//cscs benchmarks.push_back (new CMSB::AlltoallBench	  (messageSizePerProc));
	//cscs benchmarks.push_back (new CMSB::BcastAltBench	  (messageSizePerProc));
	//benchmarks.push_back (new CMSB::ScatterBench	  (messageSizePerProc));
	//benchmarks.push_back (new CMSB::ReduceAltBench	  (messageSizePerProc));
	//benchmarks.push_back (new CMSB::GatherAltBench	  (messageSizePerProc));
}

void CMSB::createCollectiveExtraSizeMicroBenches (std::vector<MicroBench*>& benchmarks, unsigned int messageSizePerProc) {
	
	// Latency-oriented benchmarks
	benchmarks.push_back (new CMSB::AllreduceBench	  (messageSizePerProc));
	benchmarks.push_back (new CMSB::ReduceBench		  (messageSizePerProc));
	benchmarks.push_back (new CMSB::BcastBench		  (messageSizePerProc));
	benchmarks.push_back (new CMSB::ScanBench		  (messageSizePerProc));
}
