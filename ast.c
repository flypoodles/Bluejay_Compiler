#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ast.h"
#include <stdio.h>



static ty_node int_ty_node    = { int_ty };
static ty_node str_ty_node    = { str_ty };
static ty_node void_ty_node   = {void_ty};
ty_node* IntTyNode(){
    return &int_ty_node;
}
ty_node* StrTyNode(){
    return &str_ty_node;
}
ty_node* VoidTyNode()
{
    return &void_ty_node;
}
// TODO add more types

ty_node* TyNode(char * type) {
    if (!strcmp(type,"int")) {
        return IntTyNode();
    }
    if (!strcmp(type, "string"))
    {
        return StrTyNode();
    }
    if(!strcmp(type, "void"))
    {
        return VoidTyNode();
    }

    // TODO add more types
    assert(0);
}

char* typeToStr(ty_node* type) {
    switch (type->kind) {
        case int_ty:
            return "int";
        // TODO add more types
    case str_ty:
            return "string";
    case void_ty:
            return "void";
        default:
            assert(0);
    }
}

param * Param(ty_node * type, char * name) {
    param * p = malloc(sizeof(param));
    p->name = name;
    p->ty = type;
    return p;
}


exp_node* IntNode(int val){
    exp_node* node = malloc(sizeof(exp_node));
    node->kind = int_exp;
    node->data.ival = val;
    return node;
}
exp_node* strConstNode(char * val)
{
    exp_node* node = malloc(sizeof(exp_node));
    node ->kind = str_exp;
    node->data.strVal=val;
    return node;
}
exp_node * voidNode()
{
    exp_node* node = malloc(sizeof(exp_node));
    node ->kind = void_exp;
    return node;
}
exp_node* varNode(char * val)
{
    exp_node* node = malloc(sizeof(exp_node));
    node ->kind = var_exp;
    node->data.varVal=S_Symbol(val);

    return node;
}

exp_node* BinOpNode(binop operation, exp_node * left, exp_node * right) {
    exp_node* node = malloc(sizeof(exp_node));
    node->kind = binop_exp;
    node->data.bin_ops.op = operation;
    node->data.bin_ops.e1 = left;
    node->data.bin_ops.e2 = right;
    return node;
}

exp_node* UnOpNode(unop operation, exp_node * expr) {
    exp_node* node = malloc(sizeof(exp_node));
    node->kind = unop_exp;
    node->data.un_ops.op = operation;
    node->data.un_ops.e = expr;
    return node;
}
exp_node* funcExpNode(char * name, list * argsList)
{
    exp_node* node = malloc(sizeof(exp_node));
    node->kind = func_exp;
    node->data.func_op.name=name;
    node->data.func_op.argsList = argsList;
    return node;
}
exp_node* ternaryNode(exp_node * cond, exp_node * then, exp_node * elseStmt)
{
    exp_node * node = malloc(sizeof(exp_node));
    node ->kind=ter_exp;
    node->data.ter_op.cond=cond;
    node->data.ter_op.then=then;
    node->data.ter_op.elsestmt= elseStmt;
    return node;
}
stmt_node* funcStatementNode(exp_node * func)
{
    stmt_node* node = malloc(sizeof(stmt_node));
    node->kind = func_stmt;
    node->data.func_op.funcExp = func;
    return node;

}
stmt_node* RetNode(exp_node* e){
    stmt_node* node = malloc(sizeof(stmt_node));
    node->kind = ret_stmt;
    node->data.ret_exp = e;
    return node;
}

stmt_node* IfNode(exp_node* cond, list * thenStmts, list * elseStmts) {
    stmt_node* node = malloc(sizeof(stmt_node));
    node->kind = if_stmt;
    node->data.if_ops.cond = cond;
    node->data.if_ops.then_stmts = thenStmts;
    node->data.if_ops.else_stmts = elseStmts;
    return node;
}
stmt_node* whileNode(exp_node* cond, list * thenStmts, list * otherWiseStmts){
    stmt_node* node = malloc(sizeof(stmt_node));
    node->kind = while_stmt;
    node->data.while_ops.cond=cond;
    node->data.while_ops.then_stmts=thenStmts;
    node->data.while_ops.otherwise_stmt=otherWiseStmts;
    return node;

}

stmt_node* repeatNode(exp_node* cond, list * thenStmts) {
    stmt_node* node = malloc(sizeof(stmt_node));
    node->kind = repeat_stmt;
    node->data.repeat_ops.cond=cond;
    node->data.repeat_ops.then_stmts=thenStmts;
    return node;
}
stmt_node* assignNode(char * name, exp_node* exp)
{
    stmt_node* node = malloc(sizeof(stmt_node));
    node->kind = assign_stmt;
    node->data.assign_op.sym=S_Symbol(name);
    node->data.assign_op.assign_exp = exp;
    return node;
}

vardec_node * VarDecNode(char * name, ty_node * type, exp_node * init, int isImplicit) {
    vardec_node* node = malloc(sizeof(vardec_node));
    node->name = name;
    node->type = type;
    node->init = init;
    node->isImplicit=isImplicit;
    return node;
}

fundec_node * FunDecNode(char * name,ty_node * type, list * argList, list * localVarList, list * stmts ) {
    fundec_node * node = malloc(sizeof(fundec_node));
    node->name = name;
    node -> type = type;
    node->argList = argList;
    node->localVarList=localVarList;
    node->stmts=stmts;
    node->impList=NULL;
    return node;
}

list* ListAddFirst(void* hd, list* lst){
    list* l = malloc(sizeof(list));
    l->head = hd;
    l->next = lst;
    return l;
}

list* ListAddLast(void* hd, list* lst){
    list* l = malloc(sizeof(list));
    l->head = hd;

    l->next = NULL;

    // Empty list
    if (lst == NULL) {
        return l;
    }

    // Find last
    list * last = lst;
    while (last->next != NULL) {
        last = last->next;
    }


    last->next = l;
    return lst;
}

list* ListCopy(list* lst) {
    if (lst == NULL)
        return NULL;
    list * copy = malloc(sizeof(list));
    copy->head = lst->head;
    copy->next = ListCopy(lst->next);
    return copy;
}