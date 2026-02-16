#ifndef ENTITY_AST_H
#define ENTITY_AST_H

#include "token.h"
#include "stmt.h"

typedef enum {
    TYPE_FLOAT,
    TYPE_INT,
    TYPE_BOOL,
    TYPE_UINT32
} FieldType;

typedef struct {
    Token name;
    FieldType type;
} EntityField;

typedef struct {
    Token name;              // entity name
    EntityField* fields;     // array of fields
    int field_count;
    Stmt* on_create;         // on_create block - nullable.
    Stmt* on_update;
} EntityDecl;

typedef struct {
    EntityDecl* entities;
    int count;
} EntityList;

EntityDecl* entity_decl_create(Token name, EntityField* fields, int field_count, Stmt* on_create, Stmt* on_update);
void entity_decl_free(EntityDecl* entity);

#endif
