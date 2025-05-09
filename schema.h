#ifndef SCHEMA_H
#define SCHEMA_H

#include "ast.h"

#define MAX_COLUMNS 100

typedef struct Schema Schema;  // Forward declaration for FK struct

typedef struct {
    char *column_name;
    Schema *referenced_schema;
} ForeignKey;

struct Schema {
    char *name;
    char **columns;
    int num_columns;
    char *parent_id_column;
    int is_junction_table;
    ASTNode *prototype;

    // Primary Key Support
    int has_primary_key;
    char *primary_key;

    // Foreign Key Support
    ForeignKey foreign_keys[MAX_COLUMNS];
    int num_foreign_keys;
};

Schema *get_schema_for_object(ASTNode *object);
Schema *get_junction_schema(const char *array_key);
ASTNode *find_pair_in_object(ASTNode *object, const char *key);
int object_has_same_structure(ASTNode *obj1, ASTNode *obj2);
void add_primary_key(Schema *schema, ASTNode *object);  // Optional utility

#endif
