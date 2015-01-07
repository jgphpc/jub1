#include <mpi.h>
#include <stdint.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include "MicroBench.h"
#include "MemEstimator.h"
#include "timing/ClockSync.h"
#include "timing/elg_pform_defs.h"
#include "collectives/CollectivesBench.h"
#include "collectives/CommMemBench.h"
#include "overheads/OverheadsBench.h"


#ifdef USE_SCOREP

#include <scorep/SCOREP_User.h>

SCOREP_USER_METRIC_LOCAL (bench_memory_metric);

#endif

	

int main (int argc, char** argv) {
	
    // Get the command arguments/
    // The command argument is the message size per process
    bool is_extra_msg_size_bench = false;
    unsigned int message_size_per_proc = 0;
    bool duplicate_world_comm = false;
    if (argc < 2) {
        return -1;
    }
    //is_extra_msg_size_bench = (std::atoi(argv[1]) == 1);
    message_size_per_proc = std::atoi(argv[1]);
    //duplicate_world_comm = (std::atoi(argv[3]) == 1);
    
	// General init steps
	CMSB::elg_pform_init ();
    
    CMSB::TimeSyncInfo timeSyncInfo;
	
	// Measure initial memory consumption
	uint64_t initial_proc_mem = CMSB::MemEstimator::getProcMemConsumption ();
    
    MPI_Init (&argc, &argv);

#ifdef USE_SCOREP    
    SCOREP_USER_METRIC_INIT (bench_memory_metric, "Memory", "bytes",
                             SCOREP_USER_METRIC_TYPE_DOUBLE, SCOREP_USER_METRIC_CONTEXT_GLOBAL);
#endif
    
    int my_rank, num_procs;

    MPI_Comm_rank (MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size (MPI_COMM_WORLD, &num_procs);
    
	// Create benchmarks
	std::vector<CMSB::MicroBench*> benchmarks;
    if (is_extra_msg_size_bench) {
        //CMSB::createCollectiveExtraSizeMicroBenches (benchmarks, message_size_per_proc);
    }
    else {
        //CMSB::createCollectiveMicroBenches (benchmarks, message_size_per_proc);
        CMSB::createCollectiveMicroBenchesMinimalVer (benchmarks, message_size_per_proc);
        //benchmarks.push_back (new CMSB::CommMemBench ());
    }
    CMSB::createOverheadsMicroBenches (benchmarks);

    CMSB::sync_init_stage1 (&timeSyncInfo);
            
    if (my_rank == 0) {
        std::cout << "Running benchmarks..." << std::endl;
        std::cout << "Extra message size bench: " << is_extra_msg_size_bench << std::endl;
        std::cout << "Max buffer size in: " << MAX_BUFF_SIZE_PER_PROC << " MB" << std::endl;
        std::cout << "Message size per process in doubles: " << message_size_per_proc << std::endl;
        std::cout << "Non comm-world communicator: " << duplicate_world_comm << std::endl;
        std::cout << "Running on " << num_procs << " ranks" << std::endl; 
        std::cout << "Memory consumption before allocating buffers " 
				  << CMSB::MemEstimator::getCurrentMemConsumption () << std::endl;
    }

	unsigned int num_benchmarks = benchmarks.size ();
	
	CMSB::MicroBench::MicroBenchInfo benchInfo;
	unsigned int buff_size = (MAX_BUFF_SIZE_PER_PROC * 1024 * 1024) / sizeof(double);
    benchInfo._sBuffLen = benchInfo._rBuffLen = buff_size * sizeof(double);
	benchInfo._sendBuff = new double[buff_size];
	benchInfo._recvBuff = new double[buff_size];
	benchInfo._sendCounts = new int[num_procs];
	benchInfo._sendDispls = new int[num_procs];
	benchInfo._recvCounts = new int[num_procs];
	benchInfo._recvDispls = new int[num_procs];
    
    if (my_rank == 0) {
		std::cout << "Memory consumption after allocating buffers " 
				  << CMSB::MemEstimator::getCurrentMemConsumption () << std::endl;
	}
	
    // Init buffers
    std::fill_n (benchInfo._sendBuff, buff_size, my_rank+1);	// +1 so that rank's zero buff contains ones instead of zeros
    std::fill_n (benchInfo._recvBuff, buff_size, 0.0);
    std::fill_n (benchInfo._sendCounts, num_procs, 0);
    std::fill_n (benchInfo._sendDispls, num_procs, 0);
    std::fill_n (benchInfo._recvCounts, num_procs, 0);
    std::fill_n (benchInfo._recvDispls, num_procs, 0);
    
    // Duplicate comm-world communicator
    MPI_Comm dup_world_comm = MPI_COMM_WORLD;
    if (duplicate_world_comm) {
        MPI_Group comm_world_grp;
        MPI_Comm_group (MPI_COMM_WORLD, &comm_world_grp);
        MPI_Comm_create (MPI_COMM_WORLD, comm_world_grp, &dup_world_comm);
        MPI_Group_free (&comm_world_grp);
    }
    
   	// Init & run benchmarks
   	for (int i = 0; i < num_benchmarks; i++) {
		if (my_rank == 0) {
			std::cout << "Starting benchmark: " << benchmarks[i]->getMicroBenchName () << std::endl;
		}
		// Re-init buffers
		std::fill_n (benchInfo._sendBuff, buff_size, my_rank+1);	// +1 so that rank's zero buff contains ones instead of zeros
		std::fill_n (benchInfo._recvBuff, buff_size, 0.0);
		benchmarks[i]->init (dup_world_comm, &benchInfo);
		benchmarks[i]->runMicroBench (&timeSyncInfo);
		benchmarks[i]->writeResultToProfile ();
		if (my_rank == 0) {
			std::cout << "Benchmark: " << benchmarks[i]->getMicroBenchName () << " finished." << std::endl;
		}
   	}   
	
	// Calculate MPI memory consumption
	uint64_t proc_mem = CMSB::MemEstimator::getProcMemConsumption () - initial_proc_mem;
	uint64_t mpi_mem = CMSB::MemEstimator::getPeakMemConsumption ();
	uint64_t overheads = CMSB::MemEstimator::getBenchesMemConsumption (benchmarks);
	overheads += sizeof(double)*2*buff_size + sizeof(int)*4*num_procs;
	mpi_mem -= overheads;
	proc_mem -= overheads;
    
    if (my_rank == 0) {		// Only one rank should update SCORE-P's metrics
        double mem_consump = double (mpi_mem) / (1024*1024);    // Convert to MB
        double proc_mem_consump = double (proc_mem) / (1024*1024);    // Convert to MB
#ifdef USE_SCOREP
        SCOREP_USER_METRIC_DOUBLE (bench_memory_metric, mem_consump);
#endif
        std::cout << "Finished all benchmarks" << std::endl;
        std::cout << "Peak memory consumption (MB): " << mem_consump << std::endl;
        std::cout << "Proc (old approach) memory consumption (MB): " << proc_mem_consump << std::endl;
        
        CMSB::MemEstimator::printSmapsFile ();
    }
	
    if (duplicate_world_comm) {
        MPI_Comm_free (&dup_world_comm);
    }
    
	for (int i = 0; i < num_benchmarks; i++) {
	    delete benchmarks[i];
    }
	
	delete[] benchInfo._sendBuff;
	delete[] benchInfo._recvBuff;
	delete[] benchInfo._sendCounts;
	delete[] benchInfo._sendDispls;
	delete[] benchInfo._recvCounts;
	delete[] benchInfo._recvDispls; 
	
    MPI_Finalize ();

    return 0;
}
