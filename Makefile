SANITIZER=-O0 -fsanitize=address
FLAGS=-Wall -ggdb -Wextra -Werror -std=c18 -Wpedantic -Wno-unused-parameter ${SANITIZER}

bluejaycc: main.o lex.yy.o y.tab.o ast.o semantic_analysis_symbols.o semantic_analysis_types.o frames.o transform.o ir.o ast_to_ir.o ir_tree_to_list.o ir_list_to_mips.o ir_pushpop.o optimize.o
	cc ${FLAGS} -g -o bluejaycc main.o lex.yy.o y.tab.o symbol.o table.o ast.o semantic_analysis_types.o semantic_analysis_symbols.o frames.o transform.o ir.o ast_to_ir.o ir_tree_to_list.o ir_list_to_mips.o ir_pushpop.o optimize.o

interp: main_interp.o interpret.o ir.o symbol.o table.o
	cc ${FLAGS} -g -o $@ $^

y.tab.o: y.tab.c y.tab.h
	cc ${FLAGS} -g -c y.tab.c

main.o: main.c y.tab.h semantic_analysis_symbols.o semantic_analysis_types.h
	cc ${FLAGS} -g -c main.c

main_interp.o: main_interp.c y.tab.h semantic_analysis_symbols.o semantic_analysis_types.h
	cc ${FLAGS} -g -c main_interp.c

interpret.o: interpret.c interpret.h
	cc ${FLAGS} -g -c interpret.c

ast.o: ast.c symbol.o
	cc ${FLAGS} -g -c ast.c

symbol.o: symbol.c table.o
	cc ${FLAGS} -g -c symbol.c

table.o: table.c
	cc ${FLAGS} -g -c table.c

semantic_analysis_types.o: semantic_analysis_types.c ast.o
	cc ${FLAGS} -g -c semantic_analysis_types.c

semantic_analysis_symbols.o: semantic_analysis_symbols.c ast.o
	cc ${FLAGS} -g -c semantic_analysis_symbols.c

frames.o: frames.c ast.o
	cc ${FLAGS} -g -c frames.c

transform.o: transform.c ast.o
	cc ${FLAGS} -g -c transform.c

ir_pushpop.o: ir_pushpop.c ir.o
	cc ${FLAGS} -g -c ir_pushpop.c

optimize.o: optimize.c ir.o ast.o
	cc ${FLAGS} -g -c optimize.c

ir_list_to_mips.o: ir_list_to_mips.c ir.o
	cc ${FLAGS} -g -c ir_list_to_mips.c

ir.o: ir.c ast.o
	cc ${FLAGS} -g -c ir.c

ast_to_ir.o: ast_to_ir.c ir.o
	cc ${FLAGS} -g -c ast_to_ir.c

ir_tree_to_list.o: ir_tree_to_list.c ir.o
	cc ${FLAGS} -g -c ir_tree_to_list.c

lex.yy.o: lex.yy.c
	cc ${SANITIZER} -g -c lex.yy.c

lex.yy.c: lexer.lex y.tab.h
	lex lexer.lex

y.tab.c: parser.y ast.h ast.o
	 yacc -o y.tab.c -dv parser.y

y.tab.h: y.tab.c
	echo "y.tab.h was created at the same time as y.tab.c"

all: bluejaycc interp

clean: 
	rm -f bluejaycc interp lex.yy.c y.tab.c y.tab.h y.output *.o

.DEFAULT_GOAL = all
