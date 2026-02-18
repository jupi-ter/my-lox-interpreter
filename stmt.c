#include "stmt.h"
#include "error.h"
#include <stdlib.h>

Stmt* stmt_expression(Expr* expression) {
    Stmt* stmt = malloc(sizeof(Stmt));
    if (!stmt) error(error_messages[ERROR_MALLOCFAIL].message);

    stmt->type = STMT_EXPRESSION;
    stmt->as.expr.expr = expression;

    return stmt;
}

Stmt* stmt_print(Expr* expression) {
    Stmt* stmt = malloc(sizeof(Stmt));
    if (!stmt) error(error_messages[ERROR_MALLOCFAIL].message);

    stmt->type = STMT_PRINT;
    stmt->as.print.expr = expression;

    return stmt;
}

Stmt* stmt_var(Token name, Expr* initializer) {
    Stmt* stmt = malloc(sizeof(Stmt));
    if (!stmt) error(error_messages[ERROR_MALLOCFAIL].message);

    stmt->type = STMT_VAR;
    stmt->as.var.name = token_copy(name);
    stmt->as.var.initializer = initializer;

    return stmt;
}

Stmt* stmt_block(Stmt** statements, int count) {
    Stmt* stmt = malloc(sizeof(Stmt));
    if (!stmt) error(error_messages[ERROR_MALLOCFAIL].message);

    stmt->type = STMT_BLOCK;
    stmt->as.block.statements = statements;
    stmt->as.block.count = count;

    return stmt;
}

Stmt* stmt_if(Expr* condition, Stmt* then_branch, Stmt* else_branch) {
    Stmt* stmt = malloc(sizeof(Stmt));
    if (!stmt) error(error_messages[ERROR_MALLOCFAIL].message);

    stmt->type = STMT_IF;
    stmt->as.if_stmt.condition = condition;
    stmt->as.if_stmt.then_branch = then_branch;
    stmt->as.if_stmt.else_branch = else_branch;

    return stmt;
}

Stmt* stmt_while(Expr* condition, Stmt* body) {
    Stmt* stmt = malloc(sizeof(Stmt));
    if (!stmt) error(error_messages[ERROR_MALLOCFAIL].message);

    stmt->type = STMT_WHILE;
    stmt->as.while_stmt.condition = condition;
    stmt->as.while_stmt.body = body;

    return stmt;
}

void stmt_free(Stmt* stmt) {
    if (!stmt) return;

    switch (stmt->type) {
        case STMT_EXPRESSION:
            expr_free(stmt->as.expr.expr);
            break;
        case STMT_PRINT:
            expr_free(stmt->as.print.expr);
            break;
        case STMT_VAR:
            free(stmt->as.var.name.lexeme);
            expr_free(stmt->as.var.initializer);
            break;
        case STMT_BLOCK:
            for (int i = 0; i < stmt->as.block.count; i++) {
                stmt_free(stmt->as.block.statements[i]);
            }
            free(stmt->as.block.statements);
            break;
        case STMT_IF:
            expr_free(stmt->as.if_stmt.condition);
            stmt_free(stmt->as.if_stmt.then_branch);
            stmt_free(stmt->as.if_stmt.else_branch);
            break;
        case STMT_WHILE:
            expr_free(stmt->as.while_stmt.condition);
            stmt_free(stmt->as.while_stmt.body);
            break;
    }

    free(stmt);
}
