#include "schema.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_SCHEMAS 100
#define MAX_COLUMNS 100

// Static variables for schema management
static Schema schemas[MAX_SCHEMAS];
static int num_schemas = 0;
static int schema_counter = 1;

static Schema junction_schemas[MAX_SCHEMAS];
static int num_junction_schemas = 0;

ASTNode *find_pair_in_object(ASTNode *object, const char *key) {
    if (!object || object->node_type != OBJECT_NODE) return NULL;
    
    ASTNode *pair = object->children;
    while (pair) {
        if (pair->node_type == PAIR_NODE && strcmp(pair->key, key) == 0) {
            return pair;
        }
        pair = pair->next;
    }
    return NULL;
}

int object_has_same_structure(ASTNode *obj1, ASTNode *obj2) {
    if (!obj1 || !obj2 || obj1->node_type != OBJECT_NODE || obj2->node_type != OBJECT_NODE) {
        return 0;
    }
    
    // Count keys in both objects
    int count1 = 0, count2 = 0;
    ASTNode *pair;
    
    for (pair = obj1->children; pair; pair = pair->next) count1++;
    for (pair = obj2->children; pair; pair = pair->next) count2++;
    
    if (count1 != count2) return 0;
    
    // Check all keys in obj1 exist in obj2 with same types
    for (pair = obj1->children; pair; pair = pair->next) {
        ASTNode *pair2 = find_pair_in_object(obj2, pair->key);
        if (!pair2) return 0;
        
        if (pair->children && pair2->children) {
            if (pair->children->node_type != pair2->children->node_type) {
                return 0;
            }
        }
    }
    
    return 1;
}
// Detect foreign keys in nested objects or arrays
void detect_nested_fk(Schema *schema, ASTNode *object) {
    ASTNode *pair = object->children;
    while (pair) {
        if (pair->node_type == PAIR_NODE) {
            const char *key = pair->key;

            // If the value is an object, check for FKs within the object
            if (pair->children && pair->children->node_type == OBJECT_NODE) {
                ASTNode *nested_object = pair->children;
                // Check if this nested object contains an FK
                if (find_pair_in_object(nested_object, "id")) {
                    // Add this as a foreign key column to the parent schema
                    schema->foreign_keys[schema->num_foreign_keys].column_name = strdup(key);
                    schema->foreign_keys[schema->num_foreign_keys].referenced_schema = get_schema_for_object(nested_object);
                    schema->num_foreign_keys++;
                }
            }
            // If the value is an array of objects, look for FK relationships in each object
            else if (pair->children && pair->children->node_type == ARRAY_NODE) {
                ASTNode *array_item = pair->children;
                while (array_item) {
                    if (array_item->node_type == OBJECT_NODE) {
                        // Check if this object contains an FK
                        if (find_pair_in_object(array_item, "id")) {
                            // Add this as a foreign key column to the parent schema
                            schema->foreign_keys[schema->num_foreign_keys].column_name = strdup(key);
                            schema->foreign_keys[schema->num_foreign_keys].referenced_schema = get_schema_for_object(array_item);
                            schema->num_foreign_keys++;
                        }
                    }
                    array_item = array_item->next;
                }
            }
        }
        pair = pair->next;
    }
}

// Updated get_schema_for_object() to handle nested FK detection
Schema *get_schema_for_object(ASTNode *object) {
    if (!object || object->node_type != OBJECT_NODE) return NULL;

    // Count columns
    int num_cols = 0;
    ASTNode *pair = object->children;
    while (pair) {
        if (pair->node_type == PAIR_NODE) {
            num_cols++;
        }
        pair = pair->next;
    }

    // Reuse existing schema
    for (int i = 0; i < num_schemas; i++) {
        if (object_has_same_structure(object, schemas[i].prototype)) {
            return &schemas[i];
        }
    }

    // Create new schema
    if (num_schemas >= MAX_SCHEMAS) {
        fprintf(stderr, "Too many schemas\n");
        exit(1);
    }

    Schema *schema = &schemas[num_schemas++];
    schema->name = malloc(32);
    snprintf(schema->name, 32, "table%d", schema_counter++);
    schema->columns = malloc(num_cols * sizeof(char *));
    if (!schema->columns) {
        perror("Failed to allocate schema columns");
        exit(1);
    }

    schema->num_columns = num_cols;
    schema->parent_id_column = NULL;
    schema->is_junction_table = 0;
    schema->prototype = object;

    schema->primary_key = NULL;
    schema->num_foreign_keys = 0;

    // Populate columns and detect PK/FKs
    int i = 0;
    pair = object->children;
    while (pair) {
        if (pair->node_type == PAIR_NODE) {
            const char *key = pair->key;
            schema->columns[i++] = strdup(key);

            // Detect primary key
            if (strcmp(key, "id") == 0) {
                schema->primary_key = strdup(key);
            }

            // Detect foreign keys
            if (strlen(key) > 3 && strcmp(key + strlen(key) - 3, "_id") == 0 && strcmp(key, "id") != 0) {
                // Attempt to find a referenced schema with PK = id
                for (int j = 0; j < num_schemas; j++) {
                    for (int k = 0; k < schemas[j].num_columns; k++) {
                        if (strcmp(schemas[j].columns[k], "id") == 0) {
                            schema->foreign_keys[schema->num_foreign_keys].column_name = strdup(key);
                            schema->foreign_keys[schema->num_foreign_keys].referenced_schema = &schemas[j];
                            schema->num_foreign_keys++;
                            break;
                        }
                    }
                }
            }
        }
        pair = pair->next;
    }

    // Check for nested FKs (in objects or arrays)
    detect_nested_fk(schema, object);

    return schema;
}



Schema *get_junction_schema(const char *array_key) {
    // Check existing junction schemas
    for (int i = 0; i < num_junction_schemas; i++) {
        if (strcmp(junction_schemas[i].name, array_key) == 0) {
            return &junction_schemas[i];
        }
    }
    
    // Create new junction schema
    if (num_junction_schemas >= MAX_SCHEMAS) {
        fprintf(stderr, "Too many junction schemas\n");
        exit(1);
    }
    
    Schema *schema = &junction_schemas[num_junction_schemas++];
    schema->name = strdup(array_key);
    schema->num_columns = 0;
    schema->columns = NULL;
    schema->parent_id_column = NULL;
    schema->is_junction_table = 1;
    schema->prototype = NULL;
    
    return schema;
}


// Function to free memory used by a schema
void free_schema(Schema *schema) {
    if (schema) {
        // Free the schema's name
        if (schema->name) {
            free(schema->name);
        }

        // Free column names
        if (schema->columns) {
            for (int i = 0; i < schema->num_columns; i++) {
                if (schema->columns[i]) {
                    free(schema->columns[i]);
                }
            }
            free(schema->columns);
        }

        // Free primary key
        if (schema->primary_key) {
            free(schema->primary_key);
        }

        // Free foreign keys
        for (int i = 0; i < schema->num_foreign_keys; i++) {
            if (schema->foreign_keys[i].column_name) {
                free(schema->foreign_keys[i].column_name);
            }
        }

        // If the schema has a prototype, it will be freed elsewhere (not shown here)
    }
}

// Free all schemas in the global schema list
void free_all_schemas() {
    for (int i = 0; i < num_schemas; i++) {
        free_schema(&schemas[i]);
    }
    num_schemas = 0;
}
