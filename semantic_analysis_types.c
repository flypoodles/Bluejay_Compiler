#include <stdlib.h>
#include <assert.h>
#include "semantic_analysis_types.h"

#include <stdio.h>

ty_node * typeCheckExpr(exp_node * e, S_table global_types, S_table function_decs, frame * f)
{
    switch(e->kind){
    case int_exp: {
            return IntTyNode();
    }
    case binop_exp: {
            ty_node * expect = IntTyNode();
            ty_node * left  = typeCheckExpr(e->data.bin_ops.e1,global_types,function_decs,f);
            ty_node * right  = typeCheckExpr(e->data.bin_ops.e2,global_types,function_decs,f);
            if(left != expect || right != expect)
            {
                fprintf(stderr, "wrong type for binary operation expected %s for both expression, got left: %s right: %s\n", typeToStr(expect), typeToStr(left),typeToStr(right));
                exit(73);
            }
            return expect;
    }
    case unop_exp: {

            ty_node * expect = IntTyNode();
            ty_node * actual  = typeCheckExpr(e->data.un_ops.e,global_types,function_decs,f);
            if(actual != expect)
            {
                fprintf(stderr, "wrong type for unary expected %s got %s\n", typeToStr(expect), typeToStr(actual));
                exit(73);
            }
            return actual;
    }
    case str_exp: {
            return StrTyNode();
    }
    case void_exp:
        {
            return VoidTyNode();
        }
    case var_exp:
        {

            if(f!= NULL)
            {
                ty_node * localResult = (ty_node *)(S_look(f->args_locs_types, e->data.varVal));
                if(localResult!= NULL)
                {
                    return localResult;
                }
                localResult = (ty_node *)(S_look(f->imp_table, e->data.varVal));
                if(localResult!= NULL)
                {
                    return localResult;
                }

            }
            // make sure to remeber to check for func_dec
            ty_node * result = (ty_node *)(S_look(global_types, e->data.varVal));

            return result;
        }
    case func_exp:
        {
            fundec_node * result = (fundec_node *)(S_look(function_decs, S_Symbol(e->data.func_op.name)));


            list * argList = result->argList;
            list * callList = e->data.func_op.argsList;

            while(argList != NULL && callList != NULL)
            {

                param * expectParam = argList->head;
                exp_node * cur_param = callList->head;

                ty_node* actual = typeCheckExpr(cur_param, global_types, function_decs, f);
                if(actual != expectParam->ty)
                {
                    fprintf(stderr, "wrong type for function's argument expected %s got %s\n", typeToStr(expectParam->ty), typeToStr(actual));
                    exit(73);
                }
                callList = callList->next;
                argList = argList ->next;
            }

            if (argList != NULL || callList != NULL)
            {
                fprintf(stderr, "wrong size for the function's argument");
                exit(73);
            }
            return result->type;
            // check for args

        }


    case ter_exp: {
        // check cond is int
            ty_node * expect = IntTyNode();
        ty_node * actual = typeCheckExpr(e->data.ter_op.cond, global_types,function_decs,f);
        if(expect != actual)
        {
            fprintf(stderr, "wrong type for condition expected %s got %s\n", typeToStr(expect), typeToStr(actual));
            exit(73);
        }

        // make sure then and else shares the same type
        ty_node * then = typeCheckExpr(e->data.ter_op.then, global_types,function_decs,f);
        ty_node * elseStmt = typeCheckExpr(e->data.ter_op.elsestmt, global_types,function_decs,f);

        if(then != elseStmt)
        {
            fprintf(stderr, "Type mismatch for ternary expression then %s else %s\n", typeToStr(then), typeToStr(elseStmt));
            exit(73);
        }
            return then;
    }
       default:
            assert(0); // Should be dead code
    }
}

void typeCheckStmts(list * l, S_table locals, S_table function_decs, frame * f);

void typeCheckStmt(stmt_node * s, S_table global_types, S_table function_decs, frame * f) {
    if(!s) return;
    switch(s->kind){
        case if_stmt: {
                ty_node * expect = IntTyNode();
                ty_node * actual  = typeCheckExpr(s->data.if_ops.cond,global_types,function_decs,f);
                if(expect != actual)
                {
                    fprintf(stderr, "wrong type for if expected %s got %s\n", typeToStr(expect), typeToStr(actual));
                    exit(73);
                }
                typeCheckStmts(s->data.if_ops.then_stmts,global_types,function_decs,f);
                typeCheckStmts(s->data.if_ops.else_stmts,global_types,function_decs,f);
            break;
        }
    case while_stmt:
            {
                ty_node * expect = IntTyNode();
                ty_node * actual  = typeCheckExpr(s->data.while_ops.cond,global_types,function_decs,f);
                if(expect != actual)
                {
                    fprintf(stderr, "wrong type for while expected %s got %s\n", typeToStr(expect), typeToStr(actual));
                    exit(73);
                }
                typeCheckStmts(s->data.while_ops.then_stmts,global_types,function_decs,f);
                typeCheckStmts(s->data.while_ops.otherwise_stmt,global_types,function_decs,f);
                break;
            }
    case repeat_stmt:{
                ty_node * expect = IntTyNode();
                ty_node * actual  = typeCheckExpr(s->data.repeat_ops.cond,global_types,function_decs,f);
                if(expect != actual)
                {
                    fprintf(stderr, "wrong type for repeat expected %s got %s\n", typeToStr(expect), typeToStr(actual));
                    exit(73);
                }
                typeCheckStmts(s->data.repeat_ops.then_stmts,global_types,function_decs,f);
                break;

            }
        case ret_stmt: {
                // assume return only return integer for Now. Change it later !!!

                ty_node * expect = IntTyNode();
                if(f!=NULL)
                {
                    expect = f->ret;
                }
                ty_node * actual  = typeCheckExpr(s->data.ret_exp,global_types,function_decs,f);
                if(expect != actual)
                {
                    fprintf(stderr, "wrong type for return expected %s got %s\n", typeToStr(expect), typeToStr(actual));
                    exit(73);
                }
            break;
        }
        case assign_stmt: {
                // make sure to remeber to check for func_dec
                if(f!=NULL)
                {
                    ty_node * localExpect = S_look(f->args_locs_types, s->data.assign_op.sym);
                    if(localExpect !=NULL)
                    {
                        ty_node * actual = typeCheckExpr(s->data.assign_op.assign_exp, global_types, function_decs,f);

                        if(localExpect != actual)
                        {
                            fprintf(stderr, "wrong type for assignment expected %s got %s\n", typeToStr(localExpect), typeToStr(actual));
                            exit(73);
                        }
                        break;
                    }
                    localExpect = S_look(f->imp_table, s->data.assign_op.sym);
                    if(localExpect !=NULL)
                    {
                        ty_node * actual = typeCheckExpr(s->data.assign_op.assign_exp, global_types, function_decs,f);

                        if(localExpect != actual)
                        {
                            fprintf(stderr, "wrong type for assignment expected %s got %s\n", typeToStr(localExpect), typeToStr(actual));
                            exit(73);
                        }
                        break;
                    }
                }
                ty_node * expect = S_look(global_types, s->data.assign_op.sym);
                ty_node * actual = typeCheckExpr(s->data.assign_op.assign_exp, global_types, function_decs,f);

                if(expect != actual)
                {
                    fprintf(stderr, "wrong type for assignment expected %s got %s\n", typeToStr(expect), typeToStr(actual));
                    exit(73);
                }
                break;
        } case func_stmt:
            {
                typeCheckExpr(s->data.func_op.funcExp, global_types, function_decs,f);
                break;
            }
        // TODO add more cases for all expressions
        default:
            assert(0); // Should be dead code
    }
}

void typeCheckStmts(list * l, S_table globals_types, S_table function_decs, frame * f) {
    if (l == NULL) return;
    typeCheckStmt(l->head, globals_types, function_decs, f);
    typeCheckStmts(l->next, globals_types, function_decs, f);
}

void typeCheckVariable(vardec_node * vardec, S_table globals_types, S_table function_decs, frame * f) {
    // TODO ensure that variable initialization expression has the same type as the variable definition

    ty_node * actual = typeCheckExpr(vardec->init, globals_types,function_decs, f);
    ty_node * expected = vardec->type;

    if(actual != expected)
    {
        fprintf(stderr, "wrong type for variable expected %s got %s\n", typeToStr(expected), typeToStr(actual));
        exit(73);
    }

}

void typeCheckVariables(list * l, S_table global_types, S_table function_decs, frame * f) {
    if (l == NULL) return;
    typeCheckVariable((vardec_node *)l->head, global_types, function_decs, f);
    typeCheckVariables(l->next, global_types, function_decs, f);
}

void typeCheckFunction(fundec_node * fundec, S_table globals_types, S_table functions_rets, frame * f) {
    // TODO ensure that the body of the function is well typed
    // TODO ensure local variables are initialized with the correct type
    typeCheckVariables(fundec->localVarList, globals_types,functions_rets,f);
    typeCheckStmts(fundec->stmts,globals_types,functions_rets,f);

}

void typeCheckFunctions(list * l, S_table global_types, S_table function_decs, S_table frames) {
    if (l == NULL) return;
    fundec_node * fundec = (fundec_node*) l->head;
    frame * f = S_look(frames, S_Symbol(fundec->name));
    assert(f);
    typeCheckFunction(fundec, global_types, function_decs, f);
    typeCheckFunctions(l->next, global_types, function_decs, frames);
}

void typeCheck(program * p, S_table global_types, S_table function_decs, S_table frames) {
    typeCheckVariables(p->variables, global_types, function_decs, NULL);
    typeCheckFunctions(p->functions, global_types, function_decs, frames);
    typeCheckStmts(p->statements, global_types, function_decs, NULL);
}