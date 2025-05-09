#ifndef CSV_H
#define CSV_H

#include "ast.h"
#include "schema.h"

/**
 * Escapes special characters in a string to make it CSV-safe.
 * This includes wrapping the string in quotes if it contains commas, newlines, or quotes.
 * 
 * @param file The file pointer to write the escaped string to.
 * @param str The string to escape.
 */
void escape_csv_string(FILE *file, const char *str);

/**
 * Writes the header of a CSV file based on the provided schema.
 * The header includes the columns of the schema and any additional columns for parent ID
 * or junction tables if applicable.
 * 
 * @param file The file pointer to write the header to.
 * @param schema The schema describing the table structure.
 */
void write_csv_header(FILE *file, Schema *schema);

/**
 * Writes a single object to the appropriate CSV file based on its schema.
 * This includes writing the regular columns as well as handling child tables or junction tables
 * in case of arrays or nested objects.
 * 
 * @param object The ASTNode representing the object to write to CSV.
 * @param schema The schema for the object.
 * @param parent_id The ID of the parent object (used for child tables).
 * @param out_dir The directory where the CSV files will be saved.
 */
void write_object_to_csv(ASTNode *object, Schema *schema, int parent_id, const char *out_dir);

/**
 * Generates CSV files for the given ASTNode (root), writing the data into the specified output directory.
 * 
 * @param root The root ASTNode of the object to be serialized into CSV files.
 * @param out_dir The directory where the CSV files will be saved.
 */
void generate_csv(ASTNode *root, const char *out_dir);

#endif // CSV_H
