#include "parser.h"
#include "error.h"
#include <stdbool.h>

Parser parser_create(TokenList tokens) {
    Parser parser = {
        .tokens = tokens.data,
        .current = 0,
        .count = tokens.count
    };

    return parser;
}

// ========= Parser utils ===========
static Token peek(Parser* parser) {
    return parser->tokens[parser->current];
}

static Token previous(Parser* parser) {
    return parser->tokens[parser->current - 1];
}

static bool is_at_end(Parser* parser) {
    return peek(parser).type == TOKEN_EOF;
}

static Token advance(Parser* parser) {
    if (!is_at_end(parser)) parser->current++;
    return previous(parser);
}

static bool check(Parser* parser, TokenType type) {
    if (is_at_end(parser)) return false;
    return peek(parser).type == type;
}

static bool match(Parser* parser, TokenType type) {
    if (check(parser, type)) {
        advance(parser);
        return true;
    }
    return false;
}

static bool match_any(Parser* parser, TokenType* types, int count) {
    for (int i = 0; i < count; i++) {
        if (check(parser, types[i])) {
            advance(parser);
            return true;
        }
    }
    return false;
}

static Token consume(Parser* parser, TokenType type, const char* message) {
    if (check(parser, type)) return advance(parser);
    error_at_token(peek(parser), message);
    return peek(parser); // unreachable
}

// ========= Grammar Rules ==========
static Expr* expression(Parser* parser);
static Expr* assignment(Parser* parser);
static Expr* logic_or(Parser* parser);
static Expr* logic_and(Parser* parser);
static Expr* equality(Parser* parser);
static Expr* comparison(Parser* parser);
static Expr* term(Parser* parser);
static Expr* factor(Parser* parser);
static Expr* unary(Parser* parser);
static Expr* primary(Parser* parser);

static Expr* primary(Parser* parser) {
    if (match(parser, TOKEN_FALSE)) {
        Literal lit = { .type = LITERAL_BOOLEAN, .as.boolean = false };
        return expr_literal(lit);
    }
    if (match(parser, TOKEN_TRUE)) {
        Literal lit = { .type = LITERAL_BOOLEAN, .as.boolean = true };
        return expr_literal(lit);
    }
    if (match(parser, TOKEN_NIL)) {
        Literal lit = { .type = LITERAL_NONE };
        return expr_literal(lit);
    }
    
    if (match(parser, TOKEN_NUMBER)) {
        return expr_literal(previous(parser).literal);
    }
    if (match(parser, TOKEN_STRING)) {
        return expr_literal(previous(parser).literal);
    }
    
    if (match(parser, TOKEN_IDENTIFIER)) {
        return expr_variable(previous(parser));
    }
    
    if (match(parser, TOKEN_LEFT_PAREN)) {
        Expr* expr = expression(parser);
        consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
        return expr_grouping(expr);
    }
    
    error_at_token(peek(parser), "Expect expression.");
    
    //unreachable
    Expr* null_expr;
    return null_expr;
}

static Expr* unary(Parser* parser) {
    TokenType unary_ops[] = {TOKEN_BANG, TOKEN_MINUS};
    if (match_any(parser, unary_ops, 2)) {
        Token operator = previous(parser);
        Expr* right = unary(parser);
        return expr_unary(operator, right);
    }
    
    return primary(parser);
}

static Expr* factor(Parser* parser) {
    Expr* expr = unary(parser);
    
    TokenType factor_ops[] = {TOKEN_SLASH, TOKEN_STAR};
    while (match_any(parser, factor_ops, 2)) {
        Token operator = previous(parser);
        Expr* right = unary(parser);
        expr = expr_binary(expr, operator, right);
    }
    
    return expr;
}

static Expr* term(Parser* parser) {
    Expr* expr = factor(parser);
    
    TokenType term_ops[] = {TOKEN_MINUS, TOKEN_PLUS};
    while (match_any(parser, term_ops, 2)) {
        Token operator = previous(parser);
        Expr* right = factor(parser);
        expr = expr_binary(expr, operator, right);
    }
    
    return expr;
}

static Expr* comparison(Parser* parser) {
    Expr* expr = term(parser);
    
    TokenType comp_ops[] = {TOKEN_GREATER, TOKEN_GREATER_EQUAL, 
                            TOKEN_LESS, TOKEN_LESS_EQUAL};
    while (match_any(parser, comp_ops, 4)) {
        Token operator = previous(parser);
        Expr* right = term(parser);
        expr = expr_binary(expr, operator, right);
    }
    
    return expr;
}

static Expr* equality(Parser* parser) {
    Expr* expr = comparison(parser);
    
    TokenType eq_ops[] = {TOKEN_BANG_EQUAL, TOKEN_EQUAL_EQUAL};
    while (match_any(parser, eq_ops, 2)) {
        Token operator = previous(parser);
        Expr* right = comparison(parser);
        expr = expr_binary(expr, operator, right);
    }
    
    return expr;
}

static Expr* logic_and(Parser* parser) {
    Expr* expr = equality(parser);
    
    while (match(parser, TOKEN_AND)) {
        Token operator = previous(parser);
        Expr* right = equality(parser);
        expr = expr_binary(expr, operator, right);
    }
    
    return expr;
}

static Expr* logic_or(Parser* parser) {
    Expr* expr = logic_and(parser);
    
    while (match(parser, TOKEN_OR)) {
        Token operator = previous(parser);
        Expr* right = logic_and(parser);
        expr = expr_binary(expr, operator, right);
    }
    
    return expr;
}

static Expr* assignment(Parser* parser) {
    Expr* expr = logic_or(parser);
    
    if (match(parser, TOKEN_EQUAL)) {
        Token equals = previous(parser);
        Expr* value = assignment(parser);
        
        if (expr->type == EXPR_VARIABLE) {
            Token name = expr->as.variable.name;
            expr_free(expr);
            return expr_assign(name, value);
        }
        
        error_at_token(equals, "Invalid assignment target.");
    }
    
    return expr;
}

static Expr* expression(Parser* parser) {
    return assignment(parser);
}

Expr* parse(Parser* parser) {
    return expression(parser);
}