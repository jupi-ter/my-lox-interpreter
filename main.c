
// https://craftinginterpreters.com/scanning.html

#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"
#include "parser.h"
#include "token.h"
#include "error.h"
#include "entity_ast.h"
#include "printer.h"
#include "codegen.h"

int run(char* source) {
    Scanner scanner = scanner_create(source);
    TokenList tokens = scan_tokens(&scanner);

    printf("=== TOKENS ===\n");
    for (int i = 0; i < tokens.count; i++) {
        printf("%s\n", token_to_string(tokens.data[i]));
    }
    printf("\n");

    Parser parser = parser_create(tokens);
    Program program = parse(&parser);

    printf("=== AST ===\n");
    print_program(program.statements, program.count);
    printf("\n=== ENTITIES ===\n");
    printf("Found %d entities\n", program.entity_count);
    for (int i = 0; i < program.entity_count; i++) {
        printf("Entity: %s (%d fields)\n", 
            program.entities[i]->name.lexeme,
            program.entities[i]->field_count);
    }
    printf("\n");

    printf("=== GENERATED C CODE ===\n");
    CodeGen codegen = codegen_create();
    codegen_generate_program(&codegen, &program);
    printf("%s\n", codegen.header_output);
    printf("%s\n", codegen.source_output);
    codegen_write_files(&codegen, "../RatGameC/src/game_generated.h", "../RatGameC/src/game_generated.c");
    codegen_free(&codegen);

    free_program(&program);
    free_token_list(&tokens);

    return 0;
}

char* read_all_bytes(char* script) {
    FILE* file = fopen(script, "rb");

    if (!file) {
        error(error_messages[ERROR_FILELOAD].message);
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* file_content_buffer = malloc( file_size + 1); // null terminator

    //malloc can fail so
    if (!file_content_buffer) {
        fclose(file);
        error(error_messages[ERROR_MALLOCFAIL].message);
    }

    size_t bytes_read = fread(file_content_buffer, 1, file_size, file);
    file_content_buffer[bytes_read] = '\0';

    fclose(file);
    return file_content_buffer;
}

int run_file(char* script) {
    char* file_contents = read_all_bytes(script);

    run(file_contents);
    free(file_contents);

    return 0;
}

int main(int argc, char** argv) {
    if (argc > 2) {
        error(error_messages[ERROR_ARGC].message);
    } else if (argc == 2) {
        run_file(argv[1]);
    } else {
        error(error_messages[ERROR_USAGE].message);
    }

    printf("Exited with no errors.");
    return 0;
}
