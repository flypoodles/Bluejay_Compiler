#ifndef OPTIMIZE_H
#define OPTIMIZE_H

#include "ast.h"
#include "ir.h"

void optimize_ast(program * p, S_table globals_types, S_table function_rets, S_table frames);

ir_node * optimize_tree_ir(ir_node * ir);

ir_node * optimize_list_ir(ir_node * ir);

#endif
