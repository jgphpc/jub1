/**********************************
 * Taken from http://www.azillionmonkeys.com/qed/hash.html
 ***********************************/

#ifndef _FAST_HASH_H
#define _FAST_HASH_H


#include <stdint.h>


#ifdef __cplusplus
#  define EXTERN extern "C"
#else
#  define EXTERN extern
#endif


EXTERN uint32_t fast_hash (const char * data, int len);


#endif // _FAST_HASH_H
