#ifndef IR_PUSHPOP_H
#define IR_PUSHPOP_H

#include "ir.h"

// Adds explicit operations to push/pop arguments to/from the stack into virtual registers 0 and 1
ir_node * addPushPop(ir_node * ir);

#endif
