#include "ir_tree_to_list.h"

#include <assert.h>
#include <stdlib.h>

__attribute__ ((unused))
static ir_node * find_last(ir_node * head) {
    if (head->next == NULL)
        return head;
    return find_last(head->next);
}

ir_node * ir_tree_to_list(ir_node * ir) {
    if (ir == NULL) {
        return Nop();
    }

    switch (ir->kind) {
        // plus_op,
        // minus_op, == e1 plus -1 mul e2 --- not should exist
        // mul_op,
        // div_op,
        // rem_op,
        // bor_op,
        // band_op,
        // xor_op,
        // lt_op,
        // le_op, == ! gt  --- not should exist
        // gt_op,
        // ge_op, == ! lt --- not should exist
        // or_op,
        // and_op,
        // eq_op,
        // ne_op  == ! eq --- not should exist
        case ir_nop: {
            return ir;
        }
        // TODO support all IRs, turn them from a tree into a list
        case ir_iconst: { return ir; }
        case ir_sconst: { return ir;  }
        case ir_sub: { assert(0); } // should never happen
        case ir_add:
        case ir_mul:
        case ir_div:
        case ir_mod:
        case ir_bor:
        case ir_band:
        case ir_xor:
        case ir_eq:
        case ir_lt:
        case ir_gt: {
                ir_node * op1 = ir_tree_to_list(ir->tree_ir_1);
                ir_node * op2 = ir_tree_to_list(ir->tree_ir_2);
                find_last(op1) ->next = op2;
                find_last(op2) -> next = ir;
                return op1;
        }
    case ir_or: {
                ir_label * skipLab = freshLabel();
                ir_label * OpLabel = freshLabel();
                ir_node * op1 = ir_tree_to_list(ir->tree_ir_1);
                ir_node * op2 = ir_tree_to_list(ir->tree_ir_2);
                ir_node * branch = Branch(skipLab);
                ir_node * jumpLabel = Jump(OpLabel);
                ir_node * notOp = Ops(ir_not);

                find_last(op1) -> next = notOp;

                notOp->next = branch;

                branch -> next = op2;

                find_last(op2)->next = Iconst(0);

                find_last(op2) ->next = jumpLabel;

                jumpLabel->next=Label(skipLab);

                find_last(jumpLabel) ->next = Iconst(1);
                find_last(jumpLabel) ->next = Iconst(1);
                find_last(jumpLabel) ->next = Label(OpLabel);
                find_last(jumpLabel) ->next = ir;
                return op1;
    }
    case ir_and:
        {

                ir_label * skipLab = freshLabel();
                ir_label * OpLabel = freshLabel();
                ir_node * op1 = ir_tree_to_list(ir->tree_ir_1);
                ir_node * op2 = ir_tree_to_list(ir->tree_ir_2);
                ir_node * branch = Branch(skipLab);
                ir_node * jumpLabel = Jump(OpLabel);

                ir_node * falseCond1 = Iconst(0);
                ir_node * falseCond2 = Iconst(0);
                falseCond1 ->next = falseCond2;
                falseCond2 ->next = Label(OpLabel);
                find_last(falseCond2)->next = ir;

                find_last(op1) ->next = branch;
                branch->next = op2;
                find_last(op2) -> next = Iconst(1);
                find_last(op2) ->next =jumpLabel;
                jumpLabel->next = Label(skipLab);
                find_last(jumpLabel)->next = falseCond1;

                return op1;
            }
        case ir_not: {
                ir_node * op = ir_tree_to_list(ir->tree_ir_1);
                find_last(op)->next = ir;
                return op;
}
        case ir_reserve: {

                if(ir->data.reserve.val == NULL)
                {
                    ir_node * init = ir_tree_to_list(ir->tree_ir_1);
                    ir ->next = init;
                }

                return  ir;
}
        case ir_read:
            {
                return ir;
            }
        case ir_write: {
                ir_node * val = ir_tree_to_list(ir->tree_ir_1);
                find_last(val) ->next= ir;
                return val;

 }
        case ir_arglocal_read: {

                return ir;
}
        case ir_arglocal_write: {

                ir_node * value = ir_tree_to_list(ir->tree_ir_1);
                find_last(value)->next = ir;
                return value;
 }
        case ir_lbl: {return ir; }
        case ir_jump: {
                return ir;
}
        case ir_branchzero: {
                ir_node * cond = ir_tree_to_list(ir->tree_ir_1);

                ir_node * then = ir_tree_to_list(ir->tree_ir_2);
                ir_node * elseSt = ir_tree_to_list(ir->tree_ir_3);

                find_last(cond) -> next = ir;
                ir->next = then;
                find_last(then) ->next = elseSt;

                return cond;
 }
        case ir_function: {

                ir->next = ir_tree_to_list(ir->tree_ir_2);
                find_last(ir)->next = ir_tree_to_list(ir->tree_ir_3);
                return ir;
 }
        case ir_call: {



                ir_node* head = ir->tree_ir_1;

                find_last(head) -> next = ir_tree_to_list(ir->tree_ir_2);
                find_last(head) ->next= ir;
                return head;
 }
        case ir_ret: {
                ir_node * arg = ir_tree_to_list(ir->tree_ir_1);
                find_last(arg) -> next = ir;
                return arg;
 }
        case ir_intrinsic: {
            // ir_intrinstic
                // {tree_ir_1} arg
                // [arg...] -> ir_intrinstic

                ir_node * arg = ir_tree_to_list(ir->tree_ir_1);
                find_last(arg) ->next = ir;
                return arg;
        }

        case ir_seq:
            {
               ir_node * ir1 = ir_tree_to_list(ir->data.seq.o1);
                ir_node * ir2 = ir_tree_to_list(ir->data.seq.o2);
                // ir1 = [op1 -> op2]
                // ir2 = [op3 -> op4]
                // ret = [op1 -> op2 -> op3 -> op4]

                ir_node * ret = ir1;
                find_last(ir1) -> next = ir2;
                return ret;
            }

        case ir_push: { assert(0); }
        case ir_pop: { assert(0); }
        default: {
            assert(0); // Not supported
        }
    }
}
