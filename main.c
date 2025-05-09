#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "ast.h"
#include "parser.tab.h"  // Bison header
#include "csv.h"

// External declarations
extern FILE *yyin;
extern int yyparse();
extern ASTNode *ast_root;

void print_usage() {
    printf("Usage: json2relcsv <input.json> [--print-ast] [--out-dir DIR]\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 2) print_usage();

    char *input_file = NULL;
    int print_ast_flag = 0;
    char *out_dir = ".";

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--print-ast") == 0) {
            print_ast_flag = 1;
        } else if (strcmp(argv[i], "--out-dir") == 0) {
            if (i + 1 < argc) out_dir = argv[++i];
            else print_usage();
        } else if (!input_file) {
            input_file = argv[i];
        } else {
            print_usage();
        }
    }

    if (!input_file) print_usage();

    // Open input file
    FILE *input = fopen(input_file, "r");
    if (!input) {
        perror("Failed to open input file");
        return 1;
    }
    yyin = input;

    // Parse JSON input
    if (yyparse() != 0) {
        fprintf(stderr, "Parsing failed.\n");
        fclose(input);
        return 1;
    }
    fclose(input);

    // Check if AST was created
    if (!ast_root) {
        fprintf(stderr, "Error: AST root is NULL after parsing. Likely parsing failed or no AST node was created.\n");
        return 1;
    }

    // Print AST if requested
    if (print_ast_flag) {
        printf("Abstract Syntax Tree:\n");
        print_ast(ast_root, 0);
    }

    // Generate CSV output (No return value check, just call the function)
    generate_csv(ast_root, out_dir);

    // Clean up
    free_ast(ast_root);

    return 0;
}
