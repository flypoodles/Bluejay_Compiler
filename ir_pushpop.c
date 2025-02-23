#include "ir_pushpop.h"

#include <assert.h>
#include <stdlib.h>

__attribute__ ((unused))
static ir_node * PopReg(int i) {
    ir_node * ret = calloc(1, sizeof(ir_node));
    ret->kind = ir_pop;
    ret->data.iconst = i;
    return ret;
}

__attribute__ ((unused))
static ir_node * PushReg(int i) {
    ir_node * ret = calloc(1, sizeof(ir_node));
    ret->kind = ir_push;
    ret->data.iconst = i;
    return ret;
}

ir_node * addPushPop(ir_node * ir) {
    if (ir == NULL) {
        return NULL;
    }

    ir->next = addPushPop(ir->next);

    __attribute__ ((unused))
    ir_node * next = ir->next;

    switch (ir->kind) {
        case ir_nop: break;
        case ir_iconst: break;
        case ir_sconst: break;
        case ir_sub: {assert(0);};
        case ir_div:
        case ir_mod:
        case ir_add:
        case ir_mul:
        case ir_bor:
        case ir_band:
        case ir_xor:
        case ir_eq:
        case ir_lt:
        case ir_gt:
        case ir_or:
        case ir_and:
            {
                    ir_node * rest = ir->next;
                    ir_node * newOp = Nop();
                    newOp->kind=ir->kind;

                    ir->kind= ir_pop;
                    ir->data.iconst=1;

                    ir_node * pop2 = PopReg(0);
                    ir_node * push = PushReg(0);
                    ir->next = pop2;
                    pop2->next = newOp;
                    newOp->next=push;
                    push->next=rest;
                    break;

            }

        case ir_not:
            {
                ir_node * rest = ir->next;
                ir_node * newOp = Nop();
                newOp->kind=ir->kind;

                ir->kind= ir_pop;
                ir->data.iconst=0;

                ir_node * push = PushReg(0);

                ir->next=newOp;
                newOp->next=push;
                push->next=rest;
            };
    case ir_reserve: break;
        case ir_read:
            {
                ir_node * push = PushReg(0);
                push ->next = ir->next;
                ir->next = push;
                break;

            }
        case ir_arglocal_read: break;
        case ir_arglocal_write: break;
        case ir_write:
            {
                ir_node * rest = ir->next;
                ir_node * newOp = Nop();
                newOp->kind=ir->kind;
                newOp->data.sconst=ir->data.sconst;

                ir->kind = ir_pop;
                ir->data.iconst=0;

                ir->next = newOp;
                newOp->next=rest;
            };
        case ir_lbl: break;
        case ir_function: break;
        case ir_jump: break;
        case ir_branchzero: break;
        case ir_call: break;
        case ir_ret: break;
        case ir_intrinsic: break;
        case ir_seq: break;
        case ir_pop: break;
        default: {
            assert(0); // Not supported
        }
    }

    return ir;
}

