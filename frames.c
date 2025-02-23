#include <stdio.h>
#include "ast.h"
#include "frames.h"

void framesFunction(fundec_node * fundec, S_table globals, S_table functions_rets, S_table frames) {
    // Get frame created during symbol resolution
    __attribute__ ((unused))
        frame * f = S_look(frames, S_Symbol(fundec->name));

    f->name=fundec->name;
    long index = 1;
    list * argList = fundec->argList;
    while(argList!=NULL)
    {
        // TODO add each argument position to indexes
        param * node = (param *) argList->head;
        S_enter(f->indexes, S_Symbol(node->name), (void *)index);
        argList=argList->next;
        index++;
    }

    list * localList = fundec->localVarList;
    while(localList != NULL)
    {
        // TODO add each local variable position to indexes
        vardec_node * var = (vardec_node*)localList->head;
        printf("var name: %s\n",var->name);
        S_enter(f->indexes, S_Symbol(var->name), (void *)index);
        localList= localList->next;
        index++;
    }

    // TODO don't forget implicit variables!
    list * imp_list = fundec->impList;
    while(imp_list != NULL)
    {
        vardec_node * var = (vardec_node*)imp_list->head;
        //printf("I can print name here in frame : %s\n", var->name);
        S_enter(f->indexes, S_Symbol(var->name), (void *)index);
        imp_list= imp_list->next;
        index++;
    }
    printf("index at frame: %ld\n",index);

}

void framesFunctions(list * l, S_table global_types, S_table function_decs, S_table frames) {
    if (l == NULL) return;
    fundec_node * fundec = (fundec_node*) l->head;
    framesFunction(fundec, global_types, function_decs, frames);
    framesFunctions(l->next, global_types, function_decs, frames);
}

void frames(program * p, S_table global_types, S_table function_decs, S_table frames) {
    framesFunctions(p->functions, global_types, function_decs, frames);
}