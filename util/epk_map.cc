/****************************************************************************
**  EPK_MAP - An STL like map implementation for C                         **
*****************************************************************************
**  Copyright (c) 2010-2011                                                **
**  German Research School for Simulation Sciences GmbH,                   **
**  Laboratory for Parallel Programming                                    **
**                                                                         **
**  See the file COPYRIGHT in the package base directory for details       **
****************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "epk_map.h"

/**
 * @addtogroup EPIK_map
 * @{
 */

/**
 * @file   epk_map.c
 * @author Marc-Andre Hermanns <m.a.hermanns@grs-sim.de>
 *
 * @brief  Implemenation for epk_map, an assiociative array implementation
 *         for C.
 *
 * This implementation is based on the C implemenation found at
 * http://en.literateprograms.org/Red-black_tree_(C), which itself is,
 * according to the author, based on the implementation found on Wikipedia.
 *
 * This implementation makes some changes in terms of memory management
 * of the key/value information, and some minor adjustments to the different
 * public API, which is designed after the C++ STL interface.
 *
 */

/**
 * @brief Macro to enable runtime verification of the tree structure.
 *
 * The verification implies an O(n) performance penalty on any map operation
 * and therefore must NOT be enabled in production use.
 */
#ifdef EPK_MAP_VERIFY
#define VERIFY_PROPERTIES(map) verify_properties(map)
#else
#define VERIFY_PROPERTIES(map)
#endif

/**
 * Node colors
 */
typedef enum
{
    BLACK,  /**< Black node */
    RED     /**< Red node */
} EpkMapNode_color;

/**
 * Iterator direction
 */
typedef enum
{
    FORWARD, /**< Forward iterator */
    REVERSE  /**< Backward iterator */
} EpkMapIter_dir;

/**
 * Internal representation of a map node
 */
typedef struct EpkMapNode_struct EpkMapNode;
struct EpkMapNode_struct
{
    void*            key;    /**< comparision key */
    void*            value;  /**< associated value to key */
    EpkMapNode*      parent; /**< parent within tree structure */
    EpkMapNode*      left;   /**< left child within tree structure */
    EpkMapNode*      right;  /**< right child within tree structure */
    EpkMapNode_color color;  /**< node color */
};


/**
 * Internal representation of a map
 */
struct EpkMap_struct
{
    EpkMapNode*     root;     /**< Pointer to root node of the map */
    epk_compare_f   cmp;      /**< Pointer to comparison function */
    epk_copy_ctor_f key_ctor; /**< Copy constructor for keys */
    epk_copy_ctor_f val_ctor; /**< Copy constructor for values */
    size_t          size;     /**< Number of elements in the map */
};


/**
 * Map iterator
 */
struct EpkMapIter_struct
{
    EpkMap*        map;    /**< map pointer */
    EpkMapNode*    node;   /**< node pointer */
    EpkMapIter_dir dir;    /**< direction */
};


/*
 ***********************************************************************
 * Forward declarations
 ***********************************************************************
 */

/*
 * Node relationships
 */
static EpkMapNode* grandparent(EpkMapNode* node);
static EpkMapNode* sibling(EpkMapNode* node);
static EpkMapNode* uncle(EpkMapNode* node);
static EpkMapNode* max_node(EpkMapNode* node);
static EpkMapNode* min_node(EpkMapNode* node);

/*
 * Node query
 */
static EpkMapNode_color node_color(EpkMapNode* node);

#if EPK_MAP_VERIFY
/*
 * Verification
 */
static void verify_properties(EpkMap* map);
static void verify_color(EpkMapNode* node);
static void verify_black_root(EpkMapNode* node);
static void verify_red_black(EpkMapNode* node);
static void verify_black_count_recurse(EpkMapNode* node,
                                       int         current_count,
                                       int*        full_count);
static void verify_black_count(EpkMapNode* node);

#endif

/*
 * Miscellaneous
 */
static EpkMapNode* create_node(EpkMap*          map,
                               void*            key,
                               void*            value,
                               EpkMapNode_color color,
                               EpkMapNode*      left,
                               EpkMapNode*      right);
static EpkMapIter* create_iter(EpkMap*        map,
                               EpkMapNode*    node,
                               EpkMapIter_dir direction);

/*
 * Map manipulation
 */
static void replace_node(EpkMap*     map,
                         EpkMapNode* old_node,
                         EpkMapNode* new_node);
static void rotate_left(EpkMap*     map,
                        EpkMapNode* node);
static void rotate_right(EpkMap*     map,
                         EpkMapNode* node);
static void adapt_after_insert(EpkMap*     map,
                               EpkMapNode* node);
static void adapt_before_delete(EpkMap*     map,
                                EpkMapNode* node);

/**
 * @name Utility functions
 * @{
 */

/**
 * @internal
 * @brief Return the grandparent of a node
 *
 * This function returns the grandparent node of a given node.  The user is
 * responsible for ensuring that the grandparent is defined, i.e, the node is
 * not the root or one of its direct children.
 *
 * @param node Node the grandparent is searched for
 *
 * @return Grandparent node of given node
 */
static EpkMapNode* grandparent(EpkMapNode* node)
{
    /* assure node is defined */
    assert(node != NULL);
    /* assure node has a parent */
    assert(node->parent != NULL);
    /* assure node has a grandparent */
    assert(node->parent->parent != NULL);

    /* return pointer to grandparent */
    return node->parent->parent;
}


/**
 * @internal
 * @brief Return the sibling of a node
 *
 * This function returns the sibling node of a given node, i.e., the other
 * child of parent. If the parent does not have another define child the return
 * value is \a NULL.
 *
 * @param node Node the sibling is searched for
 *
 * @return Sibling node or NULL
 */
static EpkMapNode* sibling(EpkMapNode* node)
{
    /* assure node is defined */
    assert(node != NULL);
    /* assure node has a parent, as siblings are defined by
       having the same parent */
    assert(node->parent != NULL);

    /* if node is left node return right node of parent,
     * and vice versa. */
    if (node == node->parent->left)
    {
        return node->parent->right;
    }
    else
    {
        return node->parent->left;
    }
}


/**
 * @internal
 * @brief Return the uncle of a node.
 *
 * This function returns the uncle node of a given node, i.e., the sibling of
 * the given node's parent.
 *
 * @param node the uncle is searched for
 *
 * @return Uncle node of NULL
 */
static EpkMapNode* uncle(EpkMapNode* node)
{
    /* assure node is defined */
    assert(node != NULL);
    /* assure node has a parent */
    assert(node->parent != NULL);
    /* assure node has a grandparent */
    assert(node->parent->parent != NULL);

    /* return sibling of parent as uncle node */
    return sibling(node->parent);
}


/**
 * @internal
 * @brief Return node color of a given node
 *
 * @param node Queried node
 * @return Node color
 */
static EpkMapNode_color node_color(EpkMapNode* node)
{
    return (node == NULL) ? BLACK : node->color;
}


#if EPK_MAP_VERIFY
/**
 * @internal
 * @brief Verify tree properties
 * @details This function verifies the full red-black-tree properties
 *        on a given tree. This verification walks the complete tree
 *        and will inflict O(n) runtime penalties on the implementation,
 *        and is therefore disabled by default. It can be explicitly
 *        enabled by the preprocessor define \a EPK_MAP_VERIFY, in
 *        which case this function will not be called.
 */
static void verify_properties(EpkMap* map)
{
    /* Property 1: Every node has either red or black color */
    verify_color(map->root);
    /* Property 2: Root node is black */
    verify_black_root(map->root);
    /* Property 3: All leaves are black -> implicitely given by NULL==BLACK */
    /* Property 4: Every red node has black children and black parent */
    verify_red_black(map->root);
    /* Property 5: All root->leave paths have equal number of black nodes */
    verify_black_count(map->root);
}


/**
 * @internal
 * @brief Recursively verify each node is either @ref RED or @ref BLACK.
 * @param node Root node of queried subtree
 */
static void verify_color(EpkMapNode* node)
{
    assert(node_color(node) == BLACK || node_color(node) == RED);
    if (node == NULL)
    {
        return;
    }
    verify_color(node->left);
    verify_color(node->right);
}


static void verify_black_root(EpkMapNode* node)
{
    assert(node_color(node) == BLACK);
}


static void verify_red_black(EpkMapNode* node)
{
    if (node_color(node) == RED)
    {
        assert(node_color(node->parent) == BLACK);
        assert(node_color(node->left)   == BLACK);
        assert(node_color(node->right)  == BLACK);
    }
    if (node == NULL)
    {
        return;
    }
    verify_red_black(node->left);
    verify_red_black(node->right);
}


static void verify_black_count_recurse(EpkMapNode* node,
                                       int         current_count,
                                       int*        full_count)
{
    /* increase count on black node */
    if (node_color(node) == BLACK)
    {
        current_count++;
    }

    /* check whether we need to recurse */
    if (node == NULL)
    {
        /* is this the first time we are down the complete root->leave path? */
        if (*full_count == -1)
        {
            /* save current count */
            *full_count = current_count;
        }
        else
        {
            /* compare current count and full count */
            assert(*full_count == current_count);
        }
        return;
    }
    /* recurse into children */
    verify_black_count_recurse(node->left, current_count, full_count);
    verify_black_count_recurse(node->right, current_count, full_count);
} /* verify_black_count_recurse */


static void verify_black_count(EpkMapNode* node)
{
    int num_black_nodes = -1;

    verify_black_count_recurse(node, 0, &num_black_nodes);
}


#endif /* ifdef EPK_MAP_VERIFY */

static EpkMapIter* create_iter(EpkMap*        map,
                               EpkMapNode*    node,
                               EpkMapIter_dir direction)
{
    EpkMapIter* new_iter = (EpkMapIter*)malloc(sizeof(EpkMapIter));

    assert(map != NULL);
    assert(new_iter != NULL);

    new_iter->map  = map;
    new_iter->node = node;
    new_iter->dir  = direction;

    return new_iter;
}


static EpkMapNode* create_node(EpkMap*          map,
                               void*            key,
                               void*            value,
                               EpkMapNode_color color,
                               EpkMapNode*      left,
                               EpkMapNode*      right)
{
    EpkMapNode* new_node = NULL;

    /* don't allow NULL keys */
    assert(key   != NULL);
    /* don't allow NULL values */
    assert(value != NULL);
    /* allocate memory */
    new_node = (EpkMapNode*)malloc(sizeof(EpkMapNode));

    if (map->key_ctor != NULL)
    {
        new_node->key = map->key_ctor(key);
    }
    else
    {
        new_node->key = key;
    }

    if (map->val_ctor != NULL)
    {
        new_node->value = map->val_ctor(value);
    }
    else
    {
        new_node->value = value;
    }

    new_node->color  = color;
    new_node->parent = NULL;
    new_node->left   = left;
    if (new_node->left != NULL)
    {
        new_node->left->parent = new_node;
    }
    new_node->right = right;
    if (new_node->right != NULL)
    {
        new_node->right->parent = new_node;
    }

    return new_node;
} /* create_node */


static void free_node(EpkMap* map, EpkMapNode* node)
{
    if (map->key_ctor != NULL)
    {
        free(node->key);
    }
    if (map->val_ctor != NULL)
    {
        free(node->value);
    }
    free(node);
}


static void free_subtree(EpkMap*     map,
                         EpkMapNode* root)
{
    assert(map != NULL);
    assert(root != NULL);

    if (root->left != NULL)
    {
        free_subtree(map, root->left);
    }

    if (root->right != NULL)
    {
        free_subtree(map, root->right);
    }

    free_node(map, root);
}


static EpkMapNode* lower_bound(EpkMap* map, void* key)
{
    EpkMapNode* node = NULL;

    /* assure map is valid */
    assert(map != NULL);
    /* assure comparison function pointer is set */
    assert(map->cmp != NULL);
    /* assure key pointer is set */
    assert(key != NULL);

    node = map->root;
    while (node != NULL)
    {
        int cmp_res = map->cmp(key, node->key);

        if (cmp_res < 0)
        {
            /* descend into left subtree */
            node = node->left;
        }
        else
        if (cmp_res > 0)
        {
            /* descend into right subtree */
            if (node->right == NULL)
            {
                return node;
            }
            else
            {
                node = node->right;
            }
        }
        else
        {
            /* node is found and returned */
            return node;
        }
    }

    /* return NULL if node is not found */
    return node;
} /* lower_bound */


static void replace_node(EpkMap*     map,
                         EpkMapNode* old_node,
                         EpkMapNode* new_node)
{
    assert(map != NULL);
    assert(old_node != NULL);

    if (old_node->parent == NULL)
    {
        /* map root needs replacement */
        map->root = new_node;
    }
    else
    {
        /* is old node the left child? */
        if (old_node == old_node->parent->left)
        {
            /* replace parent's left child */
            old_node->parent->left = new_node;
        }
        else
        {
            /* replace parent's right child */
            old_node->parent->right = new_node;
        }
    }
    if (new_node != NULL)
    {
        new_node->parent = old_node->parent;
    }
} /* replace_node */


static void rotate_left(EpkMap* map, EpkMapNode* node)
{
    EpkMapNode* right = NULL;

    assert(map != NULL);
    assert(node != NULL);

    /* get right child */
    right = node->right;

    /* replace node with right child */
    replace_node(map, node, right);

    /* node's right child is now right child's left child */
    node->right = right->left;

    /* if right's left child is not NULL ... */
    if (right->left != NULL)
    {
        /* its parent is now the node */
        right->left->parent = node;
    }
    /* node is right's left child now */
    right->left  = node;
    node->parent = right;
}


static void rotate_right(EpkMap* map, EpkMapNode* node)
{
    EpkMapNode* left = NULL;

    assert(map != NULL);
    assert(node != NULL);

    /* get left child */
    left = node->left;

    /* replace node with left child */
    replace_node(map, node, left);

    /* node's left child is now left child's right child */
    node->left = left->right;

    /* if left's right child is not NULL ... */
    if (left->right != NULL)
    {
        left->right->parent = node;
    }

    /* node is left's right child now */
    left->right  = node;
    node->parent = left;
}


static void adapt_after_insert(EpkMap* map, EpkMapNode* node)
{
    if (node->parent == NULL)
    {
        /* Case 1:
         * empty tree; node becomes root and needs to be
         * black.
         */
        node->color = BLACK;
    }
    else
    {
        /* Case 2: */
        if (node_color(node->parent) == BLACK)
        {
            /* everything is ok in this case */
            return;
        }
        else
        {
            /* Case 3: */
            if (node_color(uncle(node)) == RED)
            {
                node->parent->color      = BLACK;
                uncle(node)->color       = BLACK;
                grandparent(node)->color = RED;
                /* restart adaptation at grandparent */
                adapt_after_insert(map, grandparent(node));
            }
            else
            {
                /* Case 4: */
                if (node == node->parent->right
                    && node->parent == grandparent(node)->left)
                {
                    rotate_left(map, node->parent);
                    node = node->left;
                }
                else
                if (node == node->parent->left
                    && node->parent == grandparent(node)->right)
                {
                    rotate_right(map, node->parent);
                    node = node->right;
                }
                /* Case 5 */
                node->parent->color      = BLACK;
                grandparent(node)->color = RED;

                if (node == node->parent->left
                    && node->parent == grandparent(node)->left)
                {
                    rotate_right(map, grandparent(node));
                }
                else
                {
                    assert(node == node->parent->right
                           && node->parent == grandparent(node)->right);
                    rotate_left(map, grandparent(node));
                }
            }
        }
    }
} /* adapt_after_insert */


/**
 * @internal
 * @brief Return node with largest key in subtree
 * @param node Root node of subtree
 * @return Node with the largest key in subtree
 */
static EpkMapNode* max_node(EpkMapNode* node)
{
    if (node != NULL)
    {
        while (node->right != NULL)
        {
            node = node->right;
        }
    }
    return node;
}


/**
 * @internal
 * @brief Return node with smallest key in subtree
 * @param node Root node of subtree
 * @return Node with the smallest key in subtree
 */
static EpkMapNode* min_node(EpkMapNode* node)
{
    if (node != NULL)
    {
        while (node->left != NULL)
        {
            node = node->left;
        }
    }

    return node;
}


/**
 * @internal
 * @brief Fix tree properties before node deletion
 * @param map Pointer to map
 * @param node Modified node as entry point
 */
static void adapt_before_delete(EpkMap*     map,
                                EpkMapNode* node)
{
    if (node->parent == NULL)
    {
        /* node is parent, which is black and on all paths, so
           all properties continue to hold. */
        return;
    }
    else
    {
        if (node_color(sibling(node)) == RED)
        {
            /* Node has a red sibling, so we swap colors and rotate */
            node->parent->color  = RED;   /* sibling's color */
            sibling(node)->color = BLACK; /* parent's color */
            /* rotate in corresponding direction */

            if (node == node->parent->left)
            {
                rotate_left(map, node->parent);
            }
            else
            {
                rotate_right(map, node->parent);
            }
        }

        if (node_color(node->parent) == BLACK
            && node_color(sibling(node)) == BLACK
            && node_color(sibling(node)->left) == BLACK
            && node_color(sibling(node)->right) == BLACK)
        {
            /* parent, sibling, and its children are black, so we
               paint the sibling red, and start over from parent */
            sibling(node)->color = RED;
            adapt_before_delete(map, node->parent);
        }
        else
        {
            if (node_color(node->parent) == RED
                && node_color(sibling(node)) == BLACK
                && node_color(sibling(node)->left) == BLACK
                && node_color(sibling(node)->right) == BLACK)
            {
                /* parent is red and sibling and its children are black,
                   so we swap color of parent and sibling */
                sibling(node)->color = RED;   /* parent's original color */
                node->parent->color  = BLACK; /* sibling's original color */
            }
            else
            {
                if (node == node->parent->left
                    && node_color(sibling(node)) == BLACK
                    && node_color(sibling(node)->left) == RED
                    && node_color(sibling(node)->right) == BLACK)
                {
                    /* swap color of sibling and its left child */
                    sibling(node)->color       = RED;
                    sibling(node)->left->color = BLACK;
                    rotate_right(map, sibling(node));
                }
                else
                if (node == node->parent->right
                    && node_color(sibling(node)) == BLACK
                    && node_color(sibling(node)->right) == RED
                    && node_color(sibling(node)->left) == BLACK)
                {
                    /* swap color of sibling and its right child */
                    sibling(node)->color        = RED;
                    sibling(node)->right->color = BLACK;
                    rotate_left(map, sibling(node));
                }

                sibling(node)->color = node_color(node->parent);
                node->parent->color  = BLACK;
                if (node == node->parent->left)
                {
                    assert(node_color(sibling(node)->right) == RED);
                    sibling(node)->right->color = BLACK;
                    rotate_left(map, node->parent);
                }
                else
                {
                    assert(node_color(sibling(node)->left) == RED);
                    sibling(node)->left->color = BLACK;
                    rotate_right(map, node->parent);
                }
            }
        }
    }
} /* adapt_before_delete */


static EpkMapNode* next_node(EpkMap* map, EpkMapNode* node)
{
    assert(map != NULL);
    assert(node != NULL);

    /* get smallest element in right subtree */
    if (node->right == NULL)
    {
        void* current_key = node->key;
        while (node != NULL
               && map->cmp(current_key, node->key) >= 0)
        {
            node = node->parent;
        }
    }
    else
    {
        node = min_node(node->right);
    }

    return node;
}


static EpkMapNode* prev_node(EpkMap* map, EpkMapNode* node)
{
    assert(map != NULL);
    assert(node != NULL);

    /* get biggest element in light subtree */
    if (node->left == NULL)
    {
        void* current_key = node->key;
        while (node != NULL
               && map->cmp(current_key, node->key) <= 0)
        {
            node = node->parent;
        }
    }
    else
    {
        node = max_node(node->left);
    }

    return node;
}


/**
 * @}
 */

/**
 * -------------------------------------------------------------------
 * @defgroup EPIK_map_construction Construction & destruction
 * -------------------------------------------------------------------
 * @{
 */

/**
 * @brief Create a map
 * This function creates an empty map structure, and registeres
 * the supplied callback functions.
 * @param cmp Comparison function to sort elements (mandatory)
 * @param key_ctor Copy-constructor function for the key (may be NULL)
 * @param val_ctor Copy-constructor function for the value (may be NULL)
 * @return New map handle
 */
EpkMap* epk_map_create(epk_compare_f   cmp,
                       epk_copy_ctor_f key_ctor,
                       epk_copy_ctor_f val_ctor)
{
    /* allocate memory */
    EpkMap* new_map = (EpkMap*)malloc(sizeof(EpkMap));

    assert(new_map != NULL);

    /* initialize root to an empty map */
    new_map->root = NULL;
    new_map->size = 0;

    /* save comparison function pointer */
    assert(cmp != NULL);
    new_map->cmp = cmp;

    /* save key/value constructors */
    new_map->key_ctor = key_ctor;
    new_map->val_ctor = val_ctor;

    VERIFY_PROPERTIES(new_map);

    return new_map;
}


/**
 * @brief Free map.
 * This function frees the map. All memory allocated for this map
 * will be freed.
 * @param map Map handle to be freed
 */
void epk_map_free(EpkMap* map)
{
    assert(map != NULL);

    if (map->root != NULL)
    {
        free_subtree(map, map->root);
    }
    free(map);
    map = NULL;
}


/**
 * @}
 * -------------------------------------------------------------------
 * @defgroup EPIK_map_iterators Iterators
 * -------------------------------------------------------------------
 * @{
 */

/**
 * @brief Create forward iterator set to the smallest element of the map
 * This function returns a forward iterator handle to be used in subsequent
 * calls. The iterator returned will point to the smallest element of the map.
 * @param map Corresponding map handle
 * @return Forward iterator handle
 * @note As C does not support automatic destruction like C++
 *       these handles need to be explicitely freed using @ref
 *       epk_map_iter_free.
 */
EpkMapIter* epk_map_begin(EpkMap* map)
{
    EpkMapNode* node = NULL;

    assert(map != NULL);
    node = min_node(map->root);

    return create_iter(map, node, FORWARD);
}


/**
 * @brief Create forward iterator set to NULL
 * This function returns a forward iterator handle to be used in subsequent
 * calls.  Note that not the handle itself is NULL, but its internal node
 * pointer.
 * @param map Corresponding map handle
 * @return Forward iterator handle
 * @note As C does not support automatic destruction like C++
 *       these handles need to be explicitely freed using @ref
 *       epk_map_iter_free.
 */
EpkMapIter* epk_map_end(EpkMap* map)
{
    assert(map != NULL);

    return create_iter(map, NULL, FORWARD);
}


/**
 * @brief Check whether an iterator is valid
 * This function is the equivalent of checking whether an iterator has
 * reached map::end.
 * @param iter Queried iterator handle
 */
int epk_map_iter_is_end(EpkMapIter* iter)
{
    assert(iter != NULL);

    return iter->node == NULL;
}


/**
 * @brief Create reverse iterator set to the largest element in the map
 * This function returns an iterator handle to be used in subsequent
 * calls. It points to the largest element of @a map.
 * @param map Corresponding map handle
 * @return Reverse iterator handle
 * @note As C does not support automatic destruction like C++
 *       these handles need to be explicitely freed using @ref
 *       epk_map_iter_free.
 */
EpkMapIter* epk_map_rbegin(EpkMap* map)
{
    EpkMapNode* node = NULL;

    assert(map != NULL);
    node = max_node(map->root);

    return create_iter(map, node, REVERSE);
}


/**
 * @brief Create a reverse iterator set to NULL
 * This function returns a reverse iterator handle to be used in subsequent
 * calls. Not that not the iterator handle itset is @a NULL, but its interal
 * node pointer.
 * @param map Corresponding map handle
 * @return Reverse iterator handle
 * @note As C does not support automatic destruction like C++
 *       these handles need to be explicitely freed using @ref
 *       epk_map_iter_free.
 */
EpkMapIter* epk_map_rend(EpkMap* map)
{
    assert(map != NULL);

    return create_iter(map, NULL, REVERSE);
}


/**
 * @brief Free iterator handle
 * @param iter Iterator handle to be free
 */
void epk_map_iter_free(EpkMapIter* iter)
{
    assert(iter != NULL);
    free(iter);
}


/**
 * @brief Get pointer to key data
 * This function returns a pointer to the key data of the element the iterator
 * is currently pointing to.
 * @param iter Iterator handle
 * @return Pointer to key data
 */
void* epk_map_iter_first(EpkMapIter* iter)
{
    assert(iter != NULL);
    assert(iter->node != NULL);

    return iter->node->key;
}


/**
 * @brief Get pointer to value data
 * This function returns a pointer to the value data of the element the iterator
 * is currently pointing to.
 * @param iter Iterator handle
 * @return Pointer to value data
 */
void* epk_map_iter_second(EpkMapIter* iter)
{
    assert(iter != NULL);
    assert(iter->node != NULL);

    return iter->node->value;
}


/**
 * @brief Advance iterator to next element
 * This function advances the iterator to the next available element in the map
 * @param iter Iterator handle
 */
void epk_map_iter_next(EpkMapIter* iter)
{
    assert(iter != NULL);
    if (iter->node == NULL)
    {
        return;
    }
    else
    {
        if (iter->dir == FORWARD)
        {
            iter->node = next_node(iter->map, iter->node);
        }
        else
        {
            iter->node = prev_node(iter->map, iter->node);
        }
    }
}


/**
 * @brief Advance iterator to next element
 * This function sets the iterator to the previous available element in the map
 * @param iter Iterator handle
 */
void epk_map_iter_prev(EpkMapIter* iter)
{
    if (iter->node == NULL)
    {
        if (iter->dir == FORWARD)
        {
            /* start at last node */
            iter->node = max_node(iter->map->root);
        }
        else
        {
            /* start at first node */
            iter->node = min_node(iter->map->root);
        }
    }
    else
    {
        if (iter->dir == FORWARD)
        {
            iter->node = prev_node(iter->map, iter->node);
        }
        else
        {
            iter->node = next_node(iter->map, iter->node);
        }
    }
} /* epk_map_iter_prev */


/**
 * @}
 * -------------------------------------------------------------------
 * @defgroup EPIK_map_capacity Capacity
 * -------------------------------------------------------------------
 * @{
 */

/**
 * @brief Test whether the map contains elements
 * @param map Corresponding map handle
 * @return Zero if map contains no elements, a non-zero value otherwise.
 */
int epk_map_empty(EpkMap* map)
{
    assert(map != NULL);
    return map->size == 0;
}


/**
 * @brief Get the number of elements contained in map
 * @param map Corresponding map handle
 * @return Number of elements
 */
size_t epk_map_size(EpkMap* map)
{
    assert(map != NULL);
    return map->size;
}


/**
 * @brief Get the maximum number of elements supported by the map
 * @param map Corresponding map handle
 * @return Maximum number of elements supported by map
 * @todo This function does not provide any useful information, as
 *       the map supports as many elements as there are unique keys
 *       and memory available.
 */
size_t epk_map_max_size(EpkMap* map)
{
    /* TODO find a sensible value here */
    assert(map);

    return 0xffffffff;
}


/**
 * @}
 * -------------------------------------------------------------------
 * @defgroup EPIK_map_modifiers Modifiers
 * -------------------------------------------------------------------
 * @{
 */

/**
 * @brief Insert element into map
 * This function insert an element into the map referenced by @a map
 * using @a key and @a value.
 * @param map Corresponding map handle
 * @param key Key of element to be added
 * @param value Value of element to be added
 */
void epk_map_insert(EpkMap* map,
                    void*   key,
                    void*   value)
{
    /* create node pointer for later use */
    EpkMapNode* new_node = NULL;

    assert(map != NULL);
    assert(key != NULL);
    assert(value != NULL);

    if (map->root == NULL)
    {
        new_node  = create_node(map, key, value, RED, NULL, NULL);
        map->root = new_node;
        map->size++;
    }
    else
    {
        /* start at root */
        EpkMapNode* node = map->root;

        /* TODO check whether this can be replaced by a find_node() */
        while (1)
        {
            int cmp_res = map->cmp(key, node->key);
            if (cmp_res == 0)
            {
                /* replace value of existing key */
                if (map->val_ctor != NULL)
                {
                    free(node->value);
                    node->value = map->val_ctor(value);
                }
                else
                {
                    node->value = value;
                }
                return;
            }
            else
            if (cmp_res < 0)
            {
                if (node->left == NULL)
                {
                    /* create new node */
                    node->left = new_node = create_node(map, key, value, RED, NULL, NULL);
                    map->size++;
                    break;
                }
                else
                {
                    /* decend into left subtree */
                    node = node->left;
                }
            }
            else
            {
                if (node->right == NULL)
                {
                    /* create new node */
                    node->right = new_node = create_node(map, key, value, RED, NULL, NULL);
                    map->size++;
                    break;
                }
                else
                {
                    /* decend into right subtree */
                    node = node->right;
                }
            }
        }
        /* sanity check */
        assert(new_node != NULL);
        /* set parent of new node */
        new_node->parent = node;
    }
    adapt_after_insert(map, new_node);

    /* verify consistent state after insertion */
    VERIFY_PROPERTIES(map);
} /* epk_map_insert */


/**
 * @brief Insert element into map at a specific location.
 * This function inserts an element into the map at the location
 * pointed to by @a iter. This reduces time complexity of the
 * insert operation.
 * @param map Corresponding map handle
 * @param iter Corresponding iterator handle
 * @param key Key of the element to be inserted
 * @param value Value of the element to be inserted
 */

void epk_map_insert_at(EpkMap*     map,
                       EpkMapIter* iter,
                       void*       key,
                       void*       value)
{
    /** @todo implement function */
    assert(map);
    assert(iter);
    assert(key);
    assert(value);
}


/**
 * @brief Delete element with a certain key.
 * @param map Corresponding map handle
 * @param key Key of the element to be deleted
 */
void epk_map_erase(EpkMap* map, void* key)
{
    EpkMapIter* iter = NULL;

    assert(map != NULL);
    assert(key != NULL);

    iter = epk_map_find(map, key);

    if (iter != NULL)
    {
        epk_map_erase_at(map, iter);
    }
}


/**
 * @brief Delete element pointed to by iterator handle
 * @param map Corresponding map handle
 * @param iter Iterator handle pointing to the element to be deleted
 */
void epk_map_erase_at(EpkMap* map, EpkMapIter* iter)
{
    EpkMapNode* node  = NULL;
    EpkMapNode* child = NULL;

    assert(map != NULL);
    /* this function must be called with valid iterator */
    assert(iter != NULL);
    assert(iter->node != NULL);

    node = iter->node;

    /* if both children are set, we need to swap with its
       max_node */
    if (node->left != NULL
        && node->right != NULL)
    {
        EpkMapNode* pred = max_node(node->left);

        node->key   = pred->key;
        node->value = pred->value;
        node        = pred;
    }

    /* node has at one child at most */
    assert(node->left == NULL
           || node->right == NULL);

    /* get potential child */
    child = (node->left != NULL) ? node->left : node->right;

    if (node_color(node) == BLACK)
    {
        node->color = node_color(child);
        /* before we actually delete the node, we ensure that in doing
           so, all tree properties will still hold. */
        adapt_before_delete(map, node);
    }

    replace_node(map, node, child);

    /* adjust child if it is the new root */
    if (node->parent == NULL
        && child != NULL)
    {
        child->color = BLACK;
    }

    free(node);
    map->size--;
    iter->node = NULL;

    VERIFY_PROPERTIES(map);
} /* epk_map_erase_at */


/**
 * @brief Delete all elements in map
 * @param map Corresponding map handle
 */
void epk_map_clear(EpkMap* map)
{
    assert(map != NULL);

    if (map->root != NULL)
    {
        free_subtree(map, map->root);
        map->root = NULL;
        map->size = 0;
    }
}


/**
 * @}
 * ----------------------------------------------------------------
 * @defgroup EPIK_map_algorithms Algorithms
 * ----------------------------------------------------------------
 * @{
 */

/**
 * @brief Find the element identified by @a key.
 * This function returns a forward iterator handle to
 * the element identified by @a key.
 * @param map Corresponding map handle
 * @param key Key identifying the element
 * @return Iterator handle
 */
EpkMapIter* epk_map_find(EpkMap* map, void* key)
{
    EpkMapNode* node;

    assert(map != NULL);
    assert(key != NULL);

    node = lower_bound(map, key);

    if (node != NULL
        && map->cmp(key, node->key) == 0)
    {
        return create_iter(map, node, FORWARD);
    }
    else
    {
        return create_iter(map, NULL, FORWARD);
    }
}


/**
 * @brief Find the first element not less than @a key .
 * @param map Map handle
 * @param key Key to find in the container
 * @return Iterator handle
 */
EpkMapIter* epk_map_lower_bound(EpkMap* map, void* key)
{
    EpkMapNode* node;

    assert(map != NULL);
    assert(key != NULL);

    node = lower_bound(map, key);

    return create_iter(map, node, FORWARD);
}


/**
 * @brief Find the first element larger than @a key.
 * @param map Map handle
 * @param key Key to find in the container
 * @return Iterator handle
 */
EpkMapIter* epk_map_upper_bound(EpkMap* map, void* key)
{
    EpkMapIter* iter;

    assert(map != NULL);
    assert(key != NULL);

    iter = epk_map_lower_bound(map, key);
    epk_map_iter_next(iter);

    return iter;
}


/**
 * @brief Apply a callback function to every element in map.
 * @param map Corresponding map handle
 * @param callback Callback function to be called for every element
 * @param data Pointer to datastructure that is passed to the callback function
 */
void epk_map_foreach(EpkMap*        map,
                     epk_callback_f callback,
                     void*          data)
{
    EpkMapIter* iter = NULL;

    assert(map != NULL);
    assert(callback != NULL);

    iter = epk_map_begin(map);
    assert(iter != NULL);

    while (iter->node != NULL)
    {
        callback(iter, data);
        epk_map_iter_next(iter);
    }
}


/**
 * @}
 */

/**
 * @}
 */
