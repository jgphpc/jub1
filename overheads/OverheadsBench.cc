
#include <algorithm>
#include <cmath>
#include <string>
#include <iostream>
#include <ios>
#include <iomanip>
#include <timing/elg_pform_defs.h>
#include "OverheadsBench.h"
#include <MemEstimator.h>
#include "CartcreateBench.h"
#include "CommcreateBench.h"
#include "CommdupBench.h"
#include "WincreateBench.h"


CMSB::OverheadsBench::OverheadsBench () 
    : _worldGrp (MPI_GROUP_NULL), _overheadSize (0), _myRank (0), _numProcs (0) {}


CMSB::OverheadsBench::~OverheadsBench () {
    
    if (_worldGrp != MPI_GROUP_NULL)
        MPI_Group_free (&_worldGrp);
}


void CMSB::OverheadsBench::init (MPI_Comm worldComm, CMSB::MicroBench::MicroBenchInfo* benchInfo) {
    
    CMSB::MicroBench::init (worldComm, benchInfo);
    
    MPI_Comm_rank (_worldComm, &_myRank);
    MPI_Comm_size (_worldComm, &_numProcs);
    MPI_Comm_group (_worldComm, &_worldGrp);
}


void CMSB::OverheadsBench::runMicroBench (CMSB::TimeSyncInfo* syncInfo) {
    
    double start_time, end_time;
    double mem_before, mem_after;
	std::string mpi_collective_name (getMicroBenchName ());

	// First run the warmpup runs - no need to measure times
	double warmup_times[NUM_WARMPUP_ITERS];
	double avg_warmup_time = 0.0;
	double max_warmup_time = 0.0;
    unsigned int num_internal_iters = 0;
	
	for (int i = 0; i < NUM_WARMPUP_ITERS; i++) {
		MPI_Barrier (_worldComm);
		start_time = CMSB::elg_pform_wtime ();
		num_internal_iters = runOverheadFunc ();
		end_time = CMSB::elg_pform_wtime ();
		warmup_times[i] = (end_time - start_time) * 1e6;	// Convert to usec
        warmup_times[i] /= num_internal_iters;
        cleanupOverheadFunc ();
	}
	for (int i = 0; i < NUM_WARMPUP_ITERS; i++) avg_warmup_time += warmup_times[i];
	avg_warmup_time /= NUM_WARMPUP_ITERS;
	MPI_Allreduce (&avg_warmup_time, &max_warmup_time, 1, MPI_DOUBLE, MPI_MAX, _worldComm);
    syncInfo->_esttime = max_warmup_time;
         
    double run_times[NUM_ITERS_ROUND];
    double mem_consump[NUM_ITERS_ROUND];
	double max_run_times[NUM_ITERS_TOTAL];
    double max_mem_consump[NUM_ITERS_TOTAL];
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
        
            mem_before = 0.0;
            CMSB::MemEstimator::startLocalPeakMemMeasurement ();
			start_time = CMSB::elg_pform_wtime ();
			num_internal_iters = runOverheadFunc ();
			end_time = CMSB::elg_pform_wtime ();
            mem_after = (double)CMSB::MemEstimator::getLocalPeakMemConsumption ();
            cleanupOverheadFunc ();
            
			run_times[i] = (end_time - start_time) * 1e6;	// Convert to usec
            mem_consump[i] = (mem_before > mem_after) ? 0.0 : (mem_after - mem_before);
            run_times[i] /= num_internal_iters;
            mem_consump[i] /= num_internal_iters;
            //mem_consump[i] = (mem_before > mem_after) ? (mem_before - mem_after) : (mem_after - mem_before);
		}
		
		// Check for errors in the synchronization process
		MPI_Allreduce (errors, max_errors, NUM_ITERS_ROUND, MPI_DOUBLE, MPI_MAX, _worldComm);
        int valid_runtime_back_idx = NUM_ITERS_ROUND - 1;
        while (valid_runtime_back_idx >= 0 && max_errors[valid_runtime_back_idx] > 0.0)
            valid_runtime_back_idx--;
        if (valid_runtime_back_idx < 0)
            error_count = NUM_ITERS_ROUND;
        else {
            for (int i = 0; i < NUM_ITERS_ROUND; i++) {
                if (max_errors[i] > 0.0) {
                    error_count++;
                    if (valid_runtime_back_idx > i) {
                        run_times[i] = run_times[valid_runtime_back_idx];
                        mem_consump[i] = mem_consump[valid_runtime_back_idx];
                        valid_runtime_back_idx--;
                        while (valid_runtime_back_idx >= 0 && max_errors[valid_runtime_back_idx] > 0.0)
                            valid_runtime_back_idx--;
                    }
                }
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
        MPI_Reduce (mem_consump, max_mem_consump+total_num_valid_runs, valid_runs_count, MPI_DOUBLE, MPI_MAX, 0, _worldComm);
		total_num_valid_runs += valid_runs_count;
		
		// Iterate until sufficient statistical confidence is reached
	} while (total_num_valid_runs < NUM_ITERS_TOTAL);
	
	// Calculate average
	if (_myRank == 0) {
        double avg_mem_consump = 0;
		double avg_run_time = 0.0;
		for (int i = 0; i < NUM_ITERS_TOTAL; i++) avg_run_time += max_run_times[i];
        for (int i = 0; i < NUM_ITERS_TOTAL; i++) avg_mem_consump += max_mem_consump[i];
		avg_run_time /= NUM_ITERS_TOTAL;
        avg_mem_consump /= NUM_ITERS_TOTAL;
		std::cout << mpi_collective_name << ": total runs = " << NUM_ITERS_TOTAL << std::endl;
		std::cout << mpi_collective_name << ": average runtime = " << std::setprecision(6)
				  << std::fixed << avg_run_time << std::endl;
        std::cout << mpi_collective_name << ": average mem consump = " << std::setprecision(6)
				  << std::fixed << avg_mem_consump << std::endl;
		//////////////
		double median_run_time = 0.0;
        double median_mem_consump = 0;
		std::sort (max_run_times, max_run_times + NUM_ITERS_TOTAL);
        std::sort (max_mem_consump, max_mem_consump + NUM_ITERS_TOTAL);
		if (NUM_ITERS_TOTAL % 2 > 0) {
			median_run_time = max_run_times[int (floor (NUM_ITERS_TOTAL/2.0))];
            median_mem_consump = max_mem_consump[int (floor (NUM_ITERS_TOTAL/2.0))];
		}
		else {
			double val_hi_time = max_run_times[int (NUM_ITERS_TOTAL/2.0)];
			double val_lo_time = max_run_times[int (NUM_ITERS_TOTAL/2.0) - 1];
            uint64_t val_hi_mem = max_mem_consump[int (NUM_ITERS_TOTAL/2.0)];
			uint64_t val_lo_mem = max_mem_consump[int (NUM_ITERS_TOTAL/2.0) - 1];
			median_run_time = (val_hi_time + val_lo_time) / 2;
            median_mem_consump = (val_hi_mem + val_lo_mem) / 2;
		}
		std::cout << mpi_collective_name << ": median runtime = " << std::setprecision(6)
				  << std::fixed << median_run_time << std::endl;
        std::cout << mpi_collective_name << ": median mem consump = " << std::setprecision(6)
				  << std::fixed << median_mem_consump << std::endl;
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
            std::cout << mpi_collective_name << ": memv: " << std::setprecision(6)
					  << std::fixed << max_mem_consump[i] << std::endl;
		}
		//////////////
        
        _overheadSize = avg_mem_consump;
	}
}


void CMSB::createOverheadsMicroBenches (std::vector<MicroBench*>& benchmarks) {
	
	benchmarks.push_back (new CMSB::CommcreateBench ());
	benchmarks.push_back (new CMSB::CommdupBench    ());
	benchmarks.push_back (new CMSB::WincreateBench  ());
    benchmarks.push_back (new CMSB::CartcreateBench ());
}
