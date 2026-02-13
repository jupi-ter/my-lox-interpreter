#include "expr.h"
#include "error.h"
#include <stdlib.h>

Expr* expr_binary(Expr* left, Token oprt, Expr* right) {
    Expr* expr = malloc(sizeof(Expr));
    if (!expr) error(error_messages[ERROR_MALLOCFAIL].message);

    expr->type = EXPR_BINARY;
    expr->as.binary.left = left;
    expr->as.binary.oprt = oprt;
    expr->as.binary.right = right;

    return expr;
}

Expr* expr_unary(Token oprt, Expr* right) {
    Expr* expr = malloc(sizeof(Expr));
    if (!expr) error(error_messages[ERROR_MALLOCFAIL].message);

    expr->type = EXPR_UNARY;
    expr->as.unary.oprt = oprt;
    expr->as.unary.right = right;

    return expr;
}

Expr* expr_literal(Literal value) {
    Expr* expr = malloc(sizeof(Expr));
    if (!expr) error(error_messages[ERROR_MALLOCFAIL].message);

    expr->type = EXPR_LITERAL;
    expr->as.literal.value = value;
    
    return expr;
}

Expr* expr_grouping(Expr* expression) {
    Expr* expr = malloc(sizeof(Expr));
    if (!expr) error(error_messages[ERROR_MALLOCFAIL].message);

    expr->type = EXPR_GROUPING;
    expr->as.grouping.expression = expression;

    return expr;
}

Expr* expr_variable(Token name) {
    Expr* expr = malloc(sizeof(Expr));
    if (!expr) error(error_messages[ERROR_MALLOCFAIL].message);

    expr->type = EXPR_VARIABLE;
    expr->as.variable.name = name;

    return expr;
}

Expr* expr_assign(Token name, Expr* value) {
    Expr* expr = malloc(sizeof(Expr));
    if (!expr) error(error_messages[ERROR_MALLOCFAIL].message);

    expr->type = EXPR_ASSIGN;
    expr->as.assign.name = name;
    expr->as.assign.value = value;

    return expr;
}

void expr_free(Expr *expr) {
    if (!expr) return;

    switch(expr->type) {
        case EXPR_BINARY:
            expr_free(expr->as.binary.left);
            expr_free(expr->as.binary.right);
            break;
        case EXPR_UNARY:
            expr_free(expr->as.unary.right);
            break;
        case EXPR_GROUPING:
            expr_free(expr->as.grouping.expression);
            break;
        case EXPR_ASSIGN:
            expr_free(expr->as.assign.value);
            break;
        case EXPR_LITERAL:
        case EXPR_VARIABLE:
            break;
    }

    free(expr);
}