#pragma once

#include <Thumby.h>

typedef uint32_t data_t;

typedef enum { TREE_BLACK, TREE_RED } tree_color_t;

typedef struct node {
  data_t _data;
  tree_color_t _color;
  struct node *left, *right, *parent;
} node_t;

//
// Interfaces
//
void tree_insert(node_t **root, data_t query);
bool tree_delete(node_t **root, data_t query);

void tree_insert_raw(node_t **root, node_t *node);
node_t *tree_search(const node_t *root, data_t query);
node_t *best_fit(const node_t *root, data_t query);
void tree_delete_raw(node_t **root, node_t *node);

bool traverse_inorder(node_t *root, bool (*)(data_t data));
bool traverse_inorder_backwards(node_t *node, bool (*func)(data_t data));
bool traverse_right_excl(node_t *node, data_t key, bool (*func)(data_t data));
bool traverse_left_excl(node_t *node, data_t key, bool (*func)(data_t data));

void tree_destroy(node_t **root);

node_t* tree_minimum(node_t *root);
node_t* tree_maximum(node_t *root);
node_t* tree_higherKey(node_t *root, data_t query);
node_t* tree_lowerKey(node_t *root, data_t query);
