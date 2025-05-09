# JSON to Relational CSV Converter

This project converts JSON data into relational CSV files, grouping objects into tables and linking them with primary and foreign keys. It uses **Flex** for lexical analysis, **Bison** for parsing, and C for semantic analysis and CSV generation.

---

## Features

1. **JSON Parsing**:
   - Supports JSON objects, arrays, strings, numbers, booleans, and null values.
   - Validates JSON syntax and reports errors with line and column numbers.

2. **Relational Mapping**:
   - Groups objects with the same keys into one table.
   - Handles nested objects and arrays by creating child tables with foreign keys.
   - Supports junction tables for arrays of scalar values.

3. **CSV Generation**:
   - Creates one `.csv` file per table.
   - Writes headers and rows with primary keys, foreign keys, and values.
   - Handles large JSON inputs (up to 30 MiB).

4. **Error Handling**:
   - Reports lexical, syntax, and runtime errors with clear messages.
   - Frees all allocated memory before exiting.

5. **AST Visualization**:
   - Prints the Abstract Syntax Tree (AST) in an indented format when the `--print-ast` flag is used.

---

## Project Structure

### **Files**
- **`main.c`**: Entry point of the program. Handles command-line arguments and coordinates the parsing, semantic analysis, and CSV generation.
- **`scanner.l`**: Flex file for lexical analysis. Tokenizes JSON input and tracks line/column numbers.
- **`parser.y`**: Bison file for parsing. Validates JSON syntax and builds the Abstract Syntax Tree (AST).
- **`ast.h` / `ast.c`**: Defines and implements AST node structures and helper functions.
- **`schema.h` / `schema.c`**: Handles table schema creation and foreign key detection.
- **`csv.h` / `csv.c`**: Implements CSV generation, including writing headers and rows.
- **`sample.json`**: Example JSON input file.
- **`command.txt`**: Contains build and run commands.
- **`Readme.md`**: Documentation for the project.

---

## Build Instructions

1. **Install Required Tools**:
   - Ensure the following tools are installed:
     - **Flex**: For lexical analysis.
     - **Bison**: For parsing.
     - **GCC**: For compiling the C code.

2. **Build the Project**:
   Run the following commands in the project directory:
   ```bash
   bison -d parser.y
   flex scanner.l
   make
   ```

---

## Run Instructions

1. **Basic Usage**:
   ```bash
   ./json2relcsv <input.json> [--print-ast] [--out-dir DIR]
   ```

2. **Examples**:
   - Convert `sample.json` to CSV files in the `./output_directory`:
     ```bash
     ./json2relcsv sample.json --out-dir ./output_directory
     ```
   - Print the AST for `sample.json`:
     ```bash
     ./json2relcsv sample.json --print-ast
     ```

---

## Design Notes

### **1. Lexical Analysis**
- **Tool**: Flex (`scanner.l`).
- **Purpose**: Tokenizes JSON input into meaningful components (e.g., strings, numbers, punctuation).
- **Key Features**:
  - Tracks line and column numbers for error reporting.
  - Supports escape sequences in strings (e.g., `\n`, `\u1234`).
  - Ignores whitespace but tracks newlines.

### **2. Parsing**
- **Tool**: Bison (`parser.y`).
- **Purpose**: Validates JSON syntax and builds the Abstract Syntax Tree (AST).
- **Key Features**:
  - Grammar rules for JSON objects, arrays, and values.
  - Constructs a simplified AST (not a full parse tree) for easier processing.

### **3. Semantic Analysis**
- **Files**: `ast.c`, `schema.c`.
- **Purpose**: Processes the AST to group data into relational tables.
- **Key Features**:
  - Groups objects with the same keys into one table.
  - Handles nested objects and arrays by creating child tables with foreign keys.
  - Assigns unique IDs to rows and links parent-child relationships.

### **4. CSV Generation**
- **Files**: `csv.c`.
- **Purpose**: Writes table data to `.csv` files.
- **Key Features**:
  - Creates one `.csv` file per table.
  - Writes headers and rows with primary keys, foreign keys, and values.
  - Saves files incrementally to handle large inputs.

### **5. Error Handling**
- **Lexical Errors**:
  - Detected in `scanner.l`.
  - Example: Unclosed string `"hello`.
- **Syntax Errors**:
  - Detected in `parser.y`.
  - Example: Missing comma in `{"key": 1 "key2": 2}`.
- **Runtime Errors**:
  - File errors (e.g., unable to open a file) are reported with `perror()`.
  - Memory allocation errors are partially handled with `malloc()` checks.
- **Memory Management**:
  - The `free_ast()` function in `ast.c` frees the AST before exiting.

---

## Example Input and Output

### **Input (`sample.json`)**
```json
{
    "postId": 101,
    "author": {"uid": "u1", "name": "Sara"},
    "comments": [
        {"uid": "u2", "text": "Nice!"},
        {"uid": "u3", "text": "+1"}
    ]
}
```

### **Output**
1. **`posts.csv`**
   ```
   id,postId,author_id
   1,101,1
   ```

2. **`users.csv`**
   ```
   id,uid,name
   1,u1,Sara
   2,u2,
   3,u3,
   ```

3. **`comments.csv`**
   ```
   post_id,seq,user_id,text
   1,0,2,"Nice!"
   1,1,3,"+1"
   ```

---

## Known Limitations
- Memory allocation errors are not fully handled in all cases.
- The program assumes valid UTF-8 encoding for JSON input.

---

## Future Improvements
- Add more robust memory error handling.
- Optimize performance for very large JSON files.
- Add support for additional JSON features (e.g., comments in JSON).

---

## Authors
- **Amaar** (Developer)
- **Contributors**: None yet.

---

## License
This project is licensed under the MIT License.
