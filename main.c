#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "ast_to_ir.h"
#include "ir.h"
#include "ir_tree_to_list.h"
#include "ir_list_to_mips.h"
#include "ir_pushpop.h"
#include "optimize.h"
#include "symbol.h"
#include "y.tab.h"
#include "semantic_analysis_symbols.h"
#include "semantic_analysis_types.h"
#include "frames.h"
#include "transform.h"

extern FILE *yyin;
extern int lexer();

static const char * usage = "usage: bluejaycc <in.bluejay> <out> [-ir] [-o]\n\t-ir Generate ir code instead of MIPS assembly\n\t-o enable the optimizer";

program p;

int main(int argc, char **argv) {
 char * fname;
 char * outname;

 int optimize = 0;
 int ir = 0;

 switch (argc) {
     case 5:
         if (!strcmp("-ir", argv[4]))     { ir = 1; }
         else if (!strcmp("-o", argv[4])) { optimize = 1; }
         else { fprintf(stderr,"%s\n", usage); return 1; }
         // fallthrough
     case 4:
         if (!strcmp("-ir", argv[3]))     { ir = 1; }
         else if (!strcmp("-o", argv[3])) { optimize = 1; }
         else { fprintf(stderr,"%s\n", usage); return 1;         }
         // fallthrough
     case 3:
         fname=argv[1];
         outname=argv[2];
         break;
     default:
         fprintf(stderr,"%s\n", usage);
         return 1;
 }

 p.variables = NULL;
 p.functions = NULL;
 p.statements = NULL;

 yyin = fopen(fname,"r");
 if (!yyin) { fprintf(stderr, "cannot open %s\n", fname); return 1; }


 int ret = yyparse();

 // Map from S_Symbol to ty_node
 S_table globals_types = S_empty();

 // Map from S_Symbol to ty_node
 S_table functions_decs = S_empty();

 // Map from S_Symbol to fun_frame
 S_table functions_frames = S_empty();
    // exit
    S_enter(
     functions_decs,
      S_Symbol("exit"),
      FunDecNode(
       "exit",
       VoidTyNode(),
       ListAddFirst(Param(IntTyNode(), "$$EXIT$$"),NULL),
       NULL,
       NULL
       )
     );

    // print
    S_enter(
     functions_decs,
     S_Symbol("printint"),
     FunDecNode(
      "printint",
      VoidTyNode(),
      ListAddFirst(Param(IntTyNode(), "$$print_int$$"),NULL),
      NULL,
      NULL
      )
     );

    S_enter(
     functions_decs,
     S_Symbol("printstring"),
     FunDecNode(
      "printstring",
      VoidTyNode(),
      ListAddFirst(Param(StrTyNode(), "$$print_string$$"),NULL),
      NULL,
      NULL
      )
     );

 // symbol analysis
 symbolResolution(&p, globals_types, functions_decs, functions_frames);

 // type checking
 typeCheck(&p, globals_types, functions_decs, functions_frames);

 // compute frames
 frames(&p, globals_types, functions_decs, functions_frames);

 if (optimize) { optimize_ast(&p, globals_types, functions_decs, functions_frames); }

 // transform AST as needed
 transform(&p, globals_types, functions_decs, functions_frames);

 // transform AST to IR tree
 ir_node * tree_ir = ast_to_ir(&p, globals_types, functions_decs, functions_frames);

 if (optimize) { optimize_tree_ir(tree_ir); }

 // transform IR tree to IR list
 ir_node * list_ir = ir_tree_to_list(tree_ir);

 if (optimize) { optimize_list_ir(list_ir); }

 if (ir) {
     // Generate IR file
     ir_list_write(list_ir, outname);
 } else {
     // Optional:  Add explicit push/pop IRs
     ir_node * pushpop = addPushPop(list_ir);
     // Generate MIPS file
     mips_ir(pushpop, outname);
 }

 // TODO free memory

 return ret;
}

