#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "expr.h"
#include "stmt.h"

typedef struct {
    Token* tokens;
    int current;
    int count;
} Parser;

typedef struct {
    Stmt** statements;
    int count;
} StmtList;

Parser parser_create(TokenList tokens);
StmtList parse(Parser* parser);
void free_stmt_list(StmtList* list);

#endif