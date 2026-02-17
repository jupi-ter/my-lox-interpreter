#ifndef CODEGEN_H
#define CODEGEN_H

#include "stmt.h"
#include "expr.h"
#include "entity_ast.h"
#include "parser.h"

typedef struct {
    char* header_output;
    int header_capacity;
    int header_length;
    
    char* source_output;
    int source_capacity;
    int source_length;

    int indent_level;
} CodeGen;


CodeGen codegen_create(void);
void codegen_free(CodeGen* gen);
void codegen_generate_program(CodeGen* gen, Program* program);
char* codegen_get_output(CodeGen* gen);
void codegen_write_files(CodeGen* gen, const char* header_path, const char* source_path);

#endif
