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


#ifdef __bgq__


#include <firmware/include/personality.h>
#include <spi/include/kernel/process.h>
#include <spi/include/kernel/location.h>
#ifdef __GNUC__
#include <hwi/include/bqc/A2_inlines.h>   // for GetTimebase()
#endif
#include <hwi/include/common/uci.h>
#include <builtins.h>

#include "elg_pform_defs.h"

#define BGQ_GROUP_ON_NODEBOARD


static double elg_clockspeed=1.0e-6/DEFAULT_FREQ_MHZ;


static Personality_t mybgq;

/* platform specific initialization */
void CMSB::elg_pform_init() {
  uint64_t netflags;

  Kernel_GetPersonality(&mybgq, sizeof(Personality_t));
  elg_clockspeed = 1.0e-6/(double)(mybgq.Kernel_Config.FreqMHz);
}


/* local or global wall-clock time in seconds */
double CMSB::elg_pform_wtime() {
#if defined(__IBMC__) || defined(__IBMCPP__)
  return ( __mftb() * elg_clockspeed );
#elif defined __GNUC__
  return ( GetTimeBase() * elg_clockspeed );
#else
#error "Platform BGQ: cannot determine timebase"
#endif
}


/* is a global clock provided ? */
int CMSB::elg_pform_is_gclock() {
  return 1;
}


#endif
