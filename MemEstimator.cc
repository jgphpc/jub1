#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include "MemEstimator.h"
#include "MicroBench.h"
#include "util/fast_hash.h"
#include "util/epk_hashtab.h"
#include <malloc.h>

#ifdef __bgq__
#include <spi/include/kernel/memory.h>
#endif


#define ALLOCS_HASH_TAB_SIZE    4096


static size_t allocs_hash_func (const void* key) {
    
    const uint64_t key_int = (const uint64_t)key;
    uint32_t hash_val = fast_hash ((const char *)&key_int, sizeof (uint64_t));
    return (size_t)hash_val;
}


static int allocs_cmp_func (const void* key , const void* item_key) {
    
    return (key != item_key);
}


/* Prototypes for our hooks.  */
static void my_init_hook (void);
static void *my_malloc_hook (size_t, const void *);
static void my_free_hook (void*, const void *);


/* Override initializing hook from the C library. */
void (*__malloc_initialize_hook) (void) = my_init_hook;


static void* (*old_malloc_hook)(size_t, const void *);
static void (*old_free_hook)(void*, const void*);


static uint64_t peak_mem_alloc = 0;
static uint64_t curr_mem_alloc = 0;
static uint64_t local_peak_mem_alloc = 0;
static uint64_t local_curr_mem_alloc = 0;
static EpkHashTab* alloc_hash_tab = NULL;


static void my_init_hook (void) {
    
    alloc_hash_tab = epk_hashtab_create_size (ALLOCS_HASH_TAB_SIZE, &allocs_hash_func, &allocs_cmp_func);
    
    old_malloc_hook = __malloc_hook;
    old_free_hook = __free_hook;
    __malloc_hook = my_malloc_hook;
    __free_hook = my_free_hook;
}


static void * my_malloc_hook (size_t size, const void *caller) {
    
    void *result;
    /* Restore all old hooks */
    __malloc_hook = old_malloc_hook;
    __free_hook = old_free_hook;
    /* Call recursively */
    result = malloc (size);
    
    epk_hashtab_insert (alloc_hash_tab, result, (void*)size, NULL);
    
    curr_mem_alloc += size;
    if (curr_mem_alloc > peak_mem_alloc)
        peak_mem_alloc = curr_mem_alloc;
        
    local_curr_mem_alloc += size;
    if (local_curr_mem_alloc > local_peak_mem_alloc)
        local_peak_mem_alloc = local_curr_mem_alloc;
    
    //std::cout << "malloc " << size << " returns " << result << std::endl;
    
    /* Save underlying hooks */
    old_malloc_hook = __malloc_hook;
    old_free_hook = __free_hook;
    
    /* Restore our own hooks */
    __malloc_hook = my_malloc_hook;
    __free_hook = my_free_hook;
    return result;
}


static void my_free_hook (void *ptr, const void *caller) {
    
    /* Restore all old hooks */
    __malloc_hook = old_malloc_hook;
    __free_hook = old_free_hook;
    
    /* Call recursively */
    free (ptr);
    
    if (ptr != NULL) {
		EpkHashEntry* hash_entry = epk_hashtab_find (alloc_hash_tab, ptr, NULL);
		if (hash_entry != NULL) {
			curr_mem_alloc -= (size_t)hash_entry->value;
			local_curr_mem_alloc -= (size_t)hash_entry->value;
			epk_hashtab_remove (alloc_hash_tab, ptr, NULL);
			//std::cout << "freed pointer " << ptr << std::endl;
		}
	}
    
    /* Save underlying hooks */
    old_malloc_hook = __malloc_hook;
    old_free_hook = __free_hook;
   
    /* Restore our own hooks */
    __malloc_hook = my_malloc_hook;
    __free_hook = my_free_hook;
}


uint64_t CMSB::MemEstimator::getPeakMemConsumption () {
	
	return peak_mem_alloc;
}


uint64_t CMSB::MemEstimator::getCurrentMemConsumption () {

    return curr_mem_alloc;
}


void CMSB::MemEstimator::startLocalPeakMemMeasurement () {

	local_peak_mem_alloc = 0;
	local_curr_mem_alloc = 0;
}


uint64_t CMSB::MemEstimator::getLocalPeakMemConsumption () {
	
	return local_peak_mem_alloc;
}


uint64_t CMSB::MemEstimator::getBenchesMemConsumption (const std::vector<CMSB::MicroBench*>& benchmarks) {
    
    uint64_t benches_mem_consumption = 0;
    unsigned int num_benchmarks = benchmarks.size ();
    for (int i = 0; i < num_benchmarks; i++) {
        benches_mem_consumption += benchmarks[i]->getMemConsumption ();
    }
    benches_mem_consumption += sizeof(CMSB::MicroBench*) * benchmarks.size ();
    benches_mem_consumption += sizeof(std::vector<CMSB::MicroBench*>);
    return benches_mem_consumption;
}

uint64_t CMSB::MemEstimator::getProcMemConsumption () {
	
#ifdef __bgq__
    {
        uint64_t stack_mem, heap_mem;
        Kernel_GetMemorySize (KERNEL_MEMSIZE_STACK, &stack_mem);
        Kernel_GetMemorySize (KERNEL_MEMSIZE_HEAP, &heap_mem);
        return (stack_mem + heap_mem);
    }
#else
    {
        uint64_t shared_mem = 0, private_mem = 0, pss_mem = 0;
        bool pss_exists = false;
        std::ifstream smaps_file ("/proc/self/smaps");

        if (smaps_file.fail ()) {
            return 0;
        }

        char buff[512];
        const char* shared_line_str = "Shared";
        const char* pss_line_str = "Pss";
        const char* private_line_str = "Private";
                
        while (!smaps_file.eof ()) {
            smaps_file.getline (buff, 512);
            if (strstr (buff, shared_line_str) != NULL) {
                strtok (buff, " \t"); // Starting of the line;
                shared_mem += atoi (strtok (NULL, " \t")); // The number itself
            }
            else if (strstr (buff, private_line_str) != NULL) {
                strtok (buff, " \t"); // Starting of the line;
                private_mem += atoi (strtok (NULL, " \t")); // The number itself            }
            }
            else if (strstr (buff, pss_line_str) != NULL) {
                pss_exists = true;
                strtok (buff, " \t"); // Starting of the line;
                pss_mem += atoi (strtok (NULL, " \t")); // The number itself            }
            }
        }
    
        smaps_file.close ();
        
        if (pss_exists) {
            shared_mem = pss_mem - private_mem;
        }

        return (shared_mem + private_mem) * 1024;   // Result needs to be in bytes
    }
#endif
}

void CMSB::MemEstimator::printSmapsFile () {

	std::ifstream smaps_file ("/proc/self/smaps");
	std::ofstream smaps_out ("smaps_out");
	char buff[512];
	
	if (smaps_file.fail () || smaps_out.fail ()) {
		return;
	}	
	
	while (!smaps_file.eof ()) {
		smaps_file.getline (buff, 512);
		smaps_out << buff << std::endl;
	}
	
	smaps_file.close ();
	smaps_out.close ();
}
