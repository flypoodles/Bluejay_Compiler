#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interpret.h"

extern FILE *yyin;
extern int lexer();

static ir_node * ir = NULL;

int main(int argc, char **argv) {
 if (argc < 2) {fprintf(stderr,"usage: %s <program.ir> [-p]\n", argv[0]); return 1;}

 ir = ir_list_read(argv[1]);

 if (argc > 2 && !strcmp("-p", argv[2])) {
     ir_list_print(ir);
     return 0;
 }

 // interpret!
 int ret = interpret(ir);

 return ret;
}

