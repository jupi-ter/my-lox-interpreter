#include "error.h"
#include <stdio.h>
#include <stdlib.h>

void error(const char* message) {
    printf("%s", message);
    exit(1);
}

void error_at_line(int line, const char* message) {
    fprintf(stderr, "[line %d] Error: %s\n", line, message);
    exit(1);  // Just die immediately
}

void error_at_token(Token token, const char* message) {
    fprintf(stderr, "[line %d] Error at '%s': %s\n", token.line, token.lexeme, message);
    exit(1);
}
