#ifndef PRINTER_H
#define PRINTER_H

#include "expr.h"
#include "stmt.h"

void print_ast(Expr* expr);
void print_program(Stmt** statements, int count);

#endif