#include "codegen.h"
#include "error.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_CAPACITY 4096

// Forward declarations
static void generate_expr(CodeGen* gen, Expr* expr, const char* entity_name);
static void generate_stmt(CodeGen* gen, Stmt* stmt, const char* entity_name);

CodeGen codegen_create(void) {
    CodeGen gen = {0};
    gen.header_capacity = INITIAL_CAPACITY;
    gen.header_output = malloc(gen.header_capacity);
    gen.header_output[0] = '\0';

    gen.source_capacity = INITIAL_CAPACITY;
    gen.source_output = malloc(gen.source_capacity);
    gen.source_output[0] = '\0';

    gen.indent_level = 0;
    return gen;
}

void codegen_free(CodeGen* gen) {
    free(gen->header_output);
    free(gen->source_output);
    gen->header_output = NULL;
    gen->source_output = NULL;
}

//char* codegen_get_output(CodeGen* gen) {
//    return gen->output;
//}

static void append_h(CodeGen* gen, const char* str) {
    int len = strlen(str);
    while (gen->header_length + len + 1 >= gen->header_capacity) {
        gen->header_capacity *= 2;
        char* new_output = realloc(gen->header_output, gen->header_capacity);
        if (!new_output) error(error_messages[ERROR_REALLOCFAIL].message);
        gen->header_output = new_output;
    }
    strcpy(gen->header_output + gen->header_length, str);
    gen->header_length += len;
}

static void append_indent_h(CodeGen* gen) {
    for (int i = 0; i < gen->indent_level; i++) {
        append_h(gen, "    ");
    }
}

static void appendf_h(CodeGen* gen, const char* fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    append_h(gen, buffer);
}

// Append to source
static void append(CodeGen* gen, const char* str) {
    int len = strlen(str);
    while (gen->source_length + len + 1 >= gen->source_capacity) {
        gen->source_capacity *= 2;
        char* new_output = realloc(gen->source_output, gen->source_capacity);
        if (!new_output) error(error_messages[ERROR_REALLOCFAIL].message);
        gen->source_output = new_output;
    }
    strcpy(gen->source_output + gen->source_length, str);
    gen->source_length += len;
}

static void appendf(CodeGen* gen, const char* fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    append(gen, buffer);
}

// Helper for indentation
static void append_indent(CodeGen* gen) {
    for (int i = 0; i < gen->indent_level; i++) {
        append(gen, "    ");
    }
}

// Convert FieldType to C type string
static const char* field_type_to_c(FieldType type) {
    switch (type) {
        case TYPE_FLOAT: return "float";
        case TYPE_INT: return "int";
        case TYPE_BOOL: return "bool";
        case TYPE_UINT32: return "uint32_t";
    }
    return "int";
}

// Generate entity struct
static void generate_entity_struct_h(CodeGen* gen, EntityDecl* entity) {
    appendf_h(gen, "typedef struct %s {\n", entity->name.lexeme);
    gen->indent_level++;

    append_indent(gen);
    append_h(gen, "uint32_t entity_id;\n");

    for (int i = 0; i < entity->field_count; i++) {
        append_indent(gen);
        appendf_h(gen, "%s %s;\n",
                field_type_to_c(entity->fields[i].type),
                entity->fields[i].name.lexeme);
    }

    gen->indent_level--;
    appendf_h(gen, "} %s;\n\n", entity->name.lexeme);
}

// Generate entity array
static void generate_entity_array_h(CodeGen* gen, EntityDecl* entity) {
    appendf_h(gen, "typedef struct %sArray {\n", entity->name.lexeme);
    gen->indent_level++;

    append_indent(gen);
    appendf_h(gen, "%s* data;\n", entity->name.lexeme);
    append_indent(gen);
    append_h(gen, "int count;\n");
    append_indent(gen);
    append_h(gen, "int capacity;\n");

    gen->indent_level--;
    appendf_h(gen, "} %sArray;\n\n", entity->name.lexeme);
}

// Generate GameState
static void generate_game_state_h(CodeGen* gen, Program* program) {
    append_h(gen, "typedef struct GameState {\n");
    gen->indent_level++;

    // Engine components
    append_indent(gen);
    append_h(gen, "// Engine components\n");
    append_indent(gen);
    append_h(gen, "EntityRegistry registry;\n");
    append_indent(gen);
    append_h(gen, "TransformArray transforms;\n");
    append_indent(gen);
    append_h(gen, "RenderableArray renderables;\n");
    append_indent(gen);
    append_h(gen, "CircleArray circles;\n");
    append_indent(gen);
    append_h(gen, "RectangleArray rectangles;\n");
    append_indent(gen);
    append_h(gen, "TimerArray timers;\n");
    append_h(gen, "\n");

    // Entity arrays
    append_indent(gen);
    // Game entity arrays
    for (int i = 0; i < program->entity_count; i++) {
        char lower_name[256];
        snprintf(lower_name, sizeof(lower_name), "%s", program->entities[i]->name.lexeme);
        for (int j = 0; lower_name[j]; j++) {
            if (lower_name[j] >= 'A' && lower_name[j] <= 'Z') lower_name[j] += 32;
        }
        append_indent_h(gen);
        appendf_h(gen, "%sArray %ss;\n", program->entities[i]->name.lexeme, lower_name);
    }

    gen->indent_level--;
    append_h(gen, "} GameState;\n\n");
}

// Generate expression as C code
static void generate_expr(CodeGen* gen, Expr* expr, const char* entity_name) {
    switch (expr->type) {
        case EXPR_LITERAL:
            if (expr->as.literal.value.type == LITERAL_NUMBER) {
                appendf(gen, "%g", expr->as.literal.value.as.number);
            } else if (expr->as.literal.value.type == LITERAL_STRING) {
                appendf(gen, "\"%s\"", expr->as.literal.value.as.string);
            } else if (expr->as.literal.value.type == LITERAL_BOOLEAN) {
                append(gen, expr->as.literal.value.as.boolean ? "true" : "false");
            }
            break;

        case EXPR_VARIABLE: {
            const char* varname = expr->as.variable.name.lexeme;

            if (strcmp(varname, "self") == 0) {
                append(gen, "entity");
            } else if (strcmp(varname, "transform") == 0) {
                append(gen, "(&game->transforms.data[eid])");
            } else if (strcmp(varname, "renderable") == 0) {
                append(gen, "(&game->renderables.data[eid])");
            } else if (strcmp(varname, "collision") == 0) {
                append(gen, "/* collision - needs runtime type check */");
            } else {
                append(gen, varname);
            }
            break;
        }

        case EXPR_BINARY:
            generate_expr(gen, expr->as.binary.left, entity_name);
            appendf(gen, " %s ", expr->as.binary.oprt.lexeme);
            generate_expr(gen, expr->as.binary.right, entity_name);
            break;

        case EXPR_UNARY:
            append(gen, expr->as.unary.oprt.lexeme);
            generate_expr(gen, expr->as.unary.right, entity_name);
            break;

        case EXPR_GROUPING:
            append(gen, "(");
            generate_expr(gen, expr->as.grouping.expression, entity_name);
            append(gen, ")");
            break;

        case EXPR_ASSIGN:
            // Check if assigning to self.field
            appendf(gen, "%s", expr->as.assign.name.lexeme);
            append(gen, " = ");
            generate_expr(gen, expr->as.assign.value, entity_name);
            break;

        case EXPR_GET:
            generate_expr(gen, expr->as.get.object, entity_name);
            appendf(gen, "->%s", expr->as.get.name.lexeme);
            break;

        case EXPR_SET:
            generate_expr(gen, expr->as.set.object, entity_name);
            appendf(gen, "->%s = ", expr->as.set.name.lexeme);
            generate_expr(gen, expr->as.set.value, entity_name);
            break;

        default:
            append(gen, "/* unsupported expr */");
            break;
    }
}

// Generate statement as C code
static void generate_stmt(CodeGen* gen, Stmt* stmt, const char* entity_name) {
    switch (stmt->type) {
        case STMT_EXPRESSION:
            append_indent(gen);
            generate_expr(gen, stmt->as.expr.expr, entity_name);
            append(gen, ";\n");
            break;

        case STMT_VAR:
            append_indent(gen);
            // For now, assume all vars are float - we'll improve this later
            append(gen, "float ");
            append(gen, stmt->as.var.name.lexeme);
            if (stmt->as.var.initializer) {
                append(gen, " = ");
                generate_expr(gen, stmt->as.var.initializer, entity_name);
            }
            append(gen, ";\n");
            break;

        case STMT_BLOCK:
            for (int i = 0; i < stmt->as.block.count; i++) {
                generate_stmt(gen, stmt->as.block.statements[i], entity_name);
            }
            break;

        case STMT_PRINT:
            // Skip print statements in generated code (or implement debug logging)
            break;

        case STMT_IF:
            append_indent(gen);
            append(gen, "if (");
            generate_expr(gen, stmt->as.if_stmt.condition, entity_name);
            append(gen, ") {\n");
            gen->indent_level++;
            generate_stmt(gen, stmt->as.if_stmt.then_branch, entity_name);
            gen->indent_level--;
            append_indent(gen);
            append(gen, "}");
            if (stmt->as.if_stmt.else_branch) {
                append(gen, " else {\n");
                gen->indent_level++;
                generate_stmt(gen, stmt->as.if_stmt.else_branch, entity_name);
                gen->indent_level--;
                append_indent(gen);
                append(gen, "}");
            }
            append(gen, "\n");
            break;

        case STMT_WHILE:
            append_indent(gen);
            append(gen, "while (");
            generate_expr(gen, stmt->as.while_stmt.condition, entity_name);
            append(gen, ") {\n");
            gen->indent_level++;
            generate_stmt(gen, stmt->as.while_stmt.body, entity_name);
            gen->indent_level--;
            append_indent(gen);
            append(gen, "}\n");
            break;

        default:
            append_indent(gen);
            append(gen, "/* unsupported stmt */\n");
            break;
    }
}

// Generate entity create function
static void generate_entity_create(CodeGen* gen, EntityDecl* entity) {
    // Lowercase the entity name for the function
    char lower_name[256];
    snprintf(lower_name, sizeof(lower_name), "%s", entity->name.lexeme);
    for (int i = 0; lower_name[i]; i++) {
        if (lower_name[i] >= 'A' && lower_name[i] <= 'Z') {
            lower_name[i] = lower_name[i] + 32;  // to lowercase
        }
    }

    appendf(gen, "uint32_t %s_create(GameState* game, float x, float y) {\n", lower_name);
    gen->indent_level++;

    // Create entity in engine
    append_indent(gen);
    append(gen, "uint32_t entity_id = entity_create(&game->registry, &game->transforms,\n");
    append_indent(gen);
    append(gen, "                                   &game->renderables, &game->circles, &game->rectangles);\n");
    append(gen, "\n");

    // Set default collision (we'll make this configurable later)
    append_indent(gen);
    append(gen, "entity_set_collision(&game->registry, entity_id, COLLISION_NONE);\n");
    append(gen, "\n");

    // Initialize engine components with defaults
    append_indent(gen);
    append(gen, "game->transforms.data[entity_id] = (transform_t){\n");
    gen->indent_level++;
    append_indent(gen);
    append(gen, ".x = x, .y = y,\n");
    append_indent(gen);
    append(gen, ".image_xscale = 1.0f, .image_yscale = 1.0f,\n");
    append_indent(gen);
    append(gen, ".up = 1, .right = 1, .rotation_rad = 0.0f\n");
    gen->indent_level--;
    append_indent(gen);
    append(gen, "};\n");
    append(gen, "\n");

    append_indent(gen);
    append(gen, "game->renderables.data[entity_id] = (Renderable){\n");
    gen->indent_level++;
    append_indent(gen);
    append(gen, ".current_sprite_id = SPRITE_NONE,\n");
    append_indent(gen);
    append(gen, ".image_index = 0,\n");
    append_indent(gen);
    append(gen, ".frame_counter = 0.0f,\n");
    append_indent(gen);
    append(gen, ".image_speed = 0.0f\n");
    gen->indent_level--;
    append_indent(gen);
    append(gen, "};\n");
    append(gen, "\n");

    // Add to game-specific array (with realloc if needed)
    appendf(gen, "    if (game->%ss.count >= game->%ss.capacity) {\n", lower_name, lower_name);
    appendf(gen, "        game->%ss.capacity = game->%ss.capacity == 0 ? 8 : game->%ss.capacity * 2;\n",
            lower_name, lower_name, lower_name);
    appendf(gen, "        game->%ss.data = realloc(game->%ss.data, sizeof(%s) * game->%ss.capacity);\n",
            lower_name, lower_name, entity->name.lexeme, lower_name);
    append(gen, "    }\n");
    append(gen, "\n");

    // Initialize entity struct
    appendf(gen, "    game->%ss.data[game->%ss.count++] = (%s){\n",
            lower_name, lower_name, entity->name.lexeme);
    gen->indent_level++;
    append_indent(gen);
    append(gen, ".entity_id = entity_id");

    // Initialize custom fields to zero
    for (int i = 0; i < entity->field_count; i++) {
        append(gen, ",\n");
        append_indent(gen);
        appendf(gen, ".%s = 0", entity->fields[i].name.lexeme);
    }
    append(gen, "\n");

    gen->indent_level--;
    append_indent(gen);
    append(gen, "};\n");
    append(gen, "\n");

    // Generate on_create code
    if (entity->on_create) {
        append_indent(gen);
        appendf(gen, "// on_create\n");
        append_indent(gen);
        appendf(gen, "%s* entity = &game->%ss.data[game->%ss.count - 1];\n",
                entity->name.lexeme, lower_name, lower_name);
        append_indent(gen);
        append(gen, "uint32_t eid = entity->entity_id;  // For component access\n");

        // Generate statements with eid available
        generate_stmt(gen, entity->on_create, entity->name.lexeme);
    }

    append_indent(gen);
    append(gen, "return entity_id;\n");

    gen->indent_level--;
    append(gen, "}\n\n");
}

// Generate entity update function
static void generate_entity_update(CodeGen* gen, EntityDecl* entity) {
    if (!entity->on_update) return;  // Skip if no on_update

    char lower_name[256];
    snprintf(lower_name, sizeof(lower_name), "%s", entity->name.lexeme);
    for (int i = 0; lower_name[i]; i++) {
        if (lower_name[i] >= 'A' && lower_name[i] <= 'Z') {
            lower_name[i] = lower_name[i] + 32;
        }
    }

    appendf(gen, "void %s_update(GameState* game, uint32_t entity_id) {\n", lower_name);
    gen->indent_level++;

    // Find the entity by entity_id
    append_indent(gen);
    appendf(gen, "%s* entity = NULL;\n", entity->name.lexeme);
    append_indent(gen);
    appendf(gen, "for (int i = 0; i < game->%ss.count; i++) {\n", lower_name);
    gen->indent_level++;
    append_indent(gen);
    appendf(gen, "if (game->%ss.data[i].entity_id == entity_id) {\n", lower_name);
    gen->indent_level++;
    append_indent(gen);
    appendf(gen, "entity = &game->%ss.data[i];\n", lower_name);
    append_indent(gen);
    append(gen, "break;\n");
    gen->indent_level--;
    append_indent(gen);
    append(gen, "}\n");
    gen->indent_level--;
    append_indent(gen);
    append(gen, "}\n");
    append_indent(gen);
    append(gen, "if (!entity) return;\n");
    append(gen, "\n");

    // Make eid available for component access
    append_indent(gen);
    append(gen, "uint32_t eid = entity_id;\n");
    append(gen, "\n");

    // Generate on_update code
    append_indent(gen);
    append(gen, "// on_update\n");
    generate_stmt(gen, entity->on_update, entity->name.lexeme);

    gen->indent_level--;
    append(gen, "}\n\n");
}

static void generate_entity_destroy(CodeGen* gen, EntityDecl* entity, Program* program) {
    char lower_name[256];
    snprintf(lower_name, sizeof(lower_name), "%s", entity->name.lexeme);
    for (int i = 0; lower_name[i]; i++) {
        if (lower_name[i] >= 'A' && lower_name[i] <= 'Z') {
            lower_name[i] = lower_name[i] + 32;
        }
    }

    appendf(gen, "void %s_destroy(GameState* game, uint32_t entity_id) {\n", lower_name);
    gen->indent_level++;

    // Run on_destroy user code first
    if (entity->on_destroy) {
        append_indent(gen);
        appendf(gen, "%s* entity = NULL;\n", entity->name.lexeme);
        append_indent(gen);
        appendf(gen, "for (int i = 0; i < game->%ss.count; i++) {\n", lower_name);
        gen->indent_level++;
        append_indent(gen);
        appendf(gen, "if (game->%ss.data[i].entity_id == entity_id) {\n", lower_name);
        gen->indent_level++;
        append_indent(gen);
        appendf(gen, "entity = &game->%ss.data[i];\n", lower_name);
        append_indent(gen);
        append(gen, "break;\n");
        gen->indent_level--;
        append_indent(gen);
        append(gen, "}\n");
        gen->indent_level--;
        append_indent(gen);
        append(gen, "}\n");

        append_indent(gen);
        append(gen, "if (!entity) return;\n");
        append_indent(gen);
        append(gen, "uint32_t eid = entity_id;\n");
        append_indent(gen);
        append(gen, "// on_destroy\n");
        generate_stmt(gen, entity->on_destroy, entity->name.lexeme);
        append(gen, "\n");
    }

    // Call engine destroy (swap-and-pop)
    append_indent(gen);
    append(gen, "int moved_id = entity_destroy(&game->registry, entity_id,\n");
    append_indent(gen);
    append(gen, "    &game->transforms, &game->renderables,\n");
    append_indent(gen);
    append(gen, "    &game->circles, &game->rectangles);\n");
    append(gen, "\n");

    // Remove from this entity's array
    append_indent(gen);
    appendf(gen, "for (int i = 0; i < game->%ss.count; i++) {\n", lower_name);
    gen->indent_level++;
    append_indent(gen);
    appendf(gen, "if (game->%ss.data[i].entity_id == entity_id) {\n", lower_name);
    gen->indent_level++;
    append_indent(gen);
    appendf(gen, "game->%ss.data[i] = game->%ss.data[game->%ss.count - 1];\n",
            lower_name, lower_name, lower_name);
    append_indent(gen);
    appendf(gen, "game->%ss.count--;\n", lower_name);
    append_indent(gen);
    append(gen, "break;\n");
    gen->indent_level--;
    append_indent(gen);
    append(gen, "}\n");
    gen->indent_level--;
    append_indent(gen);
    append(gen, "}\n");
    append(gen, "\n");

    // Update moved entity references in ALL entity arrays
    append_indent(gen);
    append(gen, "// Fix moved entity references (swap-and-pop)\n");
    append_indent(gen);
    append(gen, "if (moved_id != -1) {\n");
    gen->indent_level++;

    for (int i = 0; i < program->entity_count; i++) {
        char other_lower[256];
        snprintf(other_lower, sizeof(other_lower), "%s", program->entities[i]->name.lexeme);
        for (int j = 0; other_lower[j]; j++) {
            if (other_lower[j] >= 'A' && other_lower[j] <= 'Z') {
                other_lower[j] = other_lower[j] + 32;
            }
        }

        append_indent(gen);
        appendf(gen, "for (int i = 0; i < game->%ss.count; i++) {\n", other_lower);
        gen->indent_level++;
        append_indent(gen);
        appendf(gen, "if (game->%ss.data[i].entity_id == (uint32_t)moved_id) {\n", other_lower);
        gen->indent_level++;
        append_indent(gen);
        appendf(gen, "game->%ss.data[i].entity_id = entity_id;\n", other_lower);
        append_indent(gen);
        append(gen, "break;\n");
        gen->indent_level--;
        append_indent(gen);
        append(gen, "}\n");
        gen->indent_level--;
        append_indent(gen);
        append(gen, "}\n");
    }

    gen->indent_level--;
    append_indent(gen);
    append(gen, "}\n");

    gen->indent_level--;
    append(gen, "}\n\n");
}

static void generate_game_init(CodeGen* gen, Program* program) {
    append(gen, "void game_init(GameState* game) {\n");
    gen->indent_level++;

    // Initialize all entity arrays
    for (int i = 0; i < program->entity_count; i++) {
        char lower_name[256];
        snprintf(lower_name, sizeof(lower_name), "%s", program->entities[i]->name.lexeme);
        for (int j = 0; lower_name[j]; j++) {
            if (lower_name[j] >= 'A' && lower_name[j] <= 'Z') lower_name[j] += 32;
        }

        append_indent(gen);
        appendf(gen, "game->%ss.data = malloc(sizeof(%s) * 8);\n",
                lower_name, program->entities[i]->name.lexeme);
        append_indent(gen);
        appendf(gen, "game->%ss.capacity = 8;\n", lower_name);
        append_indent(gen);
        appendf(gen, "game->%ss.count = 0;\n", lower_name);
        append(gen, "\n");
    }

    append_indent(gen);
    append(gen, "// TODO: Initial entity spawns go here\n");

    gen->indent_level--;
    append(gen, "}\n\n");
}

static void generate_game_update(CodeGen* gen, Program* program) {
    append(gen, "void game_update(GameState* game) {\n");
    gen->indent_level++;

    // Update all entity types
    for (int i = 0; i < program->entity_count; i++) {
        char lower_name[256];
        snprintf(lower_name, sizeof(lower_name), "%s", program->entities[i]->name.lexeme);
        for (int j = 0; lower_name[j]; j++) {
            if (lower_name[j] >= 'A' && lower_name[j] <= 'Z') lower_name[j] += 32;
        }

        append_indent(gen);
        appendf(gen, "for (int i = 0; i < game->%ss.count; i++) {\n", lower_name);
        gen->indent_level++;
        append_indent(gen);
        appendf(gen, "%s_update(game, game->%ss.data[i].entity_id);\n",
                lower_name, lower_name);
        gen->indent_level--;
        append_indent(gen);
        append(gen, "}\n");
    }

    gen->indent_level--;
    append(gen, "}\n\n");
}

static void generate_game_cleanup(CodeGen* gen, Program* program) {
    append(gen, "void game_cleanup(GameState* game) {\n");
    gen->indent_level++;

    // Free all entity arrays
    for (int i = 0; i < program->entity_count; i++) {
        char lower_name[256];
        snprintf(lower_name, sizeof(lower_name), "%s", program->entities[i]->name.lexeme);
        for (int j = 0; lower_name[j]; j++) {
            if (lower_name[j] >= 'A' && lower_name[j] <= 'Z') lower_name[j] += 32;
        }

        append_indent(gen);
        appendf(gen, "free(game->%ss.data);\n", lower_name);
    }

    gen->indent_level--;
    append(gen, "}\n\n");
}

void codegen_generate_program(CodeGen* gen, Program* program) {
    // ===== HEADER =====
    append_h(gen, "#ifndef GAME_GENERATED_H\n");
    append_h(gen, "#define GAME_GENERATED_H\n\n");
    append_h(gen, "#include <stdint.h>\n");
    append_h(gen, "#include <stdbool.h>\n");
    append_h(gen, "#include <stdlib.h>\n");
    append_h(gen, "#include \"forward.h\"\n\n"); //forward declarations, i don't know if these are required.
    append_h(gen, "#include \"entity.h\"\n");
    append_h(gen, "#include \"transform.h\"\n");
    append_h(gen, "#include \"renderable.h\"\n");
    append_h(gen, "#include \"collision.h\"\n");
    append_h(gen, "#include \"timer.h\"\n\n");
    append_h(gen, "#include \"sprite.h\"\n\n");

    // Entity structs and arrays go in header
    for (int i = 0; i < program->entity_count; i++) {
        generate_entity_struct_h(gen, program->entities[i]);
        generate_entity_array_h(gen, program->entities[i]);
    }

    // GameState goes in header
    generate_game_state_h(gen, program);

    // Function declarations go in header
    for (int i = 0; i < program->entity_count; i++) {
        char lower_name[256];
        snprintf(lower_name, sizeof(lower_name), "%s", program->entities[i]->name.lexeme);
        for (int j = 0; lower_name[j]; j++) {
            if (lower_name[j] >= 'A' && lower_name[j] <= 'Z') lower_name[j] += 32;
        }
        appendf_h(gen, "uint32_t %s_create(GameState* game, float x, float y);\n", lower_name);
        appendf_h(gen, "void %s_update(GameState* game, uint32_t entity_id);\n", lower_name);
        appendf_h(gen, "void %s_destroy(GameState* game, uint32_t entity_id);\n", lower_name);
    }

    append_h(gen, "\n#endif // GAME_GENERATED_H\n");

    // ===== SOURCE =====
    append(gen, "#include \"game_generated.h\"\n\n");

    // Function implementations go in source
    for (int i = 0; i < program->entity_count; i++) {
        generate_entity_create(gen, program->entities[i]);
        generate_entity_update(gen, program->entities[i]);
        generate_entity_destroy(gen, program->entities[i], program);
    }

    // Generate game lifecycle functions
    generate_game_init(gen, program);
    generate_game_update(gen, program);
    generate_game_cleanup(gen, program);
}

void codegen_write_files(CodeGen* gen, const char* header_path, const char* source_path) {
    FILE* f = fopen(header_path, "w");
    if (!f) error(error_messages[ERROR_FILELOAD].message);
    fprintf(f, "%s", gen->header_output);
    fclose(f);
    printf("Wrote: %s\n", header_path);

    f = fopen(source_path, "w");
    if (!f) error(error_messages[ERROR_FILELOAD].message);
    fprintf(f, "%s", gen->source_output);
    fclose(f);
    printf("Wrote: %s\n", source_path);
}
