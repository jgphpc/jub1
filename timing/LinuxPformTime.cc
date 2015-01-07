/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 1998-2013                                                **
**  Forschungszentrum Juelich GmbH, Juelich Supercomputing Centre          **
**                                                                         **
**  Copyright (c) 2003-2008                                                **
**  University of Tennessee, Innovative Computing Laboratory               **
**                                                                         **
**  See the file COPYRIGHT in the package base directory for details       **
****************************************************************************/

#ifndef __bgq__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#ifdef __ia64__
 #include <asm/intrinsics.h>
#endif
#ifdef _CRAYC
 #include <intrinsics.h>
#endif


#include "elg_pform_defs.h"


extern "C" unsigned long long getrdtsc_impl ();


#ifndef ELG_PROCDIR 
#  define ELG_PROCDIR "/proc/"
#endif
#ifndef ELG_CPUFREQ
#  define ELG_CPUFREQ "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq"
#endif

static uint64_t       elg_cycles_per_sec=1;
static unsigned char elg_cpu_has_tsc=0;
static unsigned int  elg_cpu_count=0;


static uint64_t elg_pform_cpuinfo()
{
  FILE   *cpuinfofp;
  char   line[1024];
  char   *token;
  
  /* size corresponding to elg_cycles_per_sec */
  uint64_t hz = 0;
  
  /* check special file */
  if ((cpuinfofp = fopen (ELG_CPUFREQ, "r")) && fgets(line, sizeof(line), cpuinfofp)) {
    fclose(cpuinfofp);
    hz = 1000*atoll(line);
    return hz;
  }
  
  /* the goal is to run it on Juropa and /proc/cpuinfo exists there */
  cpuinfofp = fopen (ELG_PROCDIR "cpuinfo", "r");
  
  while (fgets(line, sizeof (line), cpuinfofp))
  {
    if (!strncmp("processor", line, 9))
      elg_cpu_count++;
#ifdef __ia64__
    if (!strncmp("itc MHz", line, 7))
#else
    if (!strncmp("cpu MHz", line, 7))
#endif
    {
      strtok(line, ":");
      
      hz = strtol((char*) strtok(NULL, " \n"), (char**) NULL, 0) * 1e6;
    }
    if (!strncmp("timebase", line, 8))
    {
      strtok(line, ":");
      
      hz = strtol((char*) strtok(NULL, " \n"), (char**) NULL, 0);
    }
    if (!strncmp("flags", line, 5))
    { 
      strtok(line, ":");
      while ( (token = (char*) strtok(NULL, " \n"))!=NULL)
      {
        if (strcmp(token,"tsc")==0)
        {
          elg_cpu_has_tsc=1;
        } else if (strcmp(token,"constant_tsc")==0)
        {
          elg_cpu_has_tsc=2;
        }
      }
    }
  }
  
  fclose(cpuinfofp);
  return hz;
}

/* platform specific initialization */
void CMSB::elg_pform_init()
{
  char* env = getenv("ESD_CLOCK_HZ");
  if (env && (atoll(env) > 0)) {
      elg_cycles_per_sec = atoll(env);
#ifdef USE_CLOCK_HZ
  } else if (USE_CLOCK_HZ > 0) {
      elg_cycles_per_sec = USE_CLOCK_HZ;
#else
  } else {
      elg_cycles_per_sec = elg_pform_cpuinfo();
  }
#endif
}


double CMSB::elg_pform_wtime()
{
  /* fast assembler codes to access the cycle counter */
  uint64_t clock_value;

#ifdef __powerpc__
  unsigned int Low, HighB, HighA;

  do {
    asm volatile ("mftbu %0" : "=r"(HighB));
    asm volatile ("mftb %0" : "=r"(Low));
    asm volatile ("mftbu %0" : "=r"(HighA));
  } while (HighB != HighA);
  clock_value = ((unsigned long long)HighA<<32) | ((unsigned long long)Low);
#elif defined(__ia64__)
  /* ... ITC */
  clock_value = __getReg(_IA64_REG_AR_ITC);
#elif defined (_CRAYC)
  clock_value = _rtc();
#elif defined (__x86_64__)
  /* ... TSC */
  {
    uint32_t low = 0;
    uint32_t high = 0;

    asm volatile ("rdtsc" : "=a" (low), "=d" (high));

    clock_value = ((uint64_t)high << 32) + low;
  }
#else
  #warning "Architecture not yet supported in ASM"
#endif
  
  return (double) (clock_value) / (double) elg_cycles_per_sec;
}


int CMSB::elg_pform_is_gclock()
{
  return 0;
}


#endif
