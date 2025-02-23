#define _POSIX_C_SOURCE 200809L
#include "ast_to_ir.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

__attribute__ ((unused))

char * freshVars() {
    static char buffer[1024];
    static int lastLabel = 0;

    snprintf(buffer, sizeof(buffer), "$repeat%d", lastLabel);
    lastLabel += 1;

    return strdup(buffer);
}
static ir_node * ast_to_ir_expr(exp_node * e, S_table global_types, S_table function_decs, frame * f) {

    if(!e) return Nop();
    switch(e->kind){
        case int_exp: {
                ir_node * ret = Iconst(e->data.ival);
            return ret;
        }
    case str_exp:
            {
                ir_node * ret = Sconst(e->data.strVal);
                return ret;
            }
        case binop_exp: {
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
            switch(e->data.bin_ops.op) {
                case plus_op: {
                        ir_node * bin = Ops(ir_add);
                        bin->tree_ir_1 = ast_to_ir_expr(e->data.bin_ops.e1,global_types, function_decs,f);
                        bin->tree_ir_2 = ast_to_ir_expr(e->data.bin_ops.e2,global_types, function_decs,f);
                        return bin;
                }
            case mul_op:
                    {
                        ir_node * bin = Ops(ir_mul);
                        bin->tree_ir_1 = ast_to_ir_expr(e->data.bin_ops.e1,global_types, function_decs,f);
                        bin->tree_ir_2 = ast_to_ir_expr(e->data.bin_ops.e2,global_types, function_decs,f);
                        return bin;
                    }
            case div_op:
                    {
                        ir_node * bin = Ops(ir_div);
                        bin->tree_ir_1 = ast_to_ir_expr(e->data.bin_ops.e1,global_types, function_decs,f);
                        bin->tree_ir_2 = ast_to_ir_expr(e->data.bin_ops.e2,global_types, function_decs,f);
                        return bin;
                    }
            case rem_op:
                    {
                        ir_node * bin = Ops(ir_mod);
                        bin->tree_ir_1 = ast_to_ir_expr(e->data.bin_ops.e1,global_types, function_decs,f);
                        bin->tree_ir_2 = ast_to_ir_expr(e->data.bin_ops.e2,global_types, function_decs,f);
                        return bin;
                    }
            case bor_op:
                    {
                        ir_node * bin = Ops(ir_bor);
                        bin->tree_ir_1 = ast_to_ir_expr(e->data.bin_ops.e1,global_types, function_decs,f);
                        bin->tree_ir_2 = ast_to_ir_expr(e->data.bin_ops.e2,global_types, function_decs,f);
                        return bin;
                    }
            case band_op:
                    {
                        ir_node * bin = Ops(ir_band);
                        bin->tree_ir_1 = ast_to_ir_expr(e->data.bin_ops.e1,global_types, function_decs,f);
                        bin->tree_ir_2 = ast_to_ir_expr(e->data.bin_ops.e2,global_types, function_decs,f);
                        return bin;
                    }
            case xor_op:
                    {
                        ir_node * bin = Ops(ir_xor);
                        bin->tree_ir_1 = ast_to_ir_expr(e->data.bin_ops.e1,global_types, function_decs,f);
                        bin->tree_ir_2 = ast_to_ir_expr(e->data.bin_ops.e2,global_types, function_decs,f);
                        return bin;
                    }
            case gt_op:
                    {
                        ir_node * bin = Ops(ir_gt);
                        bin->tree_ir_1 = ast_to_ir_expr(e->data.bin_ops.e1,global_types, function_decs,f);
                        bin->tree_ir_2 = ast_to_ir_expr(e->data.bin_ops.e2,global_types, function_decs,f);
                        return bin;
                    }
            case lt_op:
                    {
                        ir_node * bin = Ops(ir_lt);
                        bin->tree_ir_1 = ast_to_ir_expr(e->data.bin_ops.e1,global_types, function_decs,f);
                        bin->tree_ir_2 = ast_to_ir_expr(e->data.bin_ops.e2,global_types, function_decs,f);
                        return bin;
                    }
            case or_op:
                    {
                        ir_node * bin = Ops(ir_or);
                        bin->tree_ir_1 = ast_to_ir_expr(e->data.bin_ops.e1,global_types, function_decs,f);
                        bin->tree_ir_2 = ast_to_ir_expr(e->data.bin_ops.e2,global_types, function_decs,f);
                        return bin;
                    }
            case and_op:
                    {

                        ir_node * bin = Ops(ir_and);
                        bin->tree_ir_1 = ast_to_ir_expr(e->data.bin_ops.e1,global_types, function_decs,f);
                        bin->tree_ir_2 = ast_to_ir_expr(e->data.bin_ops.e2,global_types, function_decs,f);
                        return bin;
                    }
            case eq_op:
                    {
                        ir_node * bin = Ops(ir_eq);
                        bin->tree_ir_1 = ast_to_ir_expr(e->data.bin_ops.e1,global_types, function_decs,f);
                        bin->tree_ir_2 = ast_to_ir_expr(e->data.bin_ops.e2,global_types, function_decs,f);
                        return bin;
                    }
            case ne_op: {assert(0);}
            case ge_op : {assert(0);}
            case le_op : {assert(0);}
            case minus_op:{assert(0);} //
                default:
                    assert(0);
            }
            assert(0);
        }
        case unop_exp: {
            ir_node * uno = Ops(ir_not);
            uno->tree_ir_1 = ast_to_ir_expr(e->data.un_ops.e,global_types,function_decs,f);
                return uno;
        }
    case var_exp:
            {

                long index = 0;
                if (f != NULL)
                {
                    index = (long)S_look(f->indexes, e->data.varVal);

                }
                ir_node * var = (index== 0) ?  Read(S_name(e->data.varVal)) : ArgLocalRead(index-1);

                return var;
            }
    case func_exp:
            {
//                assert(e->data.func_op.argsList==NULL);
                list * l = e->data.func_op.argsList;
                ir_node * argList = Nop();
                int length = 0;
                while( l != NULL)
                {
                    exp_node * curArg = (exp_node *)l->head;
                    ir_node * curArgIr = ast_to_ir_expr(curArg,global_types, function_decs,f);
                    argList = Seq(argList,curArgIr);
                    l = l->next;
                    length++;
                }
                ir_label * functionLabel = namedLabel(e->data.func_op.name);
                ir_node * funCall = Call(functionLabel, length);

                funCall->tree_ir_1=Iconst(69420);
                funCall->tree_ir_2=argList;
                return funCall;
            }
    case void_exp:
            {
                return Iconst(0);
            }
    case ter_exp:
            {
                ir_label * ElseLabel =  freshLabel();
                ir_label * endIfLabel = freshLabel();
                ir_node * ret = Branch(ElseLabel);
                ir_node * cond = ast_to_ir_expr(e->data.ter_op.cond,global_types,function_decs,f);
                ir_node * then =  ast_to_ir_expr(e->data.ter_op.then,global_types,function_decs,f);
                ir_node * elseStmts = ast_to_ir_expr(e->data.ter_op.elsestmt,global_types,function_decs,f);
                ret ->tree_ir_1 = cond;
                ret -> tree_ir_2 = Seq(then, Jump(endIfLabel));
                ret->tree_ir_3=Seq(Seq(Label(ElseLabel), elseStmts), Label(endIfLabel));
                return ret;
            }
        default:
            assert(0); // Dead code
    }
}

static ir_node * ast_to_ir_stmts(list * l, S_table globals_types, S_table function_decs, frame * f);

static ir_node * ast_to_ir_stmt(stmt_node * s, S_table global_types, S_table function_decs, frame * f) {
    if(!s) return Nop();
    switch(s->kind){
        case if_stmt: {
            ir_label * ElseLabel =  freshLabel();
            ir_label * endIfLabel = freshLabel();
            ir_node * ret = Branch(ElseLabel);
            ir_node * cond = ast_to_ir_expr(s->data.if_ops.cond,global_types,function_decs,f);
            ir_node * then =  ast_to_ir_stmts(s->data.if_ops.then_stmts,global_types,function_decs,f);
            ir_node * elseStmts = ast_to_ir_stmts(s->data.if_ops.else_stmts,global_types,function_decs,f);
            ret ->tree_ir_1 = cond;
            ret -> tree_ir_2 = Seq(then, Jump(endIfLabel));
            ret->tree_ir_3=Seq(Seq(Label(ElseLabel), elseStmts), Label(endIfLabel));
            return ret;
        }

        case ret_stmt: {
                //TODO deal with return within function. On global treat return as exit
                if(f!=NULL)
                {
                    ir_node * ret = Return();
                    fundec_node * funcdec = S_look(function_decs, S_Symbol(f->name));
                    int index = 0;
                    list * argList = funcdec->argList;
                    while(argList!=NULL)
                    {
                        argList=argList->next;
                        index++;
                    }
                    printf("index after args: %d\n",index);

                    list * localList = funcdec->localVarList;
                    while(localList != NULL)
                    {
                        //vardec_node * var = (vardec_node*)localList->head;
                        //printf("var name: %s\n",var->name);
                        localList= localList->next;
                        index++;
                    }
                   // printf("index : %d\n",index);


                    ret->data.iconst=index;
                    ret ->tree_ir_1=ast_to_ir_expr(s->data.ret_exp,global_types,function_decs,f);
                    return ret;
                }
            ir_node * ret = Intrinsic(intrinsic_exit);;
             ir_node * arg = ast_to_ir_expr(s->data.ret_exp, global_types, function_decs, f);
                ret ->tree_ir_1 =arg;
                return ret;
        }
    case func_stmt:
            {
                if (!strcmp("exit",s->data.func_op.funcExp->data.func_op.name))
                {
                    // handle exit;
                    ir_node * ret = Intrinsic(intrinsic_exit);
                    ir_node * arg = ast_to_ir_expr(s->data.func_op.funcExp->data.func_op.argsList->head, global_types, function_decs, f);
                    ret -> tree_ir_1 = arg;
                    return ret;
                }
                if (!strcmp("printint",s->data.func_op.funcExp->data.func_op.name))
                {
                    // handle print int;
                    ir_node * ret = Intrinsic(intrinsic_print_int);
                    ir_node * arg = ast_to_ir_expr(s->data.func_op.funcExp->data.func_op.argsList->head, global_types, function_decs, f);
                    ret -> tree_ir_1 = arg;
                    return ret;
                }
                if (!strcmp("printstring",s->data.func_op.funcExp->data.func_op.name))
                {
                    // handle print int;
                    ir_node * ret = Intrinsic(intrinsic_print_string);
                    ir_node * arg = ast_to_ir_expr(s->data.func_op.funcExp->data.func_op.argsList->head, global_types, function_decs, f);
                    ret -> tree_ir_1 = arg;
                    return ret;
                }
                ir_node * ret = ast_to_ir_expr(s->data.func_op.funcExp,global_types,function_decs,f);
                return ret;
            }
    case assign_stmt:
            {

                if(f != NULL)
                {
                    long index = (long)S_look(f->indexes,s->data.assign_op.sym );
                    if(index != 0){

                        ir_node * ret = ArgLocalWrite(index-1);
                        ir_node * val = ast_to_ir_expr(s->data.assign_op.assign_exp,global_types,function_decs,f);
                        ret->tree_ir_1 = val;
                        return ret;
                    }

                }
                ir_node * ret =Write(S_name(s->data.assign_op.sym));
                ir_node * val = ast_to_ir_expr(s->data.assign_op.assign_exp,global_types,function_decs,f);

                ret->tree_ir_1 = val;
                return ret;
            }
    case while_stmt:
            {
                ir_label * guardLabel = freshLabel();
                ir_label * endWhileLabel= freshLabel();

                ir_node * branch = Branch(endWhileLabel);
                ir_node * cond =ast_to_ir_expr(s->data.while_ops.cond,global_types,function_decs,f);
                ir_node * stmts = ast_to_ir_stmts(s->data.while_ops.then_stmts,global_types,function_decs,f);
                branch ->tree_ir_1 = Seq(Label(guardLabel),cond);
                branch->tree_ir_2 = Seq(Seq(stmts,Jump(guardLabel)),Label(endWhileLabel));
                return branch;
            }
    case repeat_stmt:
            {

                exp_node* cond = s->data.repeat_ops.cond;
                list * stmts = s->data.repeat_ops.then_stmts;



                char * name = freshVars();
                ir_node * head = Reserve(4, name,NULL);
                ir_node * val = ast_to_ir_expr(cond, global_types,function_decs,f);
                ir_node * initializing = Write(name);
                head ->tree_ir_1 = initializing;
                initializing ->tree_ir_1 = val;


                ir_node *stmtsIr = ast_to_ir_stmts(stmts,global_types,function_decs,f);
                // create a condition for while loop
                ir_node * condIr = Ops(ir_lt);
                condIr ->tree_ir_1 = Iconst(0);
                condIr -> tree_ir_2 = Read(name);

                // create a decrement statment
                ir_node * subtract = Ops(ir_add); // need to be add bc we dont have subtract
                subtract ->tree_ir_1 = Iconst(-1);
                subtract ->tree_ir_2 = Read(name);

                // writ to auxilary
                ir_node * writeToAux = Write(name);
                writeToAux ->tree_ir_1=subtract;

                ir_label * guardLabel = freshLabel();
                ir_label * endWhileLabel= freshLabel();

                ir_node * branch = Branch(endWhileLabel);
                branch ->tree_ir_1 = Seq(Label(guardLabel),condIr);
                branch->tree_ir_2 = Seq(Seq(Seq(stmtsIr,writeToAux),Jump(guardLabel)),Label(endWhileLabel));


                return Seq(head,branch);

            }
        default:
            assert(0); // Dead code
    }
}

static ir_node * ast_to_ir_stmts(list * l, S_table globals_types, S_table function_decs, frame * f) {
    if (l == NULL)
        return Nop();

    __attribute__ ((unused))
    ir_node * ir1 = ast_to_ir_stmt(l->head, globals_types, function_decs, f);

    __attribute__ ((unused))
    ir_node * ir2 = ast_to_ir_stmts(l->next, globals_types, function_decs, f);

    // TODO combine ir1 with ir2
    ir_node * ret = Seq(ir1,ir2);
    return ret;
}




static ir_node * ast_to_ir_functions(list * l, S_table global_types, S_table function_decs, S_table frames) {

    ir_node * ret = Nop();
    while (l != NULL) {
        __attribute__ ((unused))
        fundec_node * fundec = (fundec_node*) l->head;

        frame * f = S_look(frames, S_Symbol(fundec->name));
        assert(f != NULL);

        int totalLocal = 0;
        list * localIter = fundec->localVarList;

        ir_node * localIr = Nop();
        while(localIter!= NULL)
        {
            vardec_node * curLocal = localIter->head;

            long curIndex = (long)S_look(f->indexes, S_Symbol(curLocal->name)) - 1;
            ir_node* curIr = ArgLocalWrite(curIndex);
            ir_node * init = ast_to_ir_expr(curLocal->init,global_types,function_decs,f);
            curIr -> tree_ir_1 = init;

            localIr = Seq(localIr, curIr);

            totalLocal++;
            localIter = localIter->next;
        }
        // TODO handle function definitions here
        ir_label * functionLabel = namedLabel(fundec->name);
        ir_node * funcNode = Function(functionLabel, totalLocal);

        funcNode ->tree_ir_2 = localIr;
        funcNode->tree_ir_3 = ast_to_ir_stmts(fundec->stmts,global_types,function_decs,f);

        ret = Seq(ret,funcNode);
        l = l->next;
    }

    return ret;
}

static ir_node * ast_to_ir_vars (list * l, S_table global_types, S_table function_decs, frame * f) {

    ir_node * ret = Nop();
    assert(f==NULL);
    while (l != NULL) {
        __attribute__ ((unused))
        vardec_node * var = (vardec_node*) l->head;

        // TODO Handle variable declarations here



        if (var->type == IntTyNode())
        {
            ir_node * head = Reserve(4, var->name,NULL);
            ir_node * val = ast_to_ir_expr(var->init, global_types,function_decs,f);
            ir_node * initializing = Write(var->name);
            head ->tree_ir_1 = initializing;
            initializing ->tree_ir_1 = val;
            ret = Seq(ret, head);

        } else if (var->type == StrTyNode())
        {

            ir_node * head = Reserve(strlen(var->init->data.strVal)+1, var->name, var->init->data.strVal);
            ret = Seq(ret, head);

        } else
        {
            assert(0);
        }

        l = l->next;
    }

    return ret;
}

ir_node * ast_to_ir(program* p, S_table globals_types, S_table functions_ret, S_table frames) {

    __attribute__ ((unused))
    ir_node * vars = ast_to_ir_vars(p->variables, globals_types, functions_ret, NULL);

    __attribute__ ((unused))
    ir_node * functions = ast_to_ir_functions(p->functions, globals_types, functions_ret, frames);

    ir_node * stmts = ast_to_ir_stmts(p->statements, globals_types, functions_ret, NULL);


    // TODO combine vars, functions, and stmts
    ir_node * wholeProgram = Seq(Seq(vars, stmts),functions);
    return wholeProgram;
}