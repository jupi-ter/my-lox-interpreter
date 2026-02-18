#ifndef TOKEN_H
#define TOKEN_H

#include "literal.h"

typedef enum {
    // Single-character tokens.
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN, TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS, TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,

    // One or two character tokens.
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,

    // Literals.
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

    // Keywords.
    TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE, TOKEN_FUN, TOKEN_FOR, TOKEN_IF, TOKEN_NIL, TOKEN_OR,
    TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS, TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE, TOKEN_GAME, TOKEN_SPAWN,

    // Entity keywords.
    TOKEN_ENTITY, TOKEN_ON_CREATE, TOKEN_ON_UPDATE, TOKEN_ON_DESTROY, TOKEN_SELF, TOKEN_FLOAT, TOKEN_INT,
    TOKEN_BOOL, TOKEN_UINT32,

    // Engine component keywords.
    TOKEN_TRANSFORM, TOKEN_RENDERABLE, TOKEN_COLLISION,

    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    int line;
    char *lexeme;
    Literal literal;
} Token;

typedef struct {
    Token* data;
    int count;
    int capacity;
} TokenList;

typedef struct {
    const char* keyword;
    TokenType type;
} KeywordMap;

#define KEYWORD_COUNT 30

static const KeywordMap keywords[] = {
    {"and" , TOKEN_AND},
    {"class" , TOKEN_CLASS},
    {"else" , TOKEN_ELSE},
    {"false" , TOKEN_FALSE},
    {"fun" , TOKEN_FUN},
    {"for" , TOKEN_FOR},
    {"if" , TOKEN_IF},
    {"nil" , TOKEN_NIL},
    {"or" , TOKEN_OR},
    {"print" , TOKEN_PRINT},
    {"return" , TOKEN_RETURN},
    {"super" , TOKEN_SUPER},
    {"this" , TOKEN_THIS},
    {"true" , TOKEN_TRUE},
    {"var" , TOKEN_VAR},
    {"while" , TOKEN_WHILE},
    {"entity", TOKEN_ENTITY},
    {"on_create", TOKEN_ON_CREATE},
    {"on_update", TOKEN_ON_UPDATE},
    {"on_destroy", TOKEN_ON_DESTROY},
    {"self", TOKEN_SELF},
    {"float", TOKEN_FLOAT},
    {"int", TOKEN_INT},
    {"bool", TOKEN_BOOL},
    {"uint32", TOKEN_UINT32},
    {"transform", TOKEN_TRANSFORM},
    {"renderable", TOKEN_RENDERABLE},
    {"collision", TOKEN_COLLISION},
    {"game", TOKEN_GAME},
    {"spawn", TOKEN_SPAWN}
};

//helpers
char* token_to_string(Token token);
Token token_copy(Token t);
char* token_type_to_string(TokenType type);

//list managers
TokenList create_token_list(int capacity);
void add_token_list(TokenList *tokens, Token token);
void free_token_list(TokenList *tokens);

#endif
