#include "printer.h"
#include "literal.h"
#include "token.h"
#include <stdio.h>
#include "stmt.h"

static void print_expr_recursive(Expr* expr, int indent) {
 if (!expr) return;

    // Print indentation
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }

    switch (expr->type) {
        case EXPR_BINARY:
            printf("Binary (%s)\n", token_type_to_string(expr->as.binary.oprt.type));
            print_expr_recursive(expr->as.binary.left, indent + 1);
            print_expr_recursive(expr->as.binary.right, indent + 1);
            break;

        case EXPR_UNARY:
            printf("Unary (%s)\n", token_type_to_string(expr->as.unary.oprt.type));
            print_expr_recursive(expr->as.unary.right, indent + 1);
            break;

        case EXPR_LITERAL:
            printf("Literal (%s)\n", literal_to_string(expr->as.literal.value));
            break;

        case EXPR_GROUPING:
            printf("Grouping\n");
            print_expr_recursive(expr->as.grouping.expression, indent + 1);
            break;

        case EXPR_VARIABLE:
            printf("Variable (%s)\n", expr->as.variable.name.lexeme);
            break;

        case EXPR_ASSIGN:
            printf("Assign (%s)\n", expr->as.assign.name.lexeme);
            print_expr_recursive(expr->as.assign.value, indent + 1);
            break;

        case EXPR_GET:
            printf("Get\n");
            print_expr_recursive(expr->as.get.object, indent + 1);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Property: %s\n", expr->as.get.name.lexeme);
            break;

        case EXPR_SET:
            printf("Set\n");
            print_expr_recursive(expr->as.set.object, indent + 1);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Property: %s\n", expr->as.set.name.lexeme);
            print_expr_recursive(expr->as.set.value, indent + 1);
            break;
        case EXPR_CALL:
            printf("Call\n");
            print_expr_recursive(expr->as.call.callee, indent + 1);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Arguments (%d):\n", expr->as.call.argc);
            for (int i = 0; i < expr->as.call.argc; i++) {
                print_expr_recursive(expr->as.call.argv[i], indent + 2);
            }
            break;
    }
}

void print_ast(Expr* expr) {
    print_expr_recursive(expr, 0);
}

static void print_expr_recursive(Expr* expr, int indent);

static void print_stmt_recursive(Stmt* stmt, int indent) {
    if (!stmt) return;

    for (int i = 0; i < indent; i++) printf("  ");

    switch (stmt->type) {
        case STMT_EXPRESSION:
            printf("ExprStmt\n");
            print_expr_recursive(stmt->as.expr.expr, indent + 1);
            break;

        case STMT_PRINT:
            printf("PrintStmt\n");
            print_expr_recursive(stmt->as.print.expr, indent + 1);
            break;

        case STMT_VAR:
            printf("VarDecl (%s)\n", stmt->as.var.name.lexeme);
            if (stmt->as.var.initializer) {
                print_expr_recursive(stmt->as.var.initializer, indent + 1);
            }
            break;

        case STMT_BLOCK:
            printf("Block\n");
            for (int i = 0; i < stmt->as.block.count; i++) {
                print_stmt_recursive(stmt->as.block.statements[i], indent + 1);
            }
            break;

        case STMT_IF:
            printf("IfStmt\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Condition:\n");
            print_expr_recursive(stmt->as.if_stmt.condition, indent + 2);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Then:\n");
            print_stmt_recursive(stmt->as.if_stmt.then_branch, indent + 2);
            if (stmt->as.if_stmt.else_branch) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("Else:\n");
                print_stmt_recursive(stmt->as.if_stmt.else_branch, indent + 2);
            }
            break;

        case STMT_WHILE:
            printf("WhileStmt\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Condition:\n");
            print_expr_recursive(stmt->as.while_stmt.condition, indent + 2);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Body:\n");
            print_stmt_recursive(stmt->as.while_stmt.body, indent + 2);
            break;
    }
}

void print_program(Stmt** statements, int count) {
    for (int i = 0; i < count; i++) {
        print_stmt_recursive(statements[i], 0);
    }
}
