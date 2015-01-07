/****************************************************************************
**  EPK_MAP - An STL like map implemenation for C                          **
*****************************************************************************
**  Copyright (c) 2011                                                     **
**  German Research School for Simulation Sciences GmbH,                   **
**  Laboratory for Parallel Programming                                    **
**                                                                         **
**  See the file COPYRIGHT in the package base directory for details       **
****************************************************************************/
#ifndef EPK_MAP_H
#define EPK_MAP_H

#include <stddef.h>

/**
 * @addtogroup EPIK_map
 * @{
 */

/**
 * @file   epk_map.h
 * @author Marc-Andre Hermanns <m.a.hermanns@grs-sim.de>
 *
 * @brief  Interface declarartion for @c epk_map, an associative array
 *         implementation for C.
 *
 * The library implements an associative array using a red-black tree
 * as the driving data-structure. Key and data values are referenced
 * by pointers to allow for arbitrary keys and values without changing
 * the implementation.
 *
 * The map can be used in two modi: (1) keeping only pointers to keys
 * and values, leaving all of the actual memory management to the user
 * and (2) copying the pointed to keys and values, taking over the
 * responsibility for their proper management.
 *
 * The first mode of operation can be achieved by supplying \a NULL
 * pointer as the respective constructor function. This mode can be
 * useful to create an efficient search structure without replicating
 * the data.
 *
 * The second mode of operation can be achieved by supplying a proper
 * 'copy-constructor' function for the key and/or value respectively.
 */

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif

/** Opaque data structure representing a dynamic assosiative array */
typedef struct EpkMap_struct EpkMap;

/** Opaque data structure representing an interator */
typedef struct EpkMapIter_struct EpkMapIter;

/**
 * Pointer-to-function type describing the comparison function
 * used to build the array.
 * @param  lhs Left-hand side of the comparison.
 * @param  rhs Right-hand side of the comparison.
 * @return Returns -1 if lhs < rhs, 1 if lhs > rhs, and 0 if lhs = rhs.
 */
typedef int (*epk_compare_f)(const void* lhs, const void* rhs);

/**
 * Pointer-to-function type describing a foreach callback function
 * to be called for each element
 */
typedef void (*epk_callback_f)(EpkMapIter* iter, void* cbdata);

/**
 * Pointer-to-function type describing a copy constructor for
 * key or value types used in the maps
 */
typedef void* (*epk_copy_ctor_f)(void* data);

/*- Construction & desctruction ------------------------------------*/

EXTERN EpkMap* epk_map_create(epk_compare_f   cmp,
                              epk_copy_ctor_f key_ctor,
                              epk_copy_ctor_f val_ctor);
EXTERN void epk_map_free(EpkMap* map);

/*- Capacity -------------------------------------------------------*/

EXTERN EpkMapIter* epk_map_begin(EpkMap* map);
EXTERN EpkMapIter* epk_map_end(EpkMap* map);
EXTERN int         epk_map_iter_is_end(EpkMapIter* iter);
EXTERN EpkMapIter* epk_map_rbegin(EpkMap* map);
EXTERN EpkMapIter* epk_map_rend(EpkMap* map);
EXTERN void        epk_map_iter_free(EpkMapIter* iter);
EXTERN void*       epk_map_iter_first(EpkMapIter* iter);
EXTERN void*       epk_map_iter_second(EpkMapIter* iter);
EXTERN void        epk_map_iter_next(EpkMapIter* iter);
EXTERN void        epk_map_iter_prev(EpkMapIter* iter);

/*- Capacity -------------------------------------------------------*/

EXTERN int    epk_map_empty(EpkMap* map);
EXTERN size_t epk_map_size(EpkMap* map);
EXTERN size_t epk_map_max_size(EpkMap* map);

/*- Modifiers ------------------------------------------------------*/

EXTERN void epk_map_insert(EpkMap* map,
                           void*   key,
                           void*   value);
EXTERN void epk_map_insert_at(EpkMap*     map,
                              EpkMapIter* iter,
                              void*       key,
                              void*       value);
EXTERN void epk_map_erase(EpkMap* map,
                          void*   key);
EXTERN void epk_map_erase_at(EpkMap*     map,
                             EpkMapIter* iter);
EXTERN void epk_map_clear(EpkMap* map);

/*- Algorithms -----------------------------------------------------*/

EXTERN EpkMapIter* epk_map_find(EpkMap* map,
                                void*   key);
EXTERN EpkMapIter* epk_map_lower_bound(EpkMap* map,
                                       void*   key);
EXTERN EpkMapIter* epk_map_upper_bound(EpkMap* map,
                                       void*   key);
EXTERN void epk_map_foreach(EpkMap*        map,
                            epk_callback_f callback,
                            void*          data);

/**
 * @}
 */
#endif
