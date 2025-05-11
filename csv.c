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

    for (int i = 0; i < schema->num_columns; i++) {
        fprintf(file, ",%s", schema->columns[i]);
    }

    if (schema->parent_id_column) {
        fprintf(file, ",%s", schema->parent_id_column);
    }

    if (schema->is_junction_table) {
        fprintf(file, ",index,value");
    }

    fprintf(file, "\n");
}

void add_seq_to_object(ASTNode *object, int seq) {
    if (!object || object->node_type != OBJECT_NODE) return;

    ASTNode *seq_node = malloc(sizeof(ASTNode));
    if (!seq_node) return;
    seq_node->node_type = NUMBER_NODE;
    seq_node->number_value = seq;
    seq_node->next = NULL;

    ASTNode *pair_node = malloc(sizeof(ASTNode));
    if (!pair_node) {
        free(seq_node);
        return;
    }

    pair_node->node_type = PAIR_NODE;
    pair_node->key = strdup("seq");
    pair_node->children = seq_node;
    pair_node->next = object->children;

    object->children = pair_node;
}

int write_object_to_csv(ASTNode *object, Schema *schema, int parent_id, const char *out_dir) {
    static int next_id = 1;
    if (!object || !schema) return -1;

    int current_id = next_id++;

    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s.csv", out_dir, schema->name);

    FILE *file = fopen(filename, "a");
    if (!file) {
        file = fopen(filename, "w");
        if (!file) {
            perror("Failed to open CSV file");
            exit(1);
        }
        write_csv_header(file, schema);
    }

    fprintf(file, "%d", current_id);

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
                    break;
                default:
                    break;
            }
        }
    }

    if (schema->parent_id_column && parent_id > 0) {
        fprintf(file, ",%d", parent_id);
    }

    fprintf(file, "\n");
    fclose(file);

    ASTNode *pair = object->children;
    while (pair) {
        if (pair->node_type == PAIR_NODE && pair->children) {
            ASTNode *child = pair->children;

            if (child->node_type == OBJECT_NODE) {
                Schema *nested_schema = get_schema_for_object(child);
                if (nested_schema) {
                    write_object_to_csv(child, nested_schema, current_id, out_dir);
                }
            }
            else if (child->node_type == ARRAY_NODE) {
                ASTNode *element = child->children;
                int index = 0;

                while (element) {
                    ASTNode *next_element = element->next;

                    if (element->node_type == OBJECT_NODE) {
                        add_seq_to_object(element, index);
                        Schema *nested_schema = get_schema_for_object(element);
                        if (nested_schema) {
                            write_object_to_csv(element, nested_schema, current_id, out_dir);
                        }
                    }
                    else if (element->node_type == STRING_NODE) {
                        char array_filename[256];
                        snprintf(array_filename, sizeof(array_filename), "%s/%s.csv", out_dir, pair->key);

                        FILE *array_file = fopen(array_filename, "a");
                        if (!array_file) {
                            array_file = fopen(array_filename, "w");
                            if (!array_file) {
                                perror("Failed to open CSV file for scalar array");
                                exit(1);
                            }
                            fprintf(array_file, "parent_id,index,value\n");
                        }

                        fprintf(array_file, "%d,%d,", current_id, index);
                        escape_csv_string(array_file, element->string_value);
                        fprintf(array_file, "\n");
                        fclose(array_file);
                    }

                    element = next_element;
                    index++;
                }
            }
        }

        pair = pair->next;
    }

    return current_id;
}

void generate_csv(ASTNode *root, const char *out_dir) {
    if (!root || root->node_type != OBJECT_NODE) {
        fprintf(stderr, "Invalid root node.\n");
        return;
    }

    struct stat st = {0};
    if (stat(out_dir, &st) == -1) {
        mkdir(out_dir, 0755);
    }

    Schema *schema = get_schema_for_object(root);
    if (!schema) {
        fprintf(stderr, "Schema not found for root object.\n");
        return;
    }

    write_object_to_csv(root, schema, 0, out_dir);
}
void free_csv_table(CSVTable *table) {
    if (table) {
        if (table->file) fclose(table->file);
        free(table->name);
        free(table);
    }
}
