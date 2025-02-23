#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "semantic_analysis_symbols.h"

#include <math.h>

void symbolResolutionExpr(exp_node * e, S_table global_types, S_table function_decs, frame * f) {


    if(!e) return;
    switch(e->kind){
        case int_exp: {
           return; // no variables to check
        }
        case binop_exp: {
                symbolResolutionExpr(e->data.bin_ops.e1, global_types,function_decs,f);
                symbolResolutionExpr(e->data.bin_ops.e2, global_types,function_decs,f);
                break;
        }
        case unop_exp: {
                symbolResolutionExpr(e->data.un_ops.e, global_types,function_decs,f);
           break;
        }
    case str_exp: {
                return;
    }
    case void_exp:
            {
                return;
            }
    case var_exp:
            {
                // make sure to remeber to check for func_dec
                if(f!=NULL)
                {
                    void * localResult = S_look(f->args_locs_types,e->data.varVal);
                    if(localResult != NULL)
                    {
                        return;
                    }
                    if(S_look(f->imp_table,e->data.varVal) != NULL)
                    {
                        return;
                    }
                }

                void * result = S_look(global_types, e->data.varVal);
                if (result == NULL)
                {
                    fprintf(stderr, "variable is not declared  %s ", S_name(e->data.varVal));
                    exit(73);
                }
                return;
            }
    case func_exp:
            {
                void * result = S_look(function_decs, S_Symbol(e->data.func_op.name));
                if (result == NULL)
                {
                    fprintf(stderr, "function is not declared  %s  \n", e->data.func_op.name);
                    exit(73);
                }

                list * argList = e->data.func_op.argsList;
                while(argList != NULL )
                {
                    exp_node* current = argList->head;
                    symbolResolutionExpr(current, global_types, function_decs,f);
                    argList= argList->next;
                }
                return;
            }
    case ter_exp:
        symbolResolutionExpr(e->data.ter_op.cond, global_types,function_decs,f);
        symbolResolutionExpr(e->data.ter_op.then, global_types,function_decs,f);
        symbolResolutionExpr(e->data.ter_op.elsestmt, global_types,function_decs,f);
        break;
        default:
            assert(0); // Should be dead code
    }
}

void symbolResolutionStmts(list * l, S_table locals, S_table function_decs, frame * f);

void symbolResolutionStmt(stmt_node * s, S_table global_types, S_table function_decs, frame * f) {

    if(!s) return;
    switch(s->kind){
        case if_stmt: {
            exp_node * cond_exp = s->data.if_ops.cond;
            list* then_list = s->data.if_ops.then_stmts;
            list * else_list = s->data.if_ops.else_stmts;

            symbolResolutionExpr(cond_exp,global_types, function_decs, f);
            symbolResolutionStmts(then_list, global_types,function_decs,f);
            symbolResolutionStmts(else_list, global_types,function_decs,f);

            break;
        }
        case while_stmt: {
                exp_node * cond_exp = s->data.while_ops.cond;
                list* then_list = s->data.while_ops.then_stmts;
                list * otherwise_list = s->data.while_ops.otherwise_stmt;

                symbolResolutionExpr(cond_exp,global_types, function_decs, f);
                symbolResolutionStmts(then_list, global_types,function_decs,f);
                symbolResolutionStmts(otherwise_list, global_types,function_decs,f);
                break;
        } case repeat_stmt: {
                exp_node * cond_exp = s->data.repeat_ops.cond;
                list* then_list = s->data.repeat_ops.then_stmts;

                symbolResolutionExpr(cond_exp,global_types, function_decs, f);
                symbolResolutionStmts(then_list, global_types,function_decs,f);
                break;
        }
        case ret_stmt: {

            exp_node * ret_exp = s->data.ret_exp;
            symbolResolutionExpr(ret_exp,global_types, function_decs, f);
            break;
        }
        case assign_stmt: {
                // make sure to remeber to check for func_dec
            if(f!= NULL)
            {
                ty_node * localResult= S_look(f->args_locs_types,s->data.assign_op.sym);
                if(localResult != NULL)
                {
                    symbolResolutionExpr(s->data.assign_op.assign_exp, global_types,function_decs,f);
                    break;
                }
                localResult = S_look(f->imp_table,s->data.assign_op.sym);
                if(localResult != NULL)
                {
                    symbolResolutionExpr(s->data.assign_op.assign_exp, global_types,function_decs,f);
                    break;
                }
            }
            ty_node * result = S_look(global_types,s->data.assign_op.sym);
            if (result == NULL)
            {
                fprintf(stderr, "variable is not declared  %s ", S_name(s->data.assign_op.sym));
                exit(73);
            }
            symbolResolutionExpr(s->data.assign_op.assign_exp, global_types,function_decs,f);
            break;
        }
    case func_stmt:
            {
                symbolResolutionExpr(s->data.func_op.funcExp, global_types,function_decs,f);
                break;
            }

        // TODO add more cases for all expressions
        default:
            assert(0); // Should be dead code
    }
}

void symbolResolutionStmts(list * l, S_table globals_types, S_table function_decs, frame * f) {
    if (l == NULL) return;
    symbolResolutionStmt(l->head, globals_types, function_decs, f);
    symbolResolutionStmts(l->next, globals_types, function_decs, f);
}

void symbolResolutionVariable(vardec_node * vardec, S_table globals_types, S_table function_decs, frame * f) {

    // TODO figure out if this is a global variable or a local variable
    // TODO Hint:  to start, just assume that everything is a global variable


    int global = 1;
    if(f != NULL)
    {
        global = 0;
    }
    symbolResolutionExpr(vardec->init,globals_types,function_decs,f);

    if(vardec->isImplicit)
    {
        return;
    }
    if (global) {
        // TODO register global variable types in globals_types
        S_symbol varSym = S_Symbol(vardec->name);

        if(S_look(globals_types,varSym) != NULL)
        {
            fprintf(stderr,"variable %s already declared\n", vardec->name);
            exit(73);
        }

        S_enter(globals_types, varSym, (void*)(vardec->type));


    } else {
        S_symbol varSym = S_Symbol(vardec->name);
        if(S_look(globals_types,varSym) != NULL || S_look(f->args_locs_types,varSym) != NULL)
        {
            fprintf(stderr,"variable %s already declared\n", vardec->name);
            exit(73);
        }

        S_enter(f->args_locs_types, varSym, vardec->type);
    }


    // TODO check that all symbols in the variable initialization are known to fail negative tests

    //assert(0); // Not implemented
}

void symbolResolutionVariables(list * l, S_table global_types, S_table function_decs, frame * f) {
    if (l == NULL) return;
    symbolResolutionVariable((vardec_node *)l->head, global_types, function_decs, f);

    symbolResolutionVariables(l->next, global_types, function_decs, f);
}

void symbolResolutionFunction(fundec_node * fundec, S_table globals, S_table functions_rets, frame * f, list * implicitVars) {

    if (S_look(functions_rets,S_Symbol(fundec->name)) != NULL)
    {
        fprintf(stderr,"function %s already declared\n", fundec->name);
        exit(73);
    }
    S_enter(functions_rets,S_Symbol(fundec->name), fundec);
    // TODO add each argument type to the f->args_locs_types




    list * arg_List = fundec->argList;

    while(arg_List != NULL)
    {
        param * curArg = arg_List->head;
        S_symbol argSymbol = S_Symbol(curArg->name);
        if (S_look(globals, argSymbol) != NULL || S_look(f->args_locs_types, argSymbol) != NULL)
        {
            fprintf(stderr,"function %s arguments: %s  already declared\n",fundec->name, curArg->name);
            exit(73);
        }

        S_enter(f->args_locs_types,argSymbol, curArg->ty);
        arg_List = arg_List ->next;

    }

    // TODO add each local variable type to f->args_locs_types
    list * local_list = fundec->localVarList;
    symbolResolutionVariables(local_list, globals, functions_rets,f);

    // implicits

   // printf("start implicit\n");
    while(implicitVars != NULL)
    {

        vardec_node * implicitVar = (vardec_node*)implicitVars->head;

        S_symbol implicitVarSym = S_Symbol(implicitVar->name);
        if(S_look(globals, implicitVarSym) != NULL || S_look(f->imp_table, implicitVarSym) != NULL)
        {
            fprintf(stderr,"variable %s already declared\n", implicitVar->name);
            exit(73);
        }

        //printf("end implicit\n");
        if(S_look(f->args_locs_types,implicitVarSym) != NULL)
        {
            implicitVars = implicitVars->next;
            continue;
        }
        //symbolResolutionExpr(implicitVar->init,globals,functions_rets,f);
        S_enter(f->imp_table, implicitVarSym,implicitVar->type );
        fundec->impList = ListAddLast(implicitVar,fundec->impList);
        //printf("I can print name here1: %s\n", ((vardec_node *)fundec->impList->head)->name);
        implicitVars = implicitVars->next;
    }

    // add i and j
    vardec_node *i = VarDecNode("i",IntTyNode(),IntNode(0),1);
    if (S_look(globals, S_Symbol(i->name)) == NULL && S_look(f->imp_table, S_Symbol(i->name)) == NULL && S_look(f->args_locs_types, S_Symbol(i->name)) == NULL)
    {
        //printf("you did\n");
        S_enter(f->imp_table, S_Symbol(i->name), IntTyNode() );
        fundec->impList = ListAddLast(i,fundec->impList);
    }


    // add i and j
    vardec_node *j = VarDecNode("j",IntTyNode(),IntNode(1),1);
    if (S_look(globals, S_Symbol(j->name)) == NULL && S_look(f->imp_table, S_Symbol(j->name)) == NULL && S_look(f->args_locs_types, S_Symbol(j->name)) == NULL)
    {
        S_enter(f->imp_table, S_Symbol(j->name), IntTyNode() );
        fundec->impList = ListAddLast(j,fundec->impList);
    }


    // TODO check that the statements in the body of the function only use known variables and functions
    symbolResolutionStmts(fundec->stmts,globals,functions_rets,f);
    f->ret=fundec->type;
}

void symbolResolutionFunctions(list * l, S_table global_types, S_table function_decs, S_table frames, list * implicitVars) {
    if (l == NULL) return;
    fundec_node * fundec = (fundec_node*) l->head;

    // Create a new frame
    frame * f = malloc(sizeof(frame));

    // Initialize all memory so we can read it later
    f->args_locs_types = S_empty();
    f->indexes = S_empty();
    f->ret = NULL;
    f->imp_table=S_empty();
    f->name=NULL;


    // Add newly computed frame to frames
    S_enter(frames, S_Symbol(fundec->name), f);

    symbolResolutionFunction(fundec, global_types, function_decs, f, implicitVars);
    symbolResolutionFunctions(l->next, global_types, function_decs, frames, implicitVars);
}

void symbolResolution(program * p, S_table global_types, S_table function_decs, S_table frames) {

    list *  implicitVars = NULL;
    list * iter = p->variables;
    while(iter!=NULL)
    {
        if(((vardec_node*)iter->head) ->isImplicit)
        {
            implicitVars = ListAddLast(iter->head,implicitVars);
        }
        iter = iter->next;
    }

    symbolResolutionVariables(p->variables, global_types, function_decs, NULL);
    symbolResolutionFunctions(p->functions, global_types, function_decs, frames, implicitVars);
    symbolResolutionStmts(p->statements, global_types, function_decs, NULL);

    list * freeder = implicitVars;
    while(freeder!=NULL)
    {
        list * current = freeder;
        freeder=freeder->next;
        free(current);
    }
}