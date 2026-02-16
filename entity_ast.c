#include "entity_ast.h"
#include "error.h"
#include <stdlib.h>

EntityDecl* entity_decl_create(Token name, EntityField* fields, int field_count, 
                                Stmt* on_create, Stmt* on_update) {
    EntityDecl* entity = malloc(sizeof(EntityDecl));
    if (!entity) error(error_messages[ERROR_MALLOCFAIL].message);
    
    entity->name = name;
    entity->fields = fields;
    entity->field_count = field_count;
    entity->on_create = on_create;
    entity->on_update = on_update;  // Add this
    
    return entity;
}

void entity_decl_free(EntityDecl* entity) {
    if (!entity) return;
    
    free(entity->fields);
    stmt_free(entity->on_create);
    stmt_free(entity->on_update);  // Add this
    free(entity);
}
