#define _POSIX_C_SOURCE 200809L // enable strdup

#include "ir.h"

#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

ir_label * freshLabel() {
    static char buffer[1024];
    static int lastLabel = 0;

    snprintf(buffer, sizeof(buffer), "L%d", lastLabel);
    ir_label * ret = calloc(1, sizeof(ir_label));
    lastLabel += 1;

    ret->name = strdup(buffer);

    return ret;
}

ir_label * namedLabel(char * name) {
    ir_label * ret = calloc(1, sizeof(ir_label));
    ret->name = name;

    return ret;
}


ir_node * Iconst(int i) {
    ir_node * ret = calloc(1, sizeof(ir_node));
    ret->kind = ir_iconst;
    ret->data.iconst = i;
    return ret;
}

ir_node * Sconst(char * s) {
    ir_node * ret = calloc(1, sizeof(ir_node));
    ret->kind = ir_sconst;
    ret->data.sconst = s;
    return ret;
}

ir_node * Ops(enum ir_op ir) {
    ir_node * ret = calloc(1, sizeof(ir_node));
    ret->kind = ir;
    return ret;
}

ir_node * Intrinsic(enum intrinsic ir) {
    ir_node * ret = calloc(1, sizeof(ir_node));
    ret->kind = ir_intrinsic;
    ret->data.intrinsic = ir;
    return ret;
}

ir_node * Reserve(int size, char * name, char * val) {
    ir_node * ret = calloc(1, sizeof(ir_node));
    ret->kind = ir_reserve;
    ret->data.reserve.name = name;
    ret->data.reserve.size = size;
    ret->data.reserve.val = val;
    return ret;
}

ir_node * Read(char * name) {
    ir_node * ret = calloc(1, sizeof(ir_node));
    ret->kind = ir_read;
    ret->data.sconst = name;
    return ret;
}

ir_node * Write(char * name) {
    ir_node * ret = calloc(1, sizeof(ir_node));
    ret->kind = ir_write;
    ret->data.sconst = name;
    return ret;
}

ir_node * ArgLocalRead(int i) {
    ir_node * ret = calloc(1, sizeof(ir_node));
    ret->kind = ir_arglocal_read;
    ret->data.iconst = i;
    return ret;
}

ir_node * ArgLocalWrite(int i) {
    ir_node * ret = calloc(1, sizeof(ir_node));
    ret->kind = ir_arglocal_write;
    ret->data.iconst = i;
    return ret;
}

ir_node * Label(ir_label * lbl) {
    ir_node * ret = calloc(1, sizeof(ir_node));
    ret->kind = ir_lbl;
    ret->data.lbl = lbl;
    return ret;
}

ir_node * Function(ir_label * lbl, int locs) {
    ir_node * ret = calloc(1, sizeof(ir_node));
    ret->kind = ir_function;
    ret->data.call_function.lbl = lbl;
    ret->data.call_function.vars = locs;
    return ret;
}

ir_node * Call(ir_label * lbl, int vars) {
    ir_node * ret = calloc(1, sizeof(ir_node));
    ret->kind = ir_call;
    ret->data.call_function.lbl = lbl;
    ret->data.call_function.vars = vars;
    return ret;
}

ir_node * Return() {
    ir_node * ret = calloc(1, sizeof(ir_node));
    ret->kind = ir_ret;
    return ret;
}

ir_node * Jump(ir_label * lbl) {
    ir_node * ret = calloc(1, sizeof(ir_node));
    ret->kind = ir_jump;
    ret->data.lbl = lbl;
    return ret;
}

ir_node * Branch(ir_label * lbl) {
    ir_node * ret = calloc(1, sizeof(ir_node));
    ret->kind = ir_branchzero;
    ret->data.lbl = lbl;
    return ret;
}

ir_node * Seq(ir_node * i1, ir_node * i2) {
    ir_node * ret = calloc(1, sizeof(ir_node));
    ret->kind = ir_seq;
    ret->data.seq.o1 = i1;
    ret->data.seq.o2 = i2;
    return ret;
}

ir_node * Nop() {
    ir_node * ret = calloc(1, sizeof(ir_node));
    ret->kind = ir_nop;
    return ret;
}

ir_node * Pop() {
    ir_node * ret = calloc(1, sizeof(ir_node));
    ret->kind = ir_pop;
    ret->data.iconst = -1;
    return ret;
}

static void write_everything(int fd, const char * buf, size_t len) {
    size_t written_total = 0;

    while (written_total != len) {
        int written = write(fd, buf+written_total, len-written_total);
        assert(written > 0);
        written_total += written;
    }
}

static void write_int(int fd, int i) {
    write_everything(fd, (char*) &i, sizeof(int));
}

static void write_null_terminated_string(int fd, const char * string) {
    int len = (string == NULL) ? 0 : strlen(string)+1;
    write_int(fd, len);
    write_everything(fd, string, len*sizeof(char));
}

void ir_list_write(ir_node * head, const char * file) {
    int fd = open(file, O_WRONLY | O_TRUNC | O_CREAT, S_IRGRP | S_IROTH | S_IRUSR | S_IWUSR);


    while (head != NULL) {

        write_int(fd, head->kind);

        switch (head->kind) {
            case ir_nop:
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
            case ir_not:
            case ir_ret:
                // No extra info needed
                break;

            case ir_iconst:
            case ir_arglocal_read:
            case ir_arglocal_write:
            case ir_push:
            case ir_pop:
                write_int(fd, head->data.iconst);
                break;

            case ir_sconst:
                write_null_terminated_string(fd, head->data.sconst);
                break;

            case ir_reserve:
                write_null_terminated_string(fd, head->data.reserve.name);
                write_null_terminated_string(fd, head->data.reserve.val);
                write_int(fd, head->data.reserve.size);
                break;

            case ir_read:
            case ir_write:
                write_null_terminated_string(fd, head->data.read_write.name);
                break;

            case ir_lbl:
            case ir_jump:
            case ir_branchzero:
                write_null_terminated_string(fd, head->data.lbl->name);
                break;

            case ir_function:
            case ir_call:
                write_null_terminated_string(fd, head->data.call_function.lbl->name);
                write_int(fd, head->data.call_function.vars);
                break;

            case ir_intrinsic:
                write_int(fd, head->data.intrinsic);
                break;

            case ir_seq:
                assert(0);
                break;

            default:
                assert(0);
                break;
        }

        head = head->next;
    }

    close(fd);
}

static void read_everything(int fd, char * buf, int size) {
    int read_total = 0;

    while (read_total != size) {
        int r = read(fd, buf + read_total, size - read_total);
        assert(r > 0);
        read_total += r;
    }

}

static int read_int(int fd) {
    int ret = 0;
    read_everything(fd, (char*) &ret, sizeof(int));
    return ret;
}

static char * read_null_terminated_string(int fd) {
    int len = read_int(fd);

    if (len == 0)
        return NULL;

    char * ret = malloc(len * sizeof(char));

    read_everything(fd, ret, len);

    return ret;
}

ir_node * ir_list_read(const char * file) {
    int fd = open(file, O_RDONLY);
    assert(fd > 0);

    ir_node * head = NULL;
    ir_node * tail = NULL;


    while (1) {
        int op;
        ssize_t r = read(fd, &op, sizeof(op));
        if (r == 0)
            break;

        ir_node * node = malloc(sizeof(*head));
        node->kind = op;
        node->next = NULL;

        if (head == NULL)
            head = node;
        else
            tail->next = node;

        tail = node;

        switch (node->kind) {
            case ir_nop:
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
            case ir_not:
            case ir_ret:
                // No extra info needed
                break;

            case ir_iconst:
            case ir_arglocal_read:
            case ir_arglocal_write:
            case ir_push:
            case ir_pop:
                node->data.iconst = read_int(fd);
                break;

            case ir_sconst:
                node->data.sconst = read_null_terminated_string(fd);
                break;

            case ir_reserve:
                node->data.reserve.name = read_null_terminated_string(fd);
                node->data.reserve.val  = read_null_terminated_string(fd);
                node->data.reserve.size = read_int(fd);
                break;

            case ir_read:
            case ir_write:
                node->data.read_write.name = read_null_terminated_string(fd);
                break;

            case ir_lbl:
            case ir_jump:
            case ir_branchzero:
                node->data.lbl = malloc(sizeof(node->data.lbl));
                node->data.lbl->name = read_null_terminated_string(fd);
                break;

            case ir_function:
            case ir_call:
                node->data.call_function.lbl = malloc(sizeof(node->data.lbl));
                node->data.call_function.lbl->name = read_null_terminated_string(fd);
                node->data.call_function.vars = read_int(fd);
                break;

            case ir_intrinsic:
                node->data.intrinsic = read_int(fd);
                break;

            case ir_seq:
                assert(0);
                break;

            default:
                assert(0);
                break;
        }
    }

    close(fd);
    return head;
}

void ir_list_print(ir_node * head) {
    const char * names[] = {
            "NOP",
            "ICONST",
            "SCONST",
            "ADD",
            "SUB",
            "MUL",
            "DIV",
            "MOD",
            "BOR",
            "BAND",
            "XOR",
            "OR",
            "AND",
            "EQ",
            "LT",
            "GT",
            "NOT",
            "RESERVE",
            "READ",
            "WRITE",
            "ARGLOCAL_READ",
            "ARGLOCAL_WRITE",
            "LBL",
            "JUMP",
            "BRANCHZERO",
            "FUNCTION",
            "CALL",
            "RET",
            "INTRINSIC",
            "SEQ",
            "PUSH",
            "POP",
    };

    const char * instrinsics[] = {
            "PRINT_INT",
            "PRINT_STRING",
            "EXIT",
    };

    while (head != NULL) {
        const char * name = names[head->kind];

        switch (head->kind) {
            case ir_nop:
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
            case ir_not:
            case ir_ret:
                printf("\t%s\n", name);
                break;

            case ir_iconst:
            case ir_arglocal_read:
            case ir_arglocal_write:
            case ir_push:
            case ir_pop:
                printf("\t%s %d\n", name, head->data.iconst);
                break;

            case ir_sconst:
                printf("\t%s %s\n", name, head->data.sconst);
                break;

            case ir_reserve:
                printf("\t%s %s %d %s\n", name, head->data.reserve.name, head->data.reserve.size, head->data.reserve.val);
                break;

            case ir_read:
            case ir_write:
                printf("\t%s %s\n", name, head->data.sconst);
                break;

            case ir_lbl:
                printf("%s:\n", head->data.lbl->name);
                break;
            case ir_jump:
            case ir_branchzero:
                printf("\t%s %s\n", name, head->data.lbl->name);
                break;

            case ir_call:
                printf("\t");
                // fall through
            case ir_function:
                printf("%s %s %d\n", name, head->data.call_function.lbl->name, head->data.call_function.vars);
                break;

            case ir_intrinsic:
                printf("\t%s %s\n", name, instrinsics[head->data.intrinsic]);
                break;

            case ir_seq:
                assert(0);
                break;

            default:
                assert(0);
                break;
        }

        head = head->next;
    }
}
