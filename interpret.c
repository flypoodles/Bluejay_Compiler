#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>

#include "interpret.h"
#include "ir.h"
#include "symbol.h"


// exits with the given code and message,
// which can take variadic arguments to print
//
// The first variadic argument is the header.  
// I would use ## to make the variadic arguments optional
// and the header an explicit parameter but I don't think that's
// compiler agnostic.  
//
// I decided to keep this macro so that any errors will have the
// right line number.  I could unfold it at every call site as well,
// but that would add a lot of clutter.  
#define ERR_EXIT(code, msg, ...)                                          \
	do {                                                                  \
		fprintf(stderr, "ERROR %d-%s: " msg "\n", __LINE__, __VA_ARGS__); \
		assert(0);                                                        \
	} while(0)


// All possible types of stack data;
// currently just Ints and Strings
enum stack_data {
	stack_int,
	stack_str,
};

/*
 * A single piece of data stored on the stack.
 * This is intended to be used as a linked list stack.
 *
 * enum stack_data kind: What kind of data is in this node
 * union data: The associated data for computation
 *
 * stack_node *next: the next element in the stack or NULL
 */
typedef struct stack_node {
	enum stack_data kind;
	// trying out duplicate data for macros
	union {
		int stack_int;
		char *stack_str;
	} data;

	struct stack_node *next;	
} stack_node;


// constructs a node on the stack containing an Int
stack_node * StackInt(int n) {
	stack_node *node = malloc(sizeof(stack_node));
	node->kind = stack_int;
	node->data.stack_int = n;
	node->next = NULL;

	return node;
}

// constucts a node on the stack containing a string
stack_node * StackString(char *s) {
	stack_node *node = malloc(sizeof(stack_node));
	node->kind = stack_str;
	node->data.stack_str = s;
	node->next = NULL;

	return node;
}

// duplicates the top node of any stack (or singleton node)
stack_node * dup_head_node(stack_node *n) {
	stack_node *new = malloc(sizeof(stack_node));

	// copy the data over
	// (we don't want to add var to the stack)
	new->kind = n->kind;
	// if we have strings here, we have to copy it over 
	// so we switch on cases
	switch (n->kind) {
		case stack_int:
			new->data.stack_int = n->data.stack_int;
			break;
		case stack_str:
			new->data.stack_str = n->data.stack_str;
			break;
		default:
			ERR_EXIT(-1, "Invalid kind when trying to copy a node: %d", 
					"INTERP_LIST_IR_READ", n->kind);
			break;
	}
	new->next = NULL;

	return new;
}

/*
 * free_stack
 *
 * Free's the provided stack, ensuring to free
 * any data stored within it. 
 *
 * 
 * stack_node *h: the head of the stack
 *
 */
void free_stack(stack_node *h) {
	if (h != NULL) {
		switch (h->kind) {
			case stack_int:
				break;
			case stack_str:
				// don't free the strings; they're shared
				break;
			default:
				ERR_EXIT(-1, "Invalid stack_node kind, number %d", "FREE_STACK", h->kind);
				break;
		}

		// free any children
		free_stack(h->next);
		free(h);
	}
}

/*
 * pop_stack 
 *
 * returns the old head of the stack (sliced off properly)
 * and mutates the rest of the stack into place
 */
stack_node * pop_stack(stack_node **stack) {
	// if we were passed a null pointer rather than the pointer to the stack
	if (stack == NULL) {
		ERR_EXIT(-1, "NULL passed to pop_stack", "POP_STACK");
	}

	// if the stack is empty, then we can't pop
	if (*stack == NULL) {
		ERR_EXIT(-1, "Tried to pop empty stack", "POP_STACK"); 
	}

	// otherwise, we grab the head
	stack_node *head = *stack;

	// and make the stack point to the next thing in itself
	*stack = head->next;

	// then tidy and return the head
	head->next = NULL;
	return head;
}

/*
 * pop_stack_int
 *
 * pops the passed stack, enforcing that the
 * top of the stack must be an int, which it returns
 *
 */
int pop_stack_int(stack_node **stack) {
	stack_node *popped = pop_stack(stack);

	// either we got and int to return or we fail
	int ret;
	switch (popped->kind) {
		case stack_int:
			ret = popped->data.stack_int;
			break;
		case stack_str:
			ERR_EXIT(-1, "Popped stack expecting an int but got string", "POP_STACK_INT");
			break;
		default:
			ERR_EXIT(-1, "Popped stack expecting an int but got unknown enum %d", "POP_STACK_INT", popped->kind);
			break;
	}

	// then free the node and return
	free_stack(popped);
	return ret;	
}

/*
 * pop_stack_str
 *
 * pops the top of the given stack, enforcing that
 * the value must be a string, which it returns
 *
 */
char * pop_stack_str(stack_node **stack) {
	stack_node *popped = pop_stack(stack);

	char *ret = NULL;
	switch (popped->kind) {
		case stack_str:
			ret = popped->data.stack_str;
			break;
		case stack_int:
			ERR_EXIT(-1, "Popped stack expecting a string but got int", "POP_STACK_STRING"); 
			break;
		default:
			ERR_EXIT(-1, "Popped stack expecting a string but got unknown enum %d", "POP_STACK_STRING", popped->kind);
			break;
	}

	free_stack(popped);
	return ret;
}

/*
 * push_node
 *
 * pushes node "n" to the top of the stack "stack"
 *
 * for consistency with popping, this directly changes the double pointer
 * rather than returning a new stack
 *
 */
void push_node(stack_node**stack, stack_node *n) {
	n->next = *stack;
	*stack = n;
}

/*
 * push_to_reg
 *
 * pushes the node in "reg" to the top of the stack.
 *
 * This also invalidates "reg," setting it to NULL
 *
 */
void push_to_reg(stack_node **stack, stack_node **reg) {
	(*reg)->next = *stack;
	*stack = *reg;
	*reg = NULL;
}

// Pops a value off the top of the stack to a register
// optionally freeing the old value stored in the register
void pop_to(stack_node **stack, stack_node **reg) {
	// error if null is passed
	if (stack == NULL) {
		ERR_EXIT(-1, "Passed in NULL rather than a stack", "POP_TO"); 
	}
	// an error if the stack is not long enough
	if (*stack == NULL) {
		ERR_EXIT(-1, "Attempted to pop empty stack", "POP_TO"); 
	}

	// copies top of stack over and returns new stack
	*reg = *stack;
	*stack = (*stack)->next;
	(*reg)->next = NULL;
}

/*
 * find_labels
 *
 * finds all labels present in the program and stores their nodes
 * as maps in a table to allow for jumps
 *
 */
S_table find_labels(ir_node *n) {
	S_table labels = S_empty();

	while (n) {
		switch(n->kind) {
			// the label is a no-op but jumping directly to it
			// allows us to always step forwards in the interpreter,
			// so we store the address of n itself
			//
			// Also, this ensures NULL = label not found
			case ir_lbl:
				S_enter(labels, S_Symbol(n->data.lbl->name), (void *) n);
				break;
			// for functions, we also want them stored here
			case ir_function:
				S_enter(labels, S_Symbol(n->data.lbl->name), (void *) n);
				break;

			// do nothing for anything else
			default:
				break;
		}

		n = n->next;
	}

	return labels;
}

/*
 * lookup_label
 *
 * returns the ir_node of a given label,
 * exiting if not present in the program
 *
 * problem is used in error messages to give a better idea of
 * why the label not being present is a problem (can't jump to it, etc)
 *
 */
ir_node * lookup_label(S_table labels, char *name, char *problem) {
	ir_node *lbl_node = S_look(labels, S_Symbol(name));
	// if this is NULL, the label doesn't exist in the program
	// since we always return
	if (!lbl_node) {
		ERR_EXIT(-1, 
				"label %s is not defined in the program: %s", 
				"INTERP_LIST_JUMP", name, problem);
	}
	// otherwise, we return the node found
	return lbl_node;	
}


// We need global "registers" to store what we're operating on
// We'll keep them as stack nodes to keep the types
//
// NOTE: by design, a register only ever holds a value if
//       something has been pushed to it.  This diverges from
//       hardware (the register always has something in it) and
//       requires that all push/pops are matched.
//
//       This may mean some programs would compile and potentially
//       work properly but fail to run in this interpreter due to
//       pushing or popping spurious values.
static stack_node *r0 = NULL;
static stack_node *r1 = NULL;


/*
 * interp_list
 *
 * Walks the list IR (using the next pointer)
 * and simulates execution on a stack.
 *
 * ir_node *head: the starting point to walk the list from
 * stack_node **frame: the arguments and locals within the function as a stack_node array
 * S_table labels: a map of all labels in the code
 * S_table globals: NULL when called at global scope, the global variables otherwise
 *
 * returns the int the program returns
 *
 */
stack_node * interp_list(ir_node *cur, stack_node **frame, S_table labels, S_table globals) {

	stack_node *stack = NULL;
	int is_global_scope;

	// if we haven't made a global table yet (we are in top level) make it.
	// unfortunately all the memory here is leaked since we can't delete an S_table
	// or even iterate over it!
    assert(globals != NULL);
	// we're in a local function
	if (frame != NULL) {
		is_global_scope = 0;

		// frame should not be NULL since we're in a function call
	}
	
	// while we have a non-null node
	while (cur) {
		int goto_next_intr = 1; // should we go to the next instruction?

		// run the action of the node
		switch (cur->kind) {
			case ir_nop: 
				// do nothing
				break;

				// Push constants
				// NOTE: these go into r0, not the stack
			case ir_iconst:
				free_stack(r0);
				r0 = StackInt(cur->data.iconst);
                push_to_reg(&stack, &r0);
				break;
			case ir_sconst: 
				free_stack(r0);
				// only a pointer to the existing string in memory
				r0 = StackString(cur->data.sconst);
                push_to_reg(&stack, &r0);
				break;

// defined because these are all identical and I can't
// use a function to simplify them easily
#define INT_BINOP(OP)                                     \
				do {                                      \
					int l, r;                             \
					r = pop_stack_int(&stack);            \
                    l = pop_stack_int(&stack);            \
                    push_node(&stack, StackInt(l OP r));  \
				} while(0)
//#undef INT_BINOP

			// Arithmetic irerations
			case ir_add:  INT_BINOP(+); break;
			case ir_sub:  INT_BINOP(-); break;
			case ir_mul:  INT_BINOP(*); break;
			case ir_div:  INT_BINOP(/); break;
			case ir_mod:  INT_BINOP(%); break;
			case ir_bor:  INT_BINOP(|); break;
			case ir_band: INT_BINOP(&); break;
			case ir_xor:  INT_BINOP(^); break;

			// logical
			// NOTE: this doesn't implement short circuiting
			case ir_or:  INT_BINOP(||); break;
			case ir_and: INT_BINOP(&&); break;
			case ir_eq:  INT_BINOP(==); break;
			case ir_lt:  INT_BINOP(<); break;
			case ir_gt:  INT_BINOP(>); break;

			case ir_not:
				{
					int top = pop_stack_int(&stack);
                    push_node(&stack, StackInt((top == 0) ? 1 : 0));
				}
				break;


				// Variables
			case ir_reserve: 
				// add var to the S_table, initialized to whatever value is given
				{
					stack_node *var = malloc(sizeof(stack_node));
					// reserving an integer requires size of 4 and val = NULL
					if (cur->data.reserve.size == 4 && cur->data.reserve.val == NULL) {
						var->kind           = stack_int;
						var->data.stack_int = 0;
						var->next           = NULL;
					}
					// If we have other length, it is a string
					// This is not specified as a valid reserve mode, but it is done 
					// on 276: ast_to_ir.c
					// so we support it here
					else {
						// ensure the length of the data and given length are the same
						int given_l = cur->data.reserve.size;
						int actual_l = strlen(cur->data.reserve.val);

						if (given_l != actual_l + 1) {
							ERR_EXIT(-1, 
									"Requested %d bytes reserved for %d byte long string (with null)", 
									"INTERP_LIST_RESERVE",
									given_l, actual_l);
						}

						// now copy data over
						var->kind = stack_str;
						var->data.stack_str = cur->data.reserve.val; 
						var->next = NULL;
					}

					S_enter(globals, S_Symbol(cur->data.reserve.name), var);
				}
				break;
			case ir_read: 
				// lookup var in global scope
				{
					stack_node *var = S_look(globals, S_Symbol(cur->data.read_write.name));
					if (!var) {
						ERR_EXIT(-1, 
								"Attempt to read to global variable %s failed; var not found in globals",
								"INTERP_LIST_IR_READ", cur->data.read_write.name);
					}
					assert(!var->next);

					// store result in r0
					free_stack(r0);
					r0 = dup_head_node(var);
                    push_node(&stack, r0);
                    r0 = NULL;
				}
				break;


			case ir_write: 
				// write to the global var
				{
					stack_node *var = S_look(globals, S_Symbol(cur->data.read_write.name));
					if (!var) {
						ERR_EXIT(-1, 
								"Attempt to write to global variable %s failed; var not found in globals",
								"INTERP_LIST_IR_WRITE", cur->data.read_write.name);
					}

					// we need the value in r0 to write it to tLhe stack
					stack_node *new_val = pop_stack(&stack);

					// also both of these should have NULL nexts
					assert(!var->next && !new_val->next);

					// if we have the variable and new value, point to the new stack data
					// we copy data over since S_table's only shadow and I don't want to keep all old ones there

					// could copy the union but this allows for error catching
					switch (new_val->kind) {
						case stack_int:
							var->data.stack_int = new_val->data.stack_int;
							break;
						case stack_str:
							var->data.stack_str = new_val->data.stack_str;
							break;
						default:
							ERR_EXIT(-1, "Invalid kind written in global variable %s, #%d", 
									"INTERP_LIST_IR_WRITE", cur->data.read_write.name, var->kind);
							break;
					}

					// if the type changes, we store that here.
					var->kind = new_val->kind;


					// we don't need the new value anymore
					// since we copied it over
					free_stack(new_val);
				}
				break;
			case ir_arglocal_read: 
				{
					if (is_global_scope) {
						ERR_EXIT(-1, "Tried to read a local variable in global scope", 
								"INTERP_LIST_IR_ARGLOCAL_READ");
					}

					// look at var in index, copy it to the stack
					int index = cur->data.iconst;
					stack_node *n = frame[index];
					if (!n) {
						ERR_EXIT(-1, "Tried to access an uninitialized stack variable at index %d", 
								"INTERP_LIST_IR_ARGLOCAL_READ", index);
					}

					// we don't push the original since this is independent
					stack_node *dup = dup_head_node(n);
					push_node(&stack, dup);
				}
				break;
			case ir_arglocal_write: 
				{
					if (is_global_scope) {
						ERR_EXIT(-1, "Tried to write a local variable in global scope", 
								"INTERP_LIST_IR_ARGLOCAL_WRITE");
					}

					// replace the old val with a new one
					// new one is in register 0 according to ast_to_ir.c
					stack_node *val = pop_stack(&stack);

					int index = cur->data.iconst;
					free_stack(frame[index]);
					frame[index] = val;
				}
				break;

				// Control-flow
			case ir_lbl: 
				// labels do nothing, we just skip over them
				break;

			case ir_jump: 
				// to jump, we look up the label in the table and go to that node
				{
					ir_node *new_cur = lookup_label(labels, cur->data.lbl->name, "cannot jump");

					// this should be a label, not a function
					if (new_cur->kind == ir_lbl) {
						// we won't run the label since after this switch we go to the next node
						// (we don't set goto_next_intr = 0)
						cur = new_cur;
					}
					else if (new_cur->kind == ir_function) {
						ERR_EXIT(-1, 
								"Tried to jump directly to a function instead of calling it", 
								"INTERP_LIST_IR_JUMP");
					}
					else {
						ERR_EXIT(-1,
								"Tried to jump to node of unknown kind %d",
								"INTERP_LIST_IR_JUMP", new_cur->kind);
					}
				}
				break;
			case ir_branchzero: 
				{
					// no matter what, the label better exist
					// checking here ensures we error if it doesn't
					ir_node *jump_to = lookup_label(labels, cur->data.lbl->name, "cannot branch if 0");

					// if the top of the stack is 0, we jump
					if (pop_stack_int(&stack) == 0) {
						cur = jump_to;
					}

					// if not, no-op, so do nothing
				}
				break;

				// Functions
			case ir_function: 
				// TODO: any function setup needed here
				// (verify number of arguments on stack?)
				// probably best left as a no-op and have everything in call
				break;
			case ir_call: 
				{
					ir_node *func_start = lookup_label(
							labels, 
							cur->data.call_function.lbl->name, 
							"cannot call function");

					// this should be a function, not a label or anything else
					if (func_start->kind == ir_function) {
						// we have some number of arguments to the function
						int num_args = cur->data.call_function.vars;
						// and a number of local variables
						int num_locs = func_start->data.call_function.vars;
						int tot_vars = num_args + num_locs;
						
						// we need an array capable of holding all of those
						// TODO: should I add a wrapper struct to ensure length is ok?
						stack_node **new_frame = malloc(sizeof(stack_node *) * tot_vars);

						// pop_args as specified by the arg_count and push them into the frame
						// Args go first, locals are null since they're established inside the program
						for (int i = 0; i < num_args; ++i) {
							stack_node *cur = pop_stack(&stack);
							// args are in reverse order on the stack
							new_frame[num_args - i - 1] = cur;
						}
						// all locals are initialized to NULL
						for (int i = num_args; i < tot_vars; ++i) {
							new_frame[i] = NULL;
						}

						// we always have an int on the stack which we need to pop off and ignore
						pop_stack_int(&stack);

						// recursively call to handle the function
						// passing in the new local frame
						stack_node *ret_val = interp_list(func_start, new_frame, labels, globals);

						// free our frame afterwards
						for (int i = 0; i < tot_vars; ++i) {
							free_stack(new_frame[i]);
						}
						free(new_frame);

						// and finally return the value!
						push_node(&stack, ret_val);
					}
					// error cases
					else if (func_start->kind == ir_lbl) {
						ERR_EXIT(-1, "Tried to call a label that isn't a function", "INTERP_LIST_IR_CALL"); 
					}
					else {
						ERR_EXIT(-1, "Tried to call a node of unknown kind %d", "INTERP_LIST_IR_CALL", func_start->kind);
					}
				}
				break;
			case ir_ret: 
				// exit from a function
				{
					if (is_global_scope) {
						ERR_EXIT(-1, "Cannot return from global scope; did you mean to exit?", "INTERP_LIST_IR_RET");
					}

					// just return the value and cleanup!
					stack_node *ret = pop_stack(&stack);
					free_stack(stack);
					return ret;
				}
				break;
			case ir_intrinsic: 
				// compiler/interpreter defined functions
				{
					switch (cur->data.intrinsic) {
						case intrinsic_exit:
							{
								// if (!r0) {
								// 	ERR_EXIT(-1, "Exited with no value in r0", "INTERP_LIST_IR_INTRINSIC");
								// }
								// stack_node *ret = r0;
                                stack_node *ret = pop_stack(&stack);
								free_stack(stack);
								return ret;
							}
							break;
						case intrinsic_print_int:
							printf("%d", pop_stack_int(&stack));
							break;
						case intrinsic_print_string:
							{
								// no need to free the string as it's from the ir
								printf("%s", pop_stack_str(&stack));
								break;
							}
						default:
							ERR_EXIT(-1, "Invalid intrinsic passed: %d", "INTERP_LIST_INTRINSIC", cur->data.intrinsic);
							break;
					}
				}
				break;

				// should not exist in list_ir
			case ir_seq: 
				assert(0);
				break;

				// register is specified in ival
			case ir_push: 
				switch (cur->data.iconst) {
					case 0:
						push_to_reg(&stack, &r0);
						break;
					case 1:
						push_to_reg(&stack, &r1);
						break;
					default:
						ERR_EXIT(-1, "Cannot push to register #%d", "INTERP_LIST_IR_PUSH", cur->data.iconst);
						break;
				}
				break;
			case ir_pop: 
				switch (cur->data.iconst) {
					case 0:
						pop_to(&stack, &r0);
						break;
					case 1:
						pop_to(&stack, &r1);
						break;
					// pop & discard
					case -1:
						{
							stack_node *ignored = pop_stack(&stack);
							free_stack(ignored);
						}
						break;
					default:
						ERR_EXIT(-1, "Cannot pop to register #%d", "INTERP_LIST_IR_POP", cur->data.iconst);
						break;
				}
				break;
			default:
				ERR_EXIT(-1, "Invalid IR kind %d", "INTERP_LIST", cur->kind);
				break;
		}

		// move on to the next node unless otherwise requested
		if (goto_next_intr) {
			cur = cur->next;
		}
	}

	free_stack(stack);
	return NULL;
}

static void destroy_globals(S_symbol sym, void * glob) {
    stack_node * s = glob;
    free(s);
}


int interpret(ir_node *head) {
	S_table labels = find_labels(head);

    S_table globals = S_empty();
 	stack_node *ret = interp_list(head, NULL, labels, globals);
 	int ret_int = 0;
 
	if (ret) {
		// a program should exit with an integer
		switch (ret->kind) {
			case stack_int:
				ret_int = ret->data.stack_int;
				break;
			case stack_str:
				ERR_EXIT(-1, "Program exited with a string value", "INTERPRET");
				break;
			default:
				ERR_EXIT(-1, "Program exited with invalid stack kind: %d", "INTERPRET", ret->kind);
				break;
		}
	 
		free_stack(ret);
	}

    S_destroy(labels, NULL);
    S_destroy(globals, destroy_globals);

    free(labels);
    free(globals);
 
	printf("\n"); //NOTE: added to mirror mars which always prints a newline at the end of file
	return ret_int;
}
