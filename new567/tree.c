#include <stdlib.h>
#include <stdio.h>
#include "tree.h"

void delete_node(Node* node) {
    if (node == NULL) {
        return;
    }
    delete_node(node->right);
    delete_node(node->left);
    free(node);
}

Node* push(Node* root, int val) {
    if (root == NULL) {
        root = (Node*)malloc(sizeof(Node));
        root->id = val;
        root->left = NULL;
        root->right = NULL;
        return root;
    } else if (val < root->id) {
        root->left = push(root->left, val);
    } else if (val >= root->id) {
        root->right = push(root->right, val);
    }
    return root;
}

Node* kill(Node* root_node, int val) {
    Node* node;
    if (root_node == NULL) {
        return NULL;
    } else if (val < root_node->id) {
        root_node->left = kill(root_node->left, val);
    } else if (val > root_node->id) {
        root_node->right = kill(root_node->right, val);
    } else {
        node = root_node;
        if (root_node->left == NULL) {
            root_node = root_node->right;
        } else if (root_node->right == NULL) {
            root_node = root_node->left;
        }
        free(node);
    }
    if (root_node == NULL) {
        return root_node;
    }
    return root_node;
}

void get_nodes(Node* node, int* result, int* count) {
    if (node == NULL) {
        return;
    }
    get_nodes(node->left, result, count);
    result[(*count)++] = node->id;
    get_nodes(node->right, result, count);
}

void tree_get_nodes(Tree* tree, int* result, int* count) {
    *count = 0;
    get_nodes(tree->root, result, count);
}

Tree* create_tree() {
    Tree* tree = (Tree*)malloc(sizeof(Tree));
    tree->root = NULL;
    return tree;
}

void destroy_tree(Tree* tree) {
    delete_node(tree->root);
    free(tree);
}
