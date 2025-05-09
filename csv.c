#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "ast.h"
#include "schema.h"

typedef struct {
    char *name;
    FILE *file;
    int row_count;
} CSVTable;

void escape_csv_string(FILE *file, const char *str) {
    int needs_quoting = 0;
    const char *p = str;
    
    while (*p) {
        if (*p == ',' || *p == '"' || *p == '\n' || *p == '\r') {
            needs_quoting = 1;
            break;
        }
        p++;
    }
    
    if (!needs_quoting) {
        fprintf(file, "%s", str);
        return;
    }
    
    putc('"', file);
    p = str;
    while (*p) {
        if (*p == '"') {
            putc('"', file);
            putc('"', file);
        } else {
            putc(*p, file);
        }
        p++;
    }
    putc('"', file);
}


void write_csv_header(FILE *file, Schema *schema) {
    fprintf(file, "id"); // Primary key column

    // Add schema-defined columns
    for (int i = 0; i < schema->num_columns; i++) {
        fprintf(file, ",%s", schema->columns[i]);
    }

    // Add parent foreign key column if applicable
    if (schema->parent_id_column) {
        fprintf(file, ",%s", schema->parent_id_column);
    }

    // Add junction table-specific columns if applicable
    if (schema->is_junction_table) {
        fprintf(file, ",index,value");
    }

    fprintf(file, "\n");
}
int write_object_to_csv(ASTNode *object, Schema *schema, int parent_id, const char *out_dir) {
    static int next_id = 1;
    int current_id = next_id++;

    // Open or create the CSV file
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s.csv", out_dir, schema->name);

    FILE *file = fopen(filename, "a");
    if (!file) {
        // File doesn't exist, create it with header
        file = fopen(filename, "w");
        if (!file) {
            perror("Failed to open CSV file");
            exit(1);
        }
        write_csv_header(file, schema); // Write column names
    }

    // Write the row
    fprintf(file, "%d", current_id); // Write the primary key

    // Write regular columns
    for (int i = 0; i < schema->num_columns; i++) {
        ASTNode *pair = find_pair_in_object(object, schema->columns[i]);
        fprintf(file, ",");

        if (pair && pair->children) {
            switch (pair->children->node_type) {
                case STRING_NODE:
                    escape_csv_string(file, pair->children->string_value);
                    break;
                case NUMBER_NODE:
                    fprintf(file, "%g", pair->children->number_value);
                    break;
                case BOOLEAN_NODE:
                    fprintf(file, "%s", pair->children->boolean_value ? "true" : "false");
                    break;
                case NULL_NODE:
                    // Leave empty for NULL
                    break;
                default:
                    // Shouldn't happen for simple values
                    break;
            }
        }
    }

    // Write parent ID if this is a child table
    if (schema->parent_id_column && parent_id > 0) {
        fprintf(file, ",%d", parent_id);
    }

    fprintf(file, "\n");
    fclose(file);

    // Process nested objects and arrays
    ASTNode *pair = object->children;
    while (pair) {
        if (pair->node_type == PAIR_NODE && pair->children) {
            if (pair->children->node_type == OBJECT_NODE) {
                // Handle nested object (e.g., "profile")
                Schema *nested_schema = get_schema_for_object(pair->children);
                write_object_to_csv(pair->children, nested_schema, current_id, out_dir);
            } else if (pair->children->node_type == ARRAY_NODE) {
                // Handle array (e.g., "posts")
                ASTNode *array = pair->children;
                ASTNode *element = array->children;

                while (element) {
                    if (element->node_type == OBJECT_NODE) {
                        // Array of objects - child table
                        Schema *child_schema = get_schema_for_object(element);
                        write_object_to_csv(element, child_schema, current_id, out_dir);
                    }
                    element = element->next;
                }
            }
        }

        pair = pair->next;
    }

    return current_id; // Return the ID of the current row
}

void generate_csv(ASTNode *root, const char *out_dir) {
    // Create output directory if it doesn't exist
    struct stat st = {0};
    if (stat(out_dir, &st) == -1) {
        mkdir(out_dir, 0755);
    }
    
    if (root->node_type == OBJECT_NODE) {
        Schema *schema = get_schema_for_object(root);
        write_object_to_csv(root, schema, 0, out_dir);
    }
}
