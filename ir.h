#ifndef _IR_H_
#define _IR_H_

#include "ast.h"

typedef struct ir_label {
    char * name;
} ir_label;


enum ir_op {
    ir_nop,

    // Push constants
    ir_iconst,
    ir_sconst,

    // Arithmetic/logic operations
    ir_add,
    ir_sub,
    ir_mul,
    ir_div,
    ir_mod,
    ir_bor,
    ir_band,
    ir_xor,
    ir_or,
    ir_and,
    ir_eq,
    ir_lt,
    ir_gt,
    ir_not,

    // Variables
    ir_reserve,
    ir_read,
    ir_write,
    ir_arglocal_read,
    ir_arglocal_write,

    // Control-flow
    ir_lbl,
    ir_jump,
    ir_branchzero,

    // Functions
    ir_function,
    ir_call,
    ir_ret,
    ir_intrinsic,

    ir_seq,  // Sequence of two nodes, uses irs

    ir_push, // register specified in ival
    ir_pop,  // register specified in ival
};

enum intrinsic {
    intrinsic_print_int,
    intrinsic_print_string,
    intrinsic_exit,
};

typedef struct ir_node {
    enum ir_op kind;
    union {
        int   iconst;
        char* sconst;
        enum intrinsic intrinsic;
        ir_label* lbl;

        struct { int size; char* name; char* val; } reserve;
        struct { char* name; int isaddress; } read_write;

        struct { ir_label* lbl; int vars; } call_function;

        struct { struct ir_node * o1; struct ir_node * o2; } seq;
    } data;
    // You can use these to form a tree
    struct ir_node * tree_ir_1;
    struct ir_node * tree_ir_2;
    struct ir_node * tree_ir_3;
    // You can use next to form a list
    struct ir_node * next;
} ir_node;

// Create a fresh unused label
ir_label * freshLabel();

// Create a label with a given name
ir_label * namedLabel(char * name);

// Constructors
ir_node * Iconst(int i);
ir_node * Sconst(char * s);
ir_node * Ops(enum ir_op);
ir_node * Intrinsic(enum intrinsic op);
ir_node * Reserve(int size, char * name, char * val);
ir_node * Read(char * name);
ir_node * Address(char * name);
ir_node * Write(char * name);
ir_node * ArgLocalRead(int i);
ir_node * ArgLocalWrite(int i);
ir_node * Label(ir_label * lbl);
ir_node * Function(ir_label * lbl, int locs);
ir_node * Call(ir_label * lbl, int vars);
ir_node * Return();
ir_node * Jump(ir_label * lbl);
ir_node * Branch(ir_label * lbl);
ir_node * Seq(ir_node * o1, ir_node * o2);
ir_node * Nop();
ir_node * Pop();

void ir_list_write(ir_node * head, const char * file);

ir_node * ir_list_read(const char * file);

void ir_list_print(ir_node * head);

#endif
