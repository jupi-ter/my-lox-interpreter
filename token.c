#include "token.h"
#include "error.h"
#include "literal.h"
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include <string.h>

char* token_type_to_string(TokenType type) {
    switch(type) {
        case TOKEN_LEFT_PAREN:
            return "(";
        case TOKEN_RIGHT_PAREN:
            return ")";
        case TOKEN_LEFT_BRACE:
            return "[";
        case TOKEN_RIGHT_BRACE:
            return "]";
        case TOKEN_COMMA:
            return ",";
        case TOKEN_DOT:
            return ".";
        case TOKEN_MINUS:
            return "-";
        case TOKEN_PLUS:
            return "+";
        case TOKEN_SEMICOLON:
            return ";";
        case TOKEN_SLASH:
            return "/";
        case TOKEN_STAR:
            return "*";
        case TOKEN_BANG:
            return "!";
        case TOKEN_BANG_EQUAL:
            return "!=";
        case TOKEN_EQUAL:
            return "=";
        case TOKEN_EQUAL_EQUAL:
            return "==";
        case TOKEN_GREATER:
            return ">";
        case TOKEN_GREATER_EQUAL:
            return ">=";
        case TOKEN_LESS:
            return "<";
        case TOKEN_LESS_EQUAL:
            return "<=";
        case TOKEN_IDENTIFIER:
            return "identifier";
        case TOKEN_STRING:
            return "string";
        case TOKEN_NUMBER:
            return "number";
        case TOKEN_AND:
            return "and";
        case TOKEN_CLASS:
            return "class";
        case TOKEN_ELSE:
            return "else";
        case TOKEN_FALSE:
            return "false";
        case TOKEN_FUN:
            return "fun";
        case TOKEN_FOR:
            return "for";
        case TOKEN_IF:
            return "if";
        case TOKEN_NIL:
            return "nil";
        case TOKEN_OR:
            return "or";
        case TOKEN_PRINT:
            return "print";
        case TOKEN_RETURN:
            return "return";
        case TOKEN_SUPER:
            return "super";
        case TOKEN_THIS:
            return "this";
        case TOKEN_TRUE:
            return "true";
        case TOKEN_VAR:
            return "var";
        case TOKEN_WHILE:
            return "while";
        case TOKEN_ENTITY:
            return "entity";
        case TOKEN_ON_CREATE:
            return "on_create";
        case TOKEN_SELF:
            return "self";
        case TOKEN_FLOAT:
            return "float";
        case TOKEN_INT:
            return "int";
        case TOKEN_BOOL:
            return "bool";
        case TOKEN_UINT32:
            return "uint32";
        case TOKEN_EOF:
            return "eof";
        case TOKEN_TRANSFORM:
            return "transform";
        case TOKEN_RENDERABLE:
            return "renderable";
        case TOKEN_COLLISION:
            return "collision";
        case TOKEN_ON_UPDATE:
            return "on_update";
        case TOKEN_ON_DESTROY:
            return "on_destroy";
    }
    return "";
}

Token token_copy(Token t) {
    t.lexeme = my_strndup(t.lexeme, strlen(t.lexeme));
    // strings in literals also need copying
    if (t.literal.type == LITERAL_STRING) {
        t.literal.as.string = my_strndup(t.literal.as.string, strlen(t.literal.as.string));
    }
    return t;
}

char* token_to_string(Token token) {
    static char buffer[256];
    sprintf(buffer, "%s %s %s", token_type_to_string(token.type), token.lexeme, literal_to_string(token.literal));

    return buffer;
}

TokenList create_token_list(int capacity) {
    TokenList tokens = {0};
    tokens.count = 0;
    tokens.capacity = capacity;
    tokens.data = malloc(sizeof(Token) * capacity);
    return tokens;
}

void add_token_list(TokenList *tokens, Token token) {
    if (tokens->count + 1 > tokens->capacity) {
        tokens->capacity *= 2;
        Token* new_data = realloc(tokens->data, sizeof(Token) * tokens->capacity);

        if (!new_data) {
            error(error_messages[ERROR_REALLOCFAIL].message);
        }

        tokens->data = new_data;
    }

    tokens->data[tokens->count++] = token;
}

void free_token_list(TokenList *tokens) {
    for (int i = 0; i < tokens->count; i++) {
        Token* token = &tokens->data[i];
        free(token->lexeme);
        if (token->literal.type == LITERAL_STRING) {
            free(token->literal.as.string);
        }
    }
    free(tokens->data);
    tokens->data = NULL;
    tokens->count = 0;
    tokens->capacity = 0;
}
