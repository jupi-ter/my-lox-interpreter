#include "parser.h"
#include "entity_ast.h"
#include "error.h"
#include "game_ast.h"
#include "token.h"
#include <stdbool.h>
#include <stdlib.h>

Parser parser_create(TokenList tokens) {
    Parser parser = {
        .tokens = tokens.data,
        .current = 0,
        .count = tokens.count
    };

    return parser;
}

static EntityDecl* entity_declaration(Parser* parser);

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
static Expr* call(Parser* parser);
static Expr* unary(Parser* parser);
static Expr* primary(Parser* parser);

// ========= Statement Grammar ==========
static Stmt* declaration(Parser* parser);
static Stmt* statement(Parser* parser);
static Stmt* print_statement(Parser* parser);
static Stmt* expression_statement(Parser* parser);
static Stmt* var_declaration(Parser* parser);
static Stmt* block_statement(Parser* parser);
static Stmt* if_statement(Parser* parser);
static Stmt* while_statement(Parser* parser);
static Stmt* for_statement(Parser* parser);

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

    if (match(parser, TOKEN_SELF)) {
        return expr_variable(previous(parser));
    }

    if (match(parser, TOKEN_TRANSFORM)) {
        return expr_variable(previous(parser));
    }

    if (match(parser, TOKEN_RENDERABLE)) {
        return expr_variable(previous(parser));
    }

    if (match(parser, TOKEN_COLLISION)) {
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

static Expr* call(Parser* parser) {
    Expr* expr = primary(parser);

    while (true) {
        if (match(parser, TOKEN_DOT)) {
            Token name = consume(parser, TOKEN_IDENTIFIER, "Expect property name after '.'.");
            expr = expr_get(expr, name);
        } else if (match(parser, TOKEN_LEFT_PAREN)) {
            // Function call!
            // For now, just parse arguments and wrap in a special call expr

            // Parse arguments
            int arg_count = 0;
            Expr** arguments = NULL;

            if (!check(parser, TOKEN_RIGHT_PAREN)) {
                int capacity = 4;
                arguments = malloc(sizeof(Expr*) * capacity);

                do {
                    if (arg_count >= capacity) {
                        capacity *= 2;
                        arguments = realloc(arguments, sizeof(Expr*) * capacity);
                    }
                    arguments[arg_count++] = expression(parser);
                } while (match(parser, TOKEN_COMMA));
            }

            consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");

            // Create a call expression
            expr = expr_call(expr, arg_count, arguments);
        } else {
            break;
        }
    }

    return expr;
}

static Expr* unary(Parser* parser) {
    TokenType unary_ops[] = {TOKEN_BANG, TOKEN_MINUS};
    if (match_any(parser, unary_ops, 2)) {
        Token operator = previous(parser);
        Expr* right = unary(parser);
        return expr_unary(operator, right);
    }

    return call(parser);
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
        } else if (expr->type == EXPR_GET) {
            // convert get to set: self.hsp = 5
            return expr_set(expr->as.get.object, expr->as.get.name, value);
        }

        error_at_token(equals, "Invalid assignment target.");
    }

    return expr;
}

static Expr* expression(Parser* parser) {
    return assignment(parser);
}

static Stmt* block_statement(Parser* parser) {
    int capacity = 8;
    int count = 0;
    Stmt** statements = malloc(sizeof(Stmt*) * capacity);
    if (!statements) error(error_messages[ERROR_MALLOCFAIL].message);

    while (!check(parser, TOKEN_RIGHT_BRACE) && !is_at_end(parser)) {
        if (count >= capacity) {
            capacity *= 2;
            Stmt** new_stmts = realloc(statements, sizeof(Stmt*) * capacity);
            if (!new_stmts) {
                // Cleanup on failure
                for (int i = 0; i < count; i++) stmt_free(statements[i]);
                free(statements);
                error(error_messages[ERROR_REALLOCFAIL].message);
            }
            statements = new_stmts;
        }

        statements[count++] = declaration(parser);
    }

    consume(parser, TOKEN_RIGHT_BRACE, "Expect '}' after block.");
    return stmt_block(statements, count);
}

static Stmt* if_statement(Parser* parser) {
    consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    Expr* condition = expression(parser);
    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after if condition.");

    Stmt* then_branch = statement(parser);
    Stmt* else_branch = NULL;

    if (match(parser, TOKEN_ELSE)) {
        else_branch = statement(parser);
    }

    return stmt_if(condition, then_branch, else_branch);
}

static Stmt* while_statement(Parser* parser) {
    consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    Expr* condition = expression(parser);
    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after while condition.");

    Stmt* body = statement(parser);

    return stmt_while(condition, body);
}

static Stmt* for_statement(Parser* parser) {
    consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");

    // Initializer: var i = 0; OR i = 0; OR nothing
    Stmt* initializer = NULL;
    if (match(parser, TOKEN_SEMICOLON)) {
        initializer = NULL;  // No initializer
    } else if (match(parser, TOKEN_VAR)) {
        initializer = var_declaration(parser);
    } else {
        initializer = expression_statement(parser);
    }

    // Condition: i < 10
    Expr* condition = NULL;
    if (!check(parser, TOKEN_SEMICOLON)) {
        condition = expression(parser);
    }
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after for condition.");

    // Increment: i = i + 1
    Expr* increment = NULL;
    if (!check(parser, TOKEN_RIGHT_PAREN)) {
        increment = expression(parser);
    }
    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

    Stmt* body = statement(parser);

    // Desugar: attach increment to end of body
    if (increment) {
        Stmt* inc_stmt = stmt_expression(increment);
        Stmt** stmts = malloc(sizeof(Stmt*) * 2);
        stmts[0] = body;
        stmts[1] = inc_stmt;
        body = stmt_block(stmts, 2);
    }

    // Desugar: wrap in while
    if (!condition) {
        // No condition means infinite loop: while (true)
        Literal lit = { .type = LITERAL_BOOLEAN, .as.boolean = true };
        condition = expr_literal(lit);
    }
    body = stmt_while(condition, body);

    // Desugar: prepend initializer
    if (initializer) {
        Stmt** stmts = malloc(sizeof(Stmt*) * 2);
        stmts[0] = initializer;
        stmts[1] = body;
        body = stmt_block(stmts, 2);
    }

    return body;
}

static Stmt* var_declaration(Parser* parser) {
    Token name = consume(parser, TOKEN_IDENTIFIER, "Expect variable name.");

    Expr* initializer = NULL;
    if (match(parser, TOKEN_EQUAL)) {
        initializer = expression(parser);
    }

    consume(parser, TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    return stmt_var(name, initializer);
}

static Stmt* print_statement(Parser* parser) {
    Expr* value = expression(parser);
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after value.");
    return stmt_print(value);
}

static Stmt* expression_statement(Parser* parser) {
    Expr* expr = expression(parser);
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after expression.");
    return stmt_expression(expr);
}

static Stmt* statement(Parser* parser) {
    if (match(parser, TOKEN_PRINT)) return print_statement(parser);
    if (match(parser, TOKEN_IF)) return if_statement(parser);
    if (match(parser, TOKEN_WHILE)) return while_statement(parser);
    if (match(parser, TOKEN_FOR)) return for_statement(parser);
    if (match(parser, TOKEN_LEFT_BRACE)) return block_statement(parser);

    return expression_statement(parser);
}

static Stmt* declaration(Parser* parser) {
    if (match(parser, TOKEN_VAR)) return var_declaration(parser);
    return statement(parser);
}

static FieldType parse_field_type(Parser* parser) {
    if (match(parser, TOKEN_FLOAT)) return TYPE_FLOAT;
    if (match(parser, TOKEN_INT)) return TYPE_INT;
    if (match(parser, TOKEN_BOOL)) return TYPE_BOOL;
    if (match(parser, TOKEN_UINT32)) return TYPE_UINT32;

    error_at_token(peek(parser), "Expected type (float, int, bool, uint32)");
    return TYPE_FLOAT; // unreachable
}

static EntityDecl* entity_declaration(Parser* parser) {
    Token name = consume(parser, TOKEN_IDENTIFIER, "Expect entity name.");
    consume(parser, TOKEN_LEFT_BRACE, "Expect '{' after entity name.");

    // Parse fields
    int field_capacity = 8;
    int field_count = 0;
    EntityField* fields = malloc(sizeof(EntityField) * field_capacity);
    if (!fields) error(error_messages[ERROR_MALLOCFAIL].message);

    while (!check(parser, TOKEN_RIGHT_BRACE) &&
            !check(parser, TOKEN_ON_CREATE) &&
            !check(parser, TOKEN_ON_UPDATE) &&
            !check(parser, TOKEN_ON_DESTROY) &&
            !is_at_end(parser)) {
        if (field_count >= field_capacity) {
            field_capacity *= 2;
            EntityField* new_fields = realloc(fields, sizeof(EntityField) * field_capacity);
            if (!new_fields) {
                free(fields);
                error(error_messages[ERROR_REALLOCFAIL].message);
            }
            fields = new_fields;
        }

        FieldType type = parse_field_type(parser);
        Token field_name = consume(parser, TOKEN_IDENTIFIER, "Expect field name.");
        consume(parser, TOKEN_SEMICOLON, "Expect ';' after field declaration.");

        fields[field_count++] = (EntityField){ .name = token_copy(field_name), .type = type };
    }

    // parse lifecycle blocks
    Stmt* on_create = NULL;
    Stmt* on_update = NULL;
    Stmt* on_collision = NULL;
    Stmt* on_destroy = NULL;
    Token collision_param = {0};

    // in any order
    while (!check(parser, TOKEN_RIGHT_BRACE)) {
        if (match(parser, TOKEN_ON_CREATE)) {
            consume(parser, TOKEN_LEFT_BRACE, "Expect '{' after on_create.");
            on_create = block_statement(parser);
        } else if (match(parser, TOKEN_ON_UPDATE)) {
            consume(parser, TOKEN_LEFT_BRACE, "Expect '{' after on_update.");
            on_update = block_statement(parser);
        } else if (match(parser, TOKEN_ON_DESTROY)) {
            consume(parser, TOKEN_LEFT_BRACE, "Expect '{' after on_destroy.");
            on_destroy = block_statement(parser);
        } else if (match(parser, TOKEN_ON_COLLISION)) {
            consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after on_collision.");
            collision_param = consume(parser, TOKEN_IDENTIFIER, "Expect parameter name.");
            consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after parameter.");
            consume(parser, TOKEN_LEFT_BRACE, "Expect '{' after on_collision.");
            on_collision = block_statement(parser);
        } else {
            error_at_token(peek(parser), "Expect on_create, on_update, or on_destroy.");
        }
    }

    consume(parser, TOKEN_RIGHT_BRACE, "Expect '}' after entity body.");

    return entity_decl_create(token_copy(name), fields, field_count, on_create, on_update, on_destroy, on_collision, collision_param);
}

static GameDecl* game_declaration(Parser* parser) {
    consume(parser, TOKEN_LEFT_BRACE, "Expect '{' after 'game'.");

    int capacity = 8;
    int count = 0;
    SpawnCall* spawns = malloc(sizeof(SpawnCall) * capacity);
    if (!spawns) error(error_messages[ERROR_MALLOCFAIL].message);

    while (!check(parser, TOKEN_RIGHT_BRACE) && !is_at_end(parser)) {
        consume(parser, TOKEN_SPAWN, "Expect 'spawn' in game block.");

        Token entity_name = consume(parser, TOKEN_IDENTIFIER, "Expect entity name after 'spawn'.");
        consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after entity name.");

        // Parse x coordinate
        Token x_token = consume(parser, TOKEN_NUMBER, "Expect x coordinate.");
        consume(parser, TOKEN_COMMA, "Expect ',' after x coordinate.");

        // Parse y coordinate
        Token y_token = consume(parser, TOKEN_NUMBER, "Expect y coordinate.");
        consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after coordinates.");
        consume(parser, TOKEN_SEMICOLON, "Expect ';' after spawn call.");

        if (count >= capacity) {
            capacity *= 2;
            SpawnCall* new_spawns = realloc(spawns, sizeof(SpawnCall) * capacity);
            if (!new_spawns) {
                free(spawns);
                error(error_messages[ERROR_REALLOCFAIL].message);
            }
            spawns = new_spawns;
        }

        spawns[count++] = (SpawnCall){
            .entity_name = entity_name,
            .x = (float)x_token.literal.as.number,
            .y = (float)y_token.literal.as.number
        };
    }

    consume(parser, TOKEN_RIGHT_BRACE, "Expect '}' after game block.");
    return game_decl_create(spawns, count);
}

Program parse(Parser* parser) {
    GameDecl* game = NULL;
    int stmt_capacity = 8;
    int stmt_count = 0;
    Stmt** statements = malloc(sizeof(Stmt*) * stmt_capacity);
    if (!statements) error(error_messages[ERROR_MALLOCFAIL].message);

    int entity_capacity = 8;
    int entity_count = 0;
    EntityDecl** entities = malloc(sizeof(EntityDecl*) * entity_capacity);
    if (!entities) {
        free(statements);
        error(error_messages[ERROR_MALLOCFAIL].message);
    }

    while (!is_at_end(parser)) {
        // Check if it's an entity declaration
        if (match(parser, TOKEN_ENTITY)) {
            if (entity_count >= entity_capacity) {
                entity_capacity *= 2;
                EntityDecl** new_entities = realloc(entities, sizeof(EntityDecl*) * entity_capacity);
                if (!new_entities) {
                    free(statements);
                    free(entities);
                    error(error_messages[ERROR_REALLOCFAIL].message);
                }
                entities = new_entities;
            }
            entities[entity_count++] = entity_declaration(parser);
        } else if (match(parser, TOKEN_GAME)) {
            if (game) error_at_token(peek(parser), "Only one 'game' block allowed.");
            game = game_declaration(parser);
        } else {
            // Regular statement
            if (stmt_count >= stmt_capacity) {
                stmt_capacity *= 2;
                Stmt** new_stmts = realloc(statements, sizeof(Stmt*) * stmt_capacity);
                if (!new_stmts) {
                    free(statements);
                    free(entities);
                    error(error_messages[ERROR_REALLOCFAIL].message);
                }
                statements = new_stmts;
            }
            statements[stmt_count++] = declaration(parser);
        }
    }

    Program prog = {
        .statements = statements,
        .count = stmt_count,
        .entities = entities,
        .entity_count = entity_count,
        .game = game //GAME IS GAME
    };
    return prog;
}

void free_program(Program* prog) {
    game_decl_free(prog->game);
    for (int i = 0; i < prog->count; i++) {
        stmt_free(prog->statements[i]);
    }
    free(prog->statements);

    for (int i = 0; i < prog->entity_count; i++) {
        entity_decl_free(prog->entities[i]);
    }
    free(prog->entities);
}
