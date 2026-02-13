#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "expr.h"

typedef struct {
    Token* tokens;
    int current;
    int count;
} Parser;

Parser parser_create(TokenList tokens);
Expr* parse(Parser* parser);

#endif