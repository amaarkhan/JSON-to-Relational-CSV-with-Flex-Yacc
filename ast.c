#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode *ast_root = NULL;

ASTNode *create_ast_node(NodeType type) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (!node) {
        perror("Failed to allocate AST node");
        exit(1);
    }
    node->node_type = type;
    node->key = NULL;
    node->string_value = NULL;
    node->children = NULL;
    node->next = NULL;
    return node;
}

ASTNode *make_string(char *val) {
    ASTNode *node = create_ast_node(STRING_NODE);
    node->string_value = strdup(val);  // Duplicate the string
    return node;
}

ASTNode *make_number(double val) {
    ASTNode *node = create_ast_node(NUMBER_NODE);
    node->number_value = val;
    return node;
}

ASTNode *make_bool(int val) {
    ASTNode *node = create_ast_node(BOOLEAN_NODE);
    node->boolean_value = val;
    return node;
}

ASTNode *make_null(void) {
    return create_ast_node(NULL_NODE);
}

ASTNode *make_object(ASTNode *pair_list) {
    ASTNode *node = create_ast_node(OBJECT_NODE);
    node->children = pair_list;
    return node;
}

ASTNode *make_array(ASTNode *elements) {
    ASTNode *node = create_ast_node(ARRAY_NODE);
    node->children = elements;
    return node;
}

ASTNode *make_pair(char *key, ASTNode *value) {
    ASTNode *node = create_ast_node(PAIR_NODE);
    node->key = strdup(key);  // Duplicate the key string
    node->children = value;
    return node;
}

ASTNode *make_pair_list(ASTNode *pair, ASTNode *pair_list) {
    if (!pair) return pair_list;
    pair->next = pair_list;
    return pair;
}

ASTNode *make_array_list(ASTNode *element, ASTNode *element_list) {
    if (!element) return element_list;
    element->next = element_list;
    return element;
}

// Recursive free function to free all dynamically allocated nodes
void free_ast(ASTNode *node) {
    if (!node) return;

    // Free children and next sibling recursively
    free_ast(node->children);
    free_ast(node->next);

    // Free any dynamically allocated memory for key or string value
    if (node->key) free(node->key);
    if (node->node_type == STRING_NODE && node->string_value) {
        free(node->string_value);  // Free string value if it's a STRING_NODE
    }

    // Finally, free the node itself
    free(node);
}

void print_ast(ASTNode *node, int indent) {
    if (!node) return;

    // Print indentation
    for (int i = 0; i < indent; i++) printf("  ");

    // Print the node type
    switch (node->node_type) {
        case OBJECT_NODE:
            printf("OBJECT\n");
            break;
        case ARRAY_NODE:
            printf("ARRAY\n");
            break;
        case PAIR_NODE:
            printf("PAIR: %s\n", node->key);
            break;
        case STRING_NODE:
            printf("STRING: %s\n", node->string_value);
            return;
        case NUMBER_NODE:
            printf("NUMBER: %g\n", node->number_value);
            return;
        case BOOLEAN_NODE:
            printf("BOOLEAN: %s\n", node->boolean_value ? "true" : "false");
            return;
        case NULL_NODE:
            printf("NULL\n");
            return;
    }

    // Recursively print the children nodes
    ASTNode *child = node->children;
    while (child) {
        print_ast(child, indent + 1);
        child = child->next;
    }
}
