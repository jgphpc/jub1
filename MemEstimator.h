#ifndef __MEM_ESTIMATOR_H__
#define __MEM_ESTIMATOR_H__

#include <stdint.h>
#include <vector>



namespace CMSB {

    class MicroBench;
    
    /**
     * Estimates the memory consumption.
     */
    class MemEstimator {
        
    public:
    
        /**
         * Returns current allocated memory estimate in bytes.
         */
        static uint64_t getCurrentMemConsumption ();
        
        /**
         * Returns the peak allocated memory estimate in bytes.
         */
        static uint64_t getPeakMemConsumption ();
        
        static void startLocalPeakMemMeasurement ();
        
        static uint64_t getLocalPeakMemConsumption ();
        
        static uint64_t getProcMemConsumption ();
        /**
         * Returns the total memory consumption of the given benchmarks.
         */
        static uint64_t getBenchesMemConsumption (const std::vector<CMSB::MicroBench*>& benchmarks);
        
        static void printSmapsFile ();
    };
    
}





#endif
