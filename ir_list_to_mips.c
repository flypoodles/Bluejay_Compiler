#define _POSIX_C_SOURCE 200809L // enable strdup

#include "ir_list_to_mips.h"

#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static FILE * out;

__attribute__ ((unused))
static void emitLabel(char * label, char * comment, ...) {
    static char buffer[1024];
    va_list argp;

    assert(comment != NULL && strcmp("",comment));
    assert(label != NULL);

    va_start(argp, comment);
    vsnprintf(buffer, sizeof(buffer), label, argp);
    va_end(argp);
    fprintf(out, "%s:\n", buffer);
    fflush(out);
}

__attribute__ ((unused))
static void emitInstruction(char * instruction, char * comment, ...) {
    static char buffer[1024];

    va_list argp;
    assert(comment != NULL && strcmp("",comment));
    assert(instruction != NULL);

    va_start(argp, comment);
    vsnprintf(buffer, sizeof(buffer), instruction, argp);
    va_end(argp);
    fprintf(out, "\t\t%s\t\t\t# %s\n", buffer, comment);
    fflush(out);
}

char * fresgStrLabel() {
    static char buffer[1024];
    static int lastLabel = 0;

    snprintf(buffer, sizeof(buffer), "Str%d", lastLabel);
    lastLabel += 1;

    return strdup(buffer);
}

static void mips_ir_translate(ir_node * ir) {
    if (ir == NULL) {
        return;
    }

    switch (ir->kind) {
        case ir_nop:            { break; }
        case ir_iconst:         {
                                    emitInstruction("li $v0, %d", "ICONST load immediate to a register", ir->data.iconst);
                                    emitInstruction("sw $v0, ($sp)", "ICONST store to the top of the stack");
                                    emitInstruction("addiu $sp, $sp, -4", "ICONST point sp to the next free space");
                                    break;
                                }
        case ir_sconst:
            {
                emitInstruction("la $v0, %s", "STRCONST: load the variable's address to the register v1", ir->data.sconst);
                emitInstruction("sw $v0, ($sp)", "STRCONST: store the content of v0 to top of the stack");
                emitInstruction("addiu $sp, $sp, -4", "STRCONST: point stack pointer  to the next free space");
                break;
            }
        case ir_add:            { emitInstruction("add $v0, $v0, $v1", "ADD:  v0 = v0 + v1 "); break; }
        case ir_sub:            { assert(0); }
        case ir_mul:            { emitInstruction("mul $v0, $v0, $v1", "multiply:  v0 = v0 * v1 "); break; }
        case ir_div:            {
                                emitInstruction("div $v0, $v1", "divide:  $lo = v0 / v1 ");
                                emitInstruction("mflo $v0", "divide:  copy from lo to v0");
                                break;
        }
        case ir_mod:            {
                emitInstruction("div $v0, $v1", "MOD:  $hi = v0 % v1 ");
                emitInstruction("mfhi $v0", "MOD:  copy from hi to v0");
                break;
 }
        case ir_bor:            { emitInstruction("or $v0, $v0, $v1", "BOR:  v0 = v0 | v1 "); break; }
        case ir_band:           { emitInstruction("and $v0, $v0, $v1", "BAND:  v0 = v0 & v1 "); break; }
        case ir_xor:            { emitInstruction("xor $v0, $v0, $v1", "XOR:  v0 = v0 ^ v1 "); break; }
        case ir_or:
            {
                emitInstruction("sne $v0, $v0 , $zero", "OR:  turn v0 into 1 if v0 != 0");
                emitInstruction("sne $v1, $v1 , $zero", "OR:  turn v1 into 1 if v1 != 0");
                emitInstruction("or $v0, $v0, $v1", "OR:  v0 = v0 | v1 ");

                break;
            }
        case ir_and:            {
                emitInstruction("sne $v0, $v0 , $zero", "AND:  turn v0 into 1 if v0 != 0");
                emitInstruction("sne $v1, $v1 , $zero", "AND:  turn v1 into 1 if v1 != 0");
                emitInstruction("and $v0, $v0, $v1", "AND:  v0 = v0 & v1 ");
                break;
}
        case ir_eq:             { emitInstruction("seq $v0, $v0, $v1", "Equality:  v0 = v0 == v1 "); break;  }
        case ir_lt:             { emitInstruction("slt $v0, $v0, $v1", "LESS than:  v0 = v0 < v1 "); break;  }
        case ir_gt:             { emitInstruction("sgt $v0, $v0, $v1", "GREATER THAN:  v0 = v0  > v1 "); break; }
        case ir_not:            {

                                    emitInstruction("seq $v0, $v0 , $zero", "NOT:  v0 = ~v0"); break;
                                }
        case ir_reserve:
            {
                /*
                * char * freshlabel = fresgStrLabel();
                    emitInstruction("%s: .asciiz \"%s\"", "RESERVE:RESERVE a space for string constant", freshlabel, ir->data.reserve.val);

                    emitInstruction("%s: .word %d", "RESERVE a space to store  the address of the string const", ir->data.reserve.name, 0);
                    ir->data.reserve.val=freshlabel;
                 */

                if(ir->data.reserve.val !=NULL)
                {
                    emitInstruction("la $v1, %s", "RESERVE: load the variable's address to the register v1", ir->data.reserve.val);
                    emitInstruction("la $v0, %s", "RESERVE: load the variable's address to the register v1", ir->data.reserve.name);
                    emitInstruction("sw $v1, ($v0)", "RESERVE: write the value popped from top of the stack to the address");

                }


                 break;
            }
        case ir_read:
            {

                emitInstruction("la $v1, %s", "READ: load the variable's address to the register v1", ir->data.sconst);
                emitInstruction("lw $v0, ($v1)", "READ: store the content of v1 to v0");
                break;
            }
        case ir_write:
            {
                emitInstruction("la $v1, %s", "WRITE: load the variable's address to the register v1", ir->data.sconst);
                emitInstruction("sw $v0, ($v1)", "WRITE: write the value popped from top of the stack to the address");
                break;
            }
    case ir_arglocal_read:  {
                int index = ir->data.iconst;
                int location = -1*(index * 4  + 4);
                emitInstruction("lw $v0, %d($fp)", "load the local argument from the stack frame to register v0", location);
                emitInstruction("sw $v0, ($sp)", " store v0 to the top of the stack");
                emitInstruction("addiu $sp, $sp, -4", " point stack pointer  to the next free space");
                break;
    }
        case ir_arglocal_write:
            {

                int index = ir->data.iconst;
                int location = -1*(index * 4  + 4);
                emitInstruction("addiu $sp, $sp, 4", "POP: Subtract sp ");
                emitInstruction("lw $v0, ($sp)", "POP: pop the content of the stack to v0");
                emitInstruction("sw $zero, ($sp)", "Zero the stack");
                emitInstruction("sw $v0, %d($fp)", "store the value from v0 to the local var", location);
                break;
            }
        case ir_lbl:            { emitLabel(ir->data.lbl->name, "LBL"); break;}
        case ir_jump:           { emitInstruction("j %s", "JUMP: Jump to Label", ir->data.lbl->name); break;}
        case ir_branchzero:     {
                emitInstruction("addiu $sp, $sp, 4", "BRANCH: Subtract sp ");
                emitInstruction("lw $v0, ($sp)", "BRANCH: pop the content of the stack to v0");
                emitInstruction("sw $zero, ($sp)", "BRANCH: Zero the stack");
                emitInstruction("beq $zero, $v0, %s", "BRANCH: Jump if v0 == 0", ir->data.lbl->name);
                break;
  }
        case ir_call:
            {
                emitInstruction("nop", "starting call: ");


                int begin = (ir->data.call_function.vars * 4) +4;
                emitInstruction("sw $fp, %d($sp)", "PUSH fp on top of the stack", begin);
                emitInstruction("addiu $fp, $sp, %d", "point frame pointer to the begining of the stack frame",begin);
                emitInstruction("jal %s", "jump to the function label", ir->data.call_function.lbl->name);

                emitInstruction("nop", "Ending call: ");
                break;
            }
        case ir_function:
            {
                emitInstruction("nop", "starting Function: ");
                emitLabel(ir->data.call_function.lbl->name, "Function Label");

                int numLocal = -1 * (ir->data.call_function.vars * 4);
                emitInstruction("addiu $sp, $sp, %d", "reserve space for local variables",numLocal);

                emitInstruction("sw $ra, ($sp)", "PUSH return address to the top of the stack");
                emitInstruction("addiu $sp, $sp, -4", "PUSH: point stack pointer  to the next free space");
                emitInstruction("nop", "Ending Function: ");
                break;
            }
        case ir_ret:            {
                emitInstruction("nop", "starting RETURN: ");
                emitInstruction("addiu $sp, $sp, 4", "POP: Subtract sp ");
                emitInstruction("lw $v0, ($sp)", "pop the return value to v0");
                emitInstruction("sw $zero, ($sp)", "Zero the stack");
                emitInstruction("addiu $sp, $sp, 4", "POP: Subtract sp ");


                int location = (ir->data.iconst * 4 + 4) * -1;
                emitInstruction("lw $ra, %d($fp)", "get the return address from fp to ra",location);
                emitInstruction("sw $zero, ($sp)", "Zero the stack");
                emitInstruction("move $sp, $fp", "reset sp");
                emitInstruction("lw $fp, ($fp)", "reset fp");
                emitInstruction("sw $zero, ($sp)", "clean the fp off the stack since fp already stores it");
                emitInstruction("sw $v0, ($sp)", " push the return value  back to the top of the stack");
                emitInstruction("addiu $sp, $sp, -4", "PUSH: point stack pointer  to the next free space");
                emitInstruction("jr $ra", "jumping back");
                emitInstruction("nop", "Ending RETURN: ");
                break;
}
        case ir_intrinsic:
            {
                switch (ir->data.intrinsic)
                {
                case intrinsic_exit:
                    /*POP*/
                        emitInstruction("addiu $sp, $sp, 4", "EXIT: Subtract sp ");
                        emitInstruction("lw $a0, ($sp)", "EXIT: pop the content of the stack to a0");
                        emitInstruction("sw $zero, ($sp)", "Zero the stack");
                        emitInstruction("li  $v0, 17", "EXIT load syscall number");
                        emitInstruction("syscall", "EXIT syscall");

                        break;
                case intrinsic_print_int:
                    emitInstruction("addiu $sp, $sp, 4", "PRINTINT: Subtract sp ");
                    emitInstruction("lw $a0, ($sp)", "PRINTINT: pop the content of the stack to a0");
                    emitInstruction("sw $zero, ($sp)", "Zero the stack");
                    emitInstruction("li  $v0, 1", "PRINTINT load syscall number");
                    emitInstruction("syscall", "PRINTINT syscall");
                    break;
                case intrinsic_print_string:
                    emitInstruction("addiu $sp, $sp, 4", "PRINTSTR: Subtract sp ");
                    emitInstruction("lw $a0, ($sp)", "PRINTSTR: pop the content of the stack to a0");
                    emitInstruction("sw $zero, ($sp)", "Zero the stack");
                    emitInstruction("li  $v0, 4", "PRINTSTR load syscall number");
                    emitInstruction("syscall", "PRINTSTR syscall");
                    break;
                    default:
                        assert(0);
                }
                break;
            }
        case ir_seq:            { assert(0); }
        case ir_push:           {

                if(ir->data.iconst==0)
                {
                    emitInstruction("sw $v0, ($sp)", "PUSH: store v0 to the top of the stack");
                    emitInstruction("addiu $sp, $sp, -4", "PUSH: point stack pointer  to the next free space");
                } else if (ir->data.iconst==1)
                {
                    emitInstruction("sw $v1, ($sp)", "PUSH: store v0 to the top of the stack");
                    emitInstruction("addiu $sp, $sp, -4", "PUSH: point stack pointer  to the next free space");
                } else
                {
                    assert(0);
                }
                break;



 }
        case ir_pop:
            {
                emitInstruction("addiu $sp, $sp, 4", "POP: Subtract sp ");
                if(ir->data.iconst == 1)
                {
                    emitInstruction("lw $v1, ($sp)", "POP: pop the content of the stack to v1");
                    emitInstruction("sw $zero, ($sp)", "Zero the stack");
                } else if(ir->data.iconst==0)
                {
                    emitInstruction("lw $v0, ($sp)", "POP: pop the content of the stack to v0 ");
                    emitInstruction("sw $zero, ($sp)", "POP: Zero the stack");
                } else
                {
                    assert(0);
                }
                break;


            }
        default: {
            assert(0); // Not supported
        }
    }

    mips_ir_translate(ir->next);
}

__attribute__ ((unused))
static void mips_ir_variables(ir_node * ir) {
    while (ir != NULL) {
        // TODO Generate MIPS code for variables
        switch (ir->kind)
        {
        case ir_nop:            { break; }
        case ir_iconst:         {
                                    break;
                                }
        case ir_sconst:
            {
                char * freshlabel = fresgStrLabel();
                emitInstruction("%s: .asciiz \"%s\"", "STRCONST:RESERVE a space for string constant", freshlabel, ir->data.sconst);
               // free(ir->data.sconst);
                ir->data.sconst=freshlabel;
                break;
            }
        case ir_add:
        case ir_sub:
        case ir_mul:
        case ir_div:
        case ir_mod:
        case ir_bor:
        case ir_band:
        case ir_xor:
        case ir_or:

        case ir_and:
        case ir_eq:
        case ir_lt:
        case ir_gt:
        case ir_not:            { break; }
        case ir_reserve:
            {
                if(ir->data.reserve.size==4 && ir->data.reserve.val==NULL)
                {
                    emitInstruction("%s: .word %d", "Initialize a int variable", ir->data.reserve.name, ir->data.reserve.val);
                } else
                {
                    //string

                    char * freshlabel = fresgStrLabel();
                    emitInstruction("%s: .asciiz \"%s\"", "RESERVE:RESERVE a space for string constant", freshlabel, ir->data.reserve.val);

                    emitInstruction("%s: .word %d", "RESERVE a space to store  the address of the string const", ir->data.reserve.name, 0);
                    ir->data.reserve.val=freshlabel;
                }

                 break;
            }
        case ir_read:
            {

                break;
            }
        case ir_write:
            {
                break;
            }
        case ir_arglocal_read:
            {


                break;
            }
        case ir_arglocal_write: {  break;}
        case ir_lbl:            { break;}
        case ir_jump:           { break;}
        case ir_branchzero:     { break;}
        case ir_call:           {  break; }
        case ir_function:       {  break; }
        case ir_ret:            {  break; }
        case ir_intrinsic:      {break;}
        case ir_seq:            { assert(0); }
        case ir_push:           {
                break;
 }
        case ir_pop:
            {
                break;
            }
        default: {
            break;
        }
        }

        ir = ir->next;
    }
}

// generate MIPS from IR
void mips_ir(ir_node * ir, const char * o) {
    out = fopen(o, "w");

    fprintf(out, ".data\n");
    fflush(out);
    mips_ir_variables(ir);

    fprintf(out, ".text\n");
    fprintf(out, ".globl main\n");
    fprintf(out, "main:\n");
    fflush(out);
    mips_ir_translate(ir);

    fclose(out);
}
