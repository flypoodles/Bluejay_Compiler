#ifndef AST_TO_IR_H
#define AST_TO_IR_H

#include "ir.h"

ir_node * ast_to_ir(program* p, S_table globals_types, S_table functions_ret_types, S_table functions_frames);

#endif
