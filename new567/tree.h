#ifndef TREE_H
#define TREE_H

#include <stdlib.h>
#include <vector>

struct Node
{
    int id;
    struct Node *left;
    struct Node *right;
};

struct Tree
{
    struct Node *root;
};

void tree_init(struct Tree *tree);
void tree_push(struct Tree *tree, int id);
void tree_kill(struct Tree *tree, int id);
void tree_get_nodes(const struct Tree *tree, std::vector<int> *result);
void tree_destroy(struct Tree *tree);

#endif // TREE_H
