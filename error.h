#ifndef ERROR_H
#define ERROR_H
#include "token.h"

typedef enum {
    ERROR_USELESS,
    ERROR_MALLOCFAIL,
    ERROR_REALLOCFAIL,
    ERROR_USAGE,
    ERROR_FILELOAD,
    ERROR_ARGC
} ErrorType;

typedef struct {
    ErrorType type;
    const char* message;
} ErrorMessageMap;

static const ErrorMessageMap error_messages[] = {
    { ERROR_USELESS, "Something failed." },
    { ERROR_MALLOCFAIL, "Memory allocation failed." },
    { ERROR_REALLOCFAIL, "Memory reallocation failed." },
    { ERROR_USAGE, "Usage: whisker <file.wsk>" },
    { ERROR_FILELOAD, "File loading failed before parsing or file doesn't exist." },
    { ERROR_ARGC, "Wrong argument amount." },
};

void error(const char* message);
void error_at_line(int line, const char* message);
void error_at_token(Token token, const char* message);

#endif
