#ifndef AST_H
#define AST_H

#include "symbol.h"

typedef struct list_ list;

struct list_ {
    void* head;
    struct list_ * next;
};

list* ListAddFirst(void* hd, list* tl);
list* ListAddLast(void* hd, list* tl);
list* ListCopy(list* lst);

typedef struct program_ {
    list * variables;
    list * functions;
    list * statements;
} program;


// types
typedef struct ty_node {
    enum {
        int_ty,
        str_ty,
        void_ty,
        // TODO Add more types if needed
    } kind;
} ty_node;

ty_node* IntTyNode();
ty_node* StrTyNode();
ty_node* VoidTyNode();
// TODO Add more types if needed
ty_node* TyNode(char * type);
char* typeToStr(ty_node* type);
// Expressions:


typedef enum {
    plus_op,
    minus_op,
    mul_op,
    div_op,
    rem_op,
    bor_op,
    band_op,
    xor_op,
    lt_op,
    le_op,
    gt_op,
    ge_op,
    or_op,
    and_op,
    eq_op,
    ne_op
    // TODO Add more binary operators
} binop;

typedef enum { not_op } unop;

typedef struct exp_node {
    // This is a tagged union
    // TODO add more variants to support all types of expressions
    enum { int_exp, binop_exp, unop_exp, str_exp,var_exp, void_exp, func_exp, ter_exp} kind;
    union {
        int ival;
        struct { binop op; struct exp_node* e1; struct exp_node* e2; } bin_ops;
        struct { unop op; struct exp_node* e; } un_ops;
        char * strVal;
        S_symbol varVal;
        struct { char * name; list * argsList; } func_op;
        struct {struct exp_node*  cond; struct exp_node* then; struct exp_node* elsestmt;} ter_op;
    } data;
} exp_node;

exp_node* IntNode(int val);
exp_node* BinOpNode(binop operation, exp_node * left, exp_node * right);
exp_node* UnOpNode(unop operation, exp_node * expr);
exp_node* strConstNode(char * val);
exp_node * voidNode();
exp_node* varNode(char * val);
exp_node* funcExpNode(char * name, list * argsList);
exp_node* ternaryNode(exp_node * cond, exp_node * then, exp_node * elseStmt);

void freeExpNode(exp_node *);

// TODO add more constructor functions, one per variant in the tagged union above

// Statements:

typedef struct stmt_node {
    // This is a tagged union
    // TODO add more variants to support all types of statements
    enum {assign_stmt ,if_stmt, ret_stmt,while_stmt, repeat_stmt, func_stmt } kind;
    union {
        struct {S_symbol sym; exp_node* assign_exp;} assign_op;
        struct { exp_node* cond; struct list_* then_stmts; struct list_* else_stmts; } if_ops;
        struct { exp_node* cond; struct list_* then_stmts; struct list_* otherwise_stmt; } while_ops;
        struct { exp_node* cond; struct list_* then_stmts; } repeat_ops;
        exp_node* ret_exp;
        struct { exp_node* funcExp; } func_op;
    } data;
} stmt_node;

stmt_node* RetNode(exp_node* e);
stmt_node* IfNode(exp_node* cond, list * thenStmts, list * elseStmts);
stmt_node* whileNode(exp_node* cond, list * thenStmts, list * otherWiseStmts);
stmt_node* repeatNode(exp_node* cond, list * thenStmts);
stmt_node* assignNode(char * name, exp_node* exp);
stmt_node* funcStatementNode(exp_node * func);
// TODO add more constructor functions, one per variant in the tagged union above

// Variable declaration:

typedef struct vardec_node {
    char * name;
    ty_node * type;
    exp_node * init;
    int isImplicit;
} vardec_node;

vardec_node * VarDecNode(char * name, ty_node * type, exp_node * init, int isImplicit);

// Function declaration:

typedef struct param {
    ty_node* ty;
    char* name;
} param ;

param * Param(ty_node * type, char * name);

typedef struct fundec_node {
    char * name;
    ty_node * type;
    list * argList; // list of Param
    list * localVarList; // list of vardecs
    list * stmts;
    list * impList;
    // TODO add everything that a function declaration needs
} fundec_node;

fundec_node * FunDecNode(char * name,ty_node * type, list * argList, list * localVarList, list * stmts );
// TODO improve constructor with everything that a function declaration needs

typedef struct frame {
    S_table args_locs_types;
    S_table indexes;
    ty_node * ret;
    S_table imp_table;
    char * name;

} frame;

#endif