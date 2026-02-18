#include "scanner.h"
#include "error.h"
#include "literal.h"
#include "token.h"
#include "utils.h"
#include <string.h>

static bool is_at_end(Scanner* scanner) {
    return scanner->source[scanner->current] == '\0';
}

static void add_token_literal(Scanner* scanner, TokenType type, Literal literal) {
    int length = scanner->current - scanner->start;
    char* text = my_strndup(scanner->source + scanner->start, length);

    if (!text) {
        error_at_line(scanner->line, "Memory allocation failed.");
        return;
    }

    Token token = {
        .type = type,
        .lexeme = text,
        .line = scanner->line,
        .literal = literal
    };

    add_token_list(&scanner->tokens, token);
}

static void add_token(Scanner* scanner, TokenType type) {
    add_token_literal(scanner, type, (Literal){ .type = LITERAL_NONE });
}

Scanner scanner_create(char* source) {
    TokenList tokens = create_token_list(16);

    Scanner scanner = {
        .source = source,
        .start = 0,
        .current = 0,
        .line = 1,
        .tokens = tokens
    };
    return scanner;
}

char advance(Scanner* scanner) {
    return scanner->source[scanner->current++];
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_alphanumeric(char c) {
    return is_alpha(c) || is_digit(c);
}

static bool match(Scanner *scanner, char expected) {
  if (is_at_end(scanner)) return false;
  if (scanner->source[scanner->current] != expected) return false;

  scanner->current++;
  return true;
}

static char peek(Scanner* scanner) {
  if (is_at_end(scanner)) return '\0';
  return scanner->source[scanner->current];
}

static TokenType get_keyword(const char* text) {
    for (int i =0; i < KEYWORD_COUNT; i++) {
        if (strcmp(text, keywords[i].keyword) == 0) {
            return keywords[i].type;
        }
    }

    return TOKEN_IDENTIFIER; //not a keyword
}

static void identifier(Scanner* scanner) {
    while (is_alphanumeric(peek(scanner))) advance(scanner);

    char* text = my_strndup(&scanner->source[scanner->start], scanner->current - scanner->start);
    TokenType type = get_keyword(text);

    add_token(scanner, type);
}

static char peek_next(Scanner* scanner) {
    if (scanner->source[scanner->current + 1] == '\0') return '\0';
    return scanner->source[scanner->current + 1];
}

static void number(Scanner* scanner) {
    while (is_digit(peek(scanner))) advance(scanner);

    // Look for a fractional part.
    if (peek(scanner) == '.' && is_digit(peek_next(scanner))) {
        // Consume the "."
        advance(scanner);

        while (is_digit(peek(scanner))) advance(scanner);
    }

    // Extract substring and parse to double
    int length = scanner->current - scanner->start;
    char* num_str = my_strndup(scanner->source + scanner->start, length);

    double num = strtod(num_str, NULL);
    free(num_str);

    Literal literal = {
        .type = LITERAL_NUMBER,
        .as.number = num
    };

    add_token_literal(scanner, TOKEN_NUMBER, literal);
}

static void string(Scanner* scanner) {
    while (peek(scanner) != '"' && !is_at_end(scanner)) {
    if (peek(scanner) == '\n') scanner->line++;
        advance(scanner);
    }

    if (is_at_end(scanner)) {
        error_at_line(scanner->line, "Unterminated string.");
    }

    // The closing ".
    advance(scanner);

    // Trim the surrounding quotes.
    char* value = my_strndup(scanner->source + scanner->start + 1, scanner->current - scanner->start - 2);

    Literal literal = {
        .type = LITERAL_STRING,
        .as.string = value
    };
    add_token_literal(scanner, TOKEN_STRING, literal);
}

void scan_token(Scanner* scanner) {
    char c = advance(scanner);

    switch(c) {
        case '(': add_token(scanner, TOKEN_LEFT_PAREN); break;
        case ')': add_token(scanner, TOKEN_RIGHT_PAREN); break;
        case '{': add_token(scanner, TOKEN_LEFT_BRACE); break;
        case '}': add_token(scanner, TOKEN_RIGHT_BRACE); break;
        case ',': add_token(scanner, TOKEN_COMMA); break;
        case '.': add_token(scanner, TOKEN_DOT); break;
        case '-': add_token(scanner, TOKEN_MINUS); break;
        case '+': add_token(scanner, TOKEN_PLUS); break;
        case ';': add_token(scanner, TOKEN_SEMICOLON); break;
        case '*': add_token(scanner, TOKEN_STAR); break;
        case '!': add_token(scanner, match(scanner, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG); break;
        case '=': add_token(scanner, match(scanner, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL); break;
        case '<': add_token(scanner, match(scanner, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS); break;
        case '>': add_token(scanner, match(scanner, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER); break;
        case '/':
            if (match(scanner ,'/')) {
                // A comment goes until the end of the line.
                while (peek(scanner) != '\n' && !is_at_end(scanner)) advance(scanner);
            } else {
                add_token(scanner, TOKEN_SLASH);
            }
            break;
        case '"': string(scanner); break;

        case ' ':
        case '\r':
        case '\t':
            // Ignore whitespace.
            break;

        case '\n':
            scanner->line++;
            break;
        default:
            if (is_digit(c)) {
                number(scanner);
            } else if (is_alpha(c)) {
                identifier(scanner);
            } else {
                error_at_line(scanner->line, "Unexpected character.");
            }
          break;
    }
}

TokenList scan_tokens(Scanner* scanner) {
    while (!is_at_end(scanner)) { //while not at eof
        scanner->start = scanner->current;
        scan_token(scanner);
    }

    Token token = {
      .type = TOKEN_EOF,
      .lexeme = my_strndup("", 0),
      .line = scanner->line
    };
    add_token_list(&scanner->tokens, token);

    return scanner->tokens;
}
