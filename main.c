
// https://craftinginterpreters.com/scanning.html

#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"
#include "parser.h"
#include "token.h"
#include "error.h"
#include "printer.h"

int run(char* source) {
    Scanner scanner = scanner_create(source);
    TokenList tokens = scan_tokens(&scanner);

    printf("=== TOKENS ===\n");
    for (int i = 0; i < tokens.count; i++) {
        printf("%s\n", token_to_string(tokens.data[i]));
    }
    printf("\n");

    Parser parser = parser_create(tokens);
    StmtList program = parse(&parser);

    printf("=== AST ===\n");
    print_program(program.statements, program.count);
    printf("\n");

    free_stmt_list(&program);
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
