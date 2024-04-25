#include "rbtree.h"
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

rbtree *new_rbtree(void);
node_t *create_node(rbtree *t, const key_t key);
node_t *create_nil(void);
node_t *rbtree_find(const rbtree *t, const key_t key);

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
// int reconcile_case_checker(rbtree *t, node_t *target);

// delete tree
void delete_rbtree(rbtree *t);
void post_order_transversal(rbtree *t, node_t *node);

// rbtree_to_array
int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n);
void in_order_traversal(const rbtree *t, node_t *node, key_t *arr,
                        size_t *index, const size_t n);

bool is_left_child(rbtree *t, node_t *target);
void print_rbtree(rbtree *t, node_t *cur);

// --------------------------------------------------------------

rbtree *new_rbtree(void) {
  // TODO: initialize struct if needed
  rbtree *tree = (rbtree *)calloc(1, sizeof(rbtree));
  node_t *nil = create_nil();

  if (!tree || !nil) return NULL;

  tree->nil = nil;
  tree->root = nil;

  return tree;
}

/* use only double pointer */
node_t *rbtree_insert(rbtree *t, const key_t key) {
  if (!t) return NULL;

  node_t *new_node = create_node(t, key);
  node_t *nil = t->nil;

  if (t->root == t->nil) {
    t->root = new_node;
    t->root->color = RBTREE_BLACK;
    return new_node;
  }

  new_node->parent = t->root;

  while (1) {

    node_t **cur =
        (new_node->key < new_node->parent->key)
            ? &(new_node->parent->left)
            : &(new_node->parent->right);  // new_node의 key값이 cur의 key값보다
                                           // 작으면 왼쪽으로 이동

    if (*cur == nil) {
      // nil 노드를 가르키던 포인터를 new node를 가르키도록 변경함
      *cur = new_node;
      break;
    }

    new_node->parent = *cur;
  }

  reconcile_insert(t, new_node);

  return new_node;
}

/* use double pointer and prev variable */
// node_t *rbtree_insert(rbtree *t, const key_t key) {
//   if (!t) return NULL;

//   node_t *new_node = create_node(t, key);
//   node_t *prev = t->root;
//   node_t *nil = t->nil;

//   if (t->root == nil) {
//     t->root = new_node;
//     t->root->color = RBTREE_BLACK;
//     return new_node;
//   }

//   while (1) {
//     // new_node의 key값이 cur의 key값보다 작으면 왼쪽으로 이동
//     node_t **cur = (new_node->key < prev->key) ? &(prev->left) :
//     &(prev->right);

//     if (*cur == nil) {
//       // nil 노드를 가르키던 포인터를 new node를 가르키도록 변경함
//       *cur = new_node;
//       break;
//     }

//     new_node->parent = prev;
//   }

//   reconcile_insert(t, new_node);

//   return new_node;
// }

/* use prev pointer */
// node_t *rbtree_insert(rbtree *t, const key_t key) {
//   if (!t) return NULL;

//   node_t *new_node = create_node(t, key);
//   node_t *prev = t->root;
//   node_t *nil = t->nil;

//   if (t->root == nil) {
//     t->root = new_node;
//     t->root->color = RBTREE_BLACK;
//     return new_node;
//   }

//   while (1) {
//     // new_node의 key값이 cur의 key값보다 작으면 왼쪽으로 이동
//     node_t **cur = (new_node->key < prev->key) ? &(prev->left) :
//     &(prev->right);

//     if (*cur == nil) {
//       // nil 노드를 가르키던 포인터를 new node를 가르키도록 변경함
//       *cur = new_node;
//       break;
//     }

//     new_node->parent = prev;
//   }

//   reconcile_insert(t, new_node);

//   return new_node;
// }

int rbtree_erase(rbtree *t, node_t *target) {
  node_t *replacer = target;
  node_t *suc = NULL;
  color_t y_original_color = replacer->color;

  if (target->left == t->nil) {
    suc = target->right;
    transplant(t, target, target->right);
  } else if (target->right == t->nil) {
    suc = target->left;
    transplant(t, target, target->left);
  } else {
    replacer = get_successor(t, target);
    y_original_color = replacer->color;
    suc = replacer->right;  // successor가 삭제된 곳을 대체할 노드를 설정
    // left는 없겠지..(successor 니까)

    if (replacer->parent == target) {  // 바로 위에 있을 경우
      suc->parent = replacer;
    } else {
      // 위에 없을경우 will_del은 그 right가 됨
      transplant(t, replacer, replacer->right);
      replacer->right = target->right;
      replacer->right->parent = replacer;
    }

    transplant(t, target, replacer);
    // will_del의 parent를 업데이트하지않음
    replacer->left = target->left;
    replacer->left->parent = replacer;
    replacer->color = target->color;
  }
  if (y_original_color == RBTREE_BLACK) {
    reconcile_erase(t, suc);
  }

  free(target);

  return 1;
}

void delete_rbtree(rbtree *t) {
  // TODO: reclaim the tree nodes's memory
  post_order_transversal(t, t->root);

  free(t->nil);
  free(t);
}

node_t *rbtree_find(const rbtree *t, const key_t key) {
  // TODO: implement find
  if (t->root == t->nil) return NULL;

  node_t *cur = t->root;

  while (1) {
    if (cur->key == key) return cur;
    if (cur == t->nil) return NULL;

    if (cur->key < key) cur = cur->right;
    if (cur->key > key) cur = cur->left;
  }

  return NULL;
}

node_t *rbtree_min(const rbtree *t) {
  // TODO: implement find
  node_t *cur = t->root;

  while (1) {
    if (cur->left == t->nil) break;

    cur = cur->left;
  }

  return cur;
}

node_t *rbtree_max(const rbtree *t) {
  // TODO: implement find
  node_t *cur = t->root;

  while (1) {
    if (cur->right == t->nil) break;

    cur = cur->right;
  }

  return cur;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  size_t index = 0;

  in_order_traversal(t, t->root, arr, &index, n);

  return 0;
}

void in_order_traversal(const rbtree *t, node_t *node, key_t *arr,
                        size_t *index, const size_t n) {
  if (node == t->nil || *index >= n) return;

  in_order_traversal(t, node->left, arr, index, n);
  arr[(*index)++] = node->key;
  in_order_traversal(t, node->right, arr, index, n);
}

void reconcile_insert(rbtree *t, node_t *cur) {
  if (!cur) return;

  node_t *parent = NULL;
  node_t *uncle = NULL;
  node_t *grand = NULL;

  while (1) {
    if (cur->parent->color == RBTREE_BLACK) break;

    parent = cur->parent;
    grand = parent->parent;
    uncle = grand->left == parent ? grand->right : grand->left;

    if (uncle->color == RBTREE_RED) {
      flip_color(3, parent, uncle, grand);
      cur = grand;
      t->root->color = RBTREE_BLACK;
      continue;
    }

    if (parent == grand->left) {
      if (uncle == RBTREE_RED) {
        flip_color(3, parent, uncle, grand);
        cur = grand;
        t->root->color = RBTREE_BLACK;
        continue;
      }

      if (cur == parent->right) {
        left_rotation(t, parent);

        node_t *temp = parent;
        parent = cur;
        cur = temp;
      }

      flip_color(2, parent, grand);
      right_rotation(t, grand);

    } else {

      if (uncle->color == RBTREE_RED) {
        flip_color(3, parent, uncle, grand);
        cur = grand;
        t->root->color = RBTREE_BLACK;
        continue;
      }

      if (cur == parent->left) {
        right_rotation(t, parent);

        node_t *temp = parent;
        parent = cur;
        cur = temp;
      }

      flip_color(2, parent, grand);
      left_rotation(t, grand);
    }

    t->root->color = RBTREE_BLACK;
  }
}

void reconcile_erase(rbtree *t, node_t *target) {
  node_t *uncle = NULL;
  node_t *parent = NULL;

  while (target != t->root && target->color == RBTREE_BLACK) {
    parent = target->parent;

    if (target == parent->left) {
      uncle = parent->right;
      if (uncle->color == RBTREE_RED) {
        uncle->color = RBTREE_BLACK;
        parent->color = RBTREE_RED;
        left_rotation(t, parent);
        uncle = parent->right;
      }
      if (uncle->left->color == RBTREE_BLACK &&
          uncle->right->color == RBTREE_BLACK) {
        uncle->color = RBTREE_RED;
        target = parent;
      } else {
        if (uncle->right->color == RBTREE_BLACK) {
          uncle->left->color = RBTREE_BLACK;
          uncle->color = RBTREE_RED;
          right_rotation(t, uncle);
          uncle = parent->right;
        }
        uncle->color = parent->color;
        parent->color = RBTREE_BLACK;
        uncle->right->color = RBTREE_BLACK;
        left_rotation(t, parent);
        target = t->root;
      }
    } else {
      uncle = parent->left;
      if (uncle->color == RBTREE_RED) {
        uncle->color = RBTREE_BLACK;
        parent->color = RBTREE_RED;
        right_rotation(t, parent);
        uncle = parent->left;
      }
      if (uncle->right->color == RBTREE_BLACK &&
          uncle->left->color == RBTREE_BLACK) {
        uncle->color = RBTREE_RED;
        target = parent;
      } else {
        if (uncle->left->color == RBTREE_BLACK) {
          uncle->right->color = RBTREE_BLACK;
          uncle->color = RBTREE_RED;
          left_rotation(t, uncle);
          uncle = parent->left;
        }
        uncle->color = parent->color;
        parent->color = RBTREE_BLACK;
        uncle->left->color = RBTREE_BLACK;
        right_rotation(t, parent);
        target = t->root;
      }
    }
  }
  target->color = RBTREE_BLACK;
}

node_t *create_node(rbtree *t, const key_t key) {
  if (!key) return NULL;

  node_t *new_node = (node_t *)calloc(1, sizeof(node_t));

  new_node->key = key;
  new_node->color = RBTREE_RED;
  new_node->parent = new_node->left = new_node->right = t->nil;

  return new_node;
}

node_t *create_nil(void) {
  node_t *nil = (node_t *)calloc(1, sizeof(node_t));
  if (!nil) return NULL;

  nil->key = INT_MIN;
  nil->color = RBTREE_BLACK;
  nil->parent = nil->left = nil->right = nil;

  return nil;
}

void transplant(rbtree *t, node_t *origin, node_t *overwrite) {
  if (origin->parent == t->nil) {
    t->root = overwrite;
  } else if (is_left_child(t, origin)) {
    origin->parent->left = overwrite;
  } else {
    origin->parent->right = overwrite;
  }

  overwrite->parent = origin->parent;
}

bool is_left_child(rbtree *t, node_t *target) {
  if (target == t->root) return false;

  node_t *parent = target->parent;

  return (parent->left == target) ? true : false;
}

node_t *get_successor(rbtree *t, node_t *node) {
  if (node->right == t->nil) return node;

  node_t *cur = node->right;

  while (1) {
    if (cur->left == t->nil) break;

    cur = cur->left;
  }

  return cur;
}

void print_rbtree(rbtree *t, node_t *cur) {
  if (cur == t->nil) return;

  printf("%d ->", cur->key);
  print_rbtree(t, cur->left);
  print_rbtree(t, cur->right);
}

void post_order_transversal(rbtree *t, node_t *node) {
  if (node == t->nil) return;

  post_order_transversal(t, node->left);
  post_order_transversal(t, node->right);
  free(node);
}

void flip_color(int count, ...) {
  va_list args;
  va_start(args, count);

  for (int i = 0; i < count; i++) {
    node_t *node = va_arg(args, node_t *);
    if (!node) continue;

    node->color = (node->color == RBTREE_RED) ? RBTREE_BLACK : RBTREE_RED;
  }

  va_end(args);
}

void right_rotation(rbtree *t, node_t *pivot) {
  node_t *child = pivot->left;
  pivot->left = child->right;

  if (child->right != t->nil) {
    child->right->parent = pivot;
  }

  if (pivot == pivot->parent->left) {
    pivot->parent->left = child;
  } else if (pivot == pivot->parent->right) {
    pivot->parent->right = child;
  } else {
    t->root = child;
  }

  child->right = pivot;
  child->parent = pivot->parent;
  pivot->parent = child;
}

void left_rotation(rbtree *t, node_t *pivot) {
  node_t *child = pivot->right;

  pivot->right = child->left;
  if (child->left != t->nil) {
    child->left->parent = pivot;
  }

  if (pivot == pivot->parent->left) {
    pivot->parent->left = child;
  } else if (pivot == pivot->parent->right) {
    pivot->parent->right = child;
  } else {
    t->root = child;
  }

  child->left = pivot;
  child->parent = pivot->parent;
  pivot->parent = child;
}
