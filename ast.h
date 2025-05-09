#ifndef AST_H
#define AST_H

typedef enum {
    OBJECT_NODE,
    ARRAY_NODE,
    PAIR_NODE,
    STRING_NODE,
    NUMBER_NODE,
    BOOLEAN_NODE,
    NULL_NODE
} NodeType;

typedef struct ASTNode {
    NodeType node_type;
    char *key;  // For PAIR_NODE
    union {
        char *string_value;
        double number_value;
        int boolean_value;
    };
    struct ASTNode *children;  // For OBJECT or ARRAY
    struct ASTNode *next;      // For sibling nodes
} ASTNode;

extern ASTNode *ast_root;

ASTNode *create_ast_node(NodeType type);
void free_ast(ASTNode *node);
void print_ast(ASTNode *node, int indent);

// JSON constructors
ASTNode *make_string(char *val);
ASTNode *make_number(double val);
ASTNode *make_bool(int val);
ASTNode *make_null(void);
ASTNode *make_object(ASTNode *pair_list);
ASTNode *make_array(ASTNode *elements);
ASTNode *make_pair(char *key, ASTNode *value);
ASTNode *make_pair_list(ASTNode *pair, ASTNode *pair_list);
ASTNode *make_array_list(ASTNode *element, ASTNode *element_list);

#endif
