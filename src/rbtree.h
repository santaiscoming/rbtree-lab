#ifndef _RBTREE_H_
#define _RBTREE_H_

#include <stdbool.h>
#include <stddef.h>

typedef enum { RBTREE_RED, RBTREE_BLACK } color_t;

typedef int key_t;

typedef struct node_t {
  color_t color;
  key_t key;
  struct node_t *parent, *left, *right;
} node_t;

typedef struct {
  node_t *root;
  node_t *nil;  // for sentinel
} rbtree;

rbtree *new_rbtree(void);
node_t *create_node(rbtree *t, const key_t key);
node_t *create_nil(void);
node_t *rbtree_find(const rbtree *t, const key_t key);

// rbtree min, max
node_t *rbtree_min(const rbtree *t);
node_t *rbtree_max(const rbtree *t);

// insert node
node_t *rbtree_insert(rbtree *t, const key_t key);
void reconcile_insert(rbtree *t, node_t *cur);
void right_rotation(rbtree *t, node_t *pivot);
void left_rotation(rbtree *t, node_t *pivot);

// delete node
int rbtree_erase(rbtree *t, node_t *p);
void reconcile_erase(rbtree *t, node_t *cur);
node_t *get_successor(rbtree *t, node_t *node);
void flip_color(int count, ...);
void transplant(rbtree *t, node_t *origin, node_t *overwrite);

// delete tree
void delete_rbtree(rbtree *t);
void post_order_transversal(rbtree *t, node_t *node);

// rbtree_to_array
int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n);
void in_order_traversal(const rbtree *t, node_t *node, key_t *arr,
                        size_t *index, const size_t n);

bool is_left_child(rbtree *t, node_t *target);
void print_rbtree(rbtree *t, node_t *cur);

#endif  // _RBTREE_H_
