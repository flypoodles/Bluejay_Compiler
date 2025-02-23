#define _POSIX_C_SOURCE 200809L // enable strdup

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "ast.h"
#include "transform.h"


void transformExpr(exp_node * e, S_table global_types, S_table function_decs, frame * f) {
    if(!e) return;
    switch(e->kind){
        case int_exp: {
            // Transform int constants as needed
            break;
        }
        case binop_exp: {
    // plus_op,
    // minus_op, == e1 plus -1 mul e2
    // mul_op,
    // div_op,
    // rem_op,
    // bor_op,
    // band_op,
    // xor_op,
    // lt_op,
    // le_op, == ! gt
    // gt_op,
    // ge_op, == ! lt
    // or_op,
    // and_op,
    // eq_op,
    // ne_op  == ! eq
            // Transform binary operations as needed
            // Don't forget to transform the whole AST
            switch(e->data.bin_ops.op)
            {
            case eq_op:
            case and_op:
            case or_op:
            case gt_op:
            case xor_op:
            case band_op:
            case rem_op:
            case div_op:
            case bor_op:
            case mul_op:
            case lt_op:
            case plus_op:
                {

                    break;
                }
                case minus_op:
                {
                    exp_node * newOp2 = BinOpNode(mul_op, IntNode(-1), e->data.bin_ops.e2);
                    e->data.bin_ops.e2 = newOp2;
                    e->data.bin_ops.op = plus_op;
                    transformExpr(e, global_types, function_decs,f);
                    return;
                }
            case ge_op:
                {
                    exp_node* op1 = e->data.bin_ops.e1;
                    exp_node * op2 = e->data.bin_ops.e2;
                    exp_node * newOp = BinOpNode(lt_op, op1,op2);
                    e->kind =unop_exp;
                    e->data.un_ops.op=not_op;
                    e->data.un_ops.e=newOp;
                    transformExpr(e, global_types, function_decs,f);
                    return;
                }
            case le_op:
                {
                    exp_node* op1 = e->data.bin_ops.e1;
                    exp_node * op2 = e->data.bin_ops.e2;
                    exp_node * newOp = BinOpNode(gt_op, op1,op2);
                    e->kind =unop_exp;
                    e->data.un_ops.op=not_op;
                    e->data.un_ops.e=newOp;
                    transformExpr(e, global_types, function_decs,f);
                    return;
                }
            case ne_op:
                {
                    exp_node* op1 = e->data.bin_ops.e1;
                    exp_node * op2 = e->data.bin_ops.e2;
                    exp_node * newOp = BinOpNode(eq_op, op1,op2);
                    e->kind =unop_exp;
                    e->data.un_ops.op=not_op;
                    e->data.un_ops.e=newOp;
                    transformExpr(e, global_types, function_decs,f);
                    return;
                }


                default:
                    assert(0);
            }
            transformExpr(e->data.bin_ops.e1, global_types, function_decs, f);
            transformExpr(e->data.bin_ops.e2, global_types, function_decs, f);
            break;
        }
        case unop_exp: {
            // Transform unary operations as needed
            // Don't forget to transform the whole AST
            transformExpr(e->data.un_ops.e, global_types, function_decs, f);
            break;
        }
        // TODO transform the rest of the expressions in the AST as needed
        case func_exp:
            {
                list * argList = e->data.func_op.argsList;
                while(argList != NULL )
                {
                    exp_node* current = argList->head;
                    transformExpr(current, global_types, function_decs,f);
                    argList= argList->next;
                }
                break;
            }
        case str_exp:
            break;
        case var_exp:
            break;
    case void_exp:
            break;
    case ter_exp:
        {
            transformExpr(e->data.ter_op.cond,global_types,function_decs,f);
            transformExpr(e->data.ter_op.elsestmt,global_types,function_decs,f);
            transformExpr(e->data.ter_op.then,global_types,function_decs,f);
            break;
        }

        default:
            assert(0);
    }
}

void transformStmts(list * l, S_table global_types, S_table function_decs, frame * f) {
    if (l == NULL) return;

    stmt_node * s = l->head;
    if(!s) return;
    switch(s->kind){
        case if_stmt: {
            // Transform if statements as needed
            // Don't forget to traverse the whole AST
            transformExpr(s->data.if_ops.cond, global_types, function_decs, f);
            transformStmts(s->data.if_ops.then_stmts, global_types, function_decs, f);
            transformStmts(s->data.if_ops.else_stmts, global_types, function_decs, f);
            break;
        }
        case ret_stmt: {
            // Transform if statements as needed
            // Don't forget to traverse the whole AST
            transformExpr(s->data.ret_exp, global_types, function_decs, f);
            break;
        }
    case func_stmt:
            {
                transformExpr(s->data.func_op.funcExp, global_types, function_decs, f);
                break;
            }
    case assign_stmt:
            {
                transformExpr(s->data.assign_op.assign_exp,global_types,function_decs,f);
                break;
            }
    case while_stmt:
            {
                if(s->data.while_ops.otherwise_stmt == NULL){
                    transformExpr(s->data.while_ops.cond, global_types, function_decs, f);
                    transformStmts(s->data.while_ops.then_stmts, global_types, function_decs, f);
                    break;
                }
                list * thenstmts = s->data.while_ops.then_stmts;
                list * elsestmts = s->data.while_ops.otherwise_stmt;
                exp_node * cond = s->data.while_ops.cond;
                list * stmts = ListCopy(thenstmts);
                stmt_node* newWhile = whileNode(cond,thenstmts,NULL);
                stmts = ListAddLast(newWhile, stmts);

                s->kind= if_stmt;
                s->data.if_ops.cond=cond;
                s->data.if_ops.then_stmts=stmts;
                s->data.if_ops.else_stmts=elsestmts;
                transformStmts(l, global_types, function_decs, f);
                return;
            }
    case repeat_stmt:
            {
                transformStmts(s->data.repeat_ops.then_stmts, global_types, function_decs, f);

            }
        // TODO transform the rest of the statements in the AST as needed
        default:
            break;
    }

    transformStmts(l->next, global_types, function_decs, f);
}

void transformVariable(vardec_node * node, S_table globals_types, S_table function_decs, frame * f) {
    // Transform variable declarations as needed
    transformExpr(node->init,globals_types,function_decs,f);
}

void transformVariables(list * l, S_table global_types, S_table function_decs, frame * f) {
    if (l == NULL) return;
    transformVariable((vardec_node *)l->head, global_types, function_decs, f);
    transformVariables(l->next, global_types, function_decs, f);
}

void transformFunction(fundec_node * fundec, S_table globals, S_table functions_rets, frame * f) {
    // Transform function declarations as needed
//    assert(fundec->localVarList==NULL);


    list * newLocalImplicit = ListCopy(fundec->impList);
    list * newLocal = ListCopy(fundec->localVarList);

    while(newLocalImplicit != NULL)
    {
        newLocal = ListAddFirst(newLocalImplicit->head, newLocal);
        newLocalImplicit = newLocalImplicit->next;
    }
    fundec->localVarList = newLocal;

    list * localList = fundec->localVarList;
    while(localList != NULL)
    {

        vardec_node * curLocal = localList->head;
        printf("%s\n",curLocal->name);
        transformExpr(curLocal->init,globals,functions_rets,f);
        localList=localList->next;
    }

    transformStmts(fundec->stmts, globals, functions_rets, f);

    if (fundec->type == VoidTyNode())
        ListAddLast(RetNode(voidNode()), fundec->stmts);
}

void transformFunctions(list * l, S_table global_types, S_table function_decs, S_table frames) {
    if (l == NULL) return;
    fundec_node * fundec = (fundec_node*) l->head;
    frame * f = S_look(frames, S_Symbol(fundec->name));
    assert(f != NULL);
    transformFunction(fundec, global_types, function_decs, f);
    transformFunctions(l->next, global_types, function_decs, frames);
}

void transform(program * p, S_table global_types, S_table function_decs, S_table frames) {
    transformVariables(p->variables, global_types, function_decs, NULL);
    transformFunctions(p->functions, global_types, function_decs, frames);

    stmt_node * exitNode =funcStatementNode( funcExpNode("exit", ListAddFirst(IntNode(0), NULL)));
    ListAddLast(exitNode, p->statements);
    transformStmts(p->statements, global_types, function_decs, NULL);
}