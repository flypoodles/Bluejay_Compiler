%{

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "ast.h"

extern char * result;

int yylex(void); /* function prototype */

void yyerror(const char *s) {
    fprintf(stderr,"parsing error %s\n", s); exit(73);
}


#define BUF_SIZE 100000

__attribute__((unused))
static char * buf_printf(const char * format, ...) {
   static char buf[BUF_SIZE];
   va_list argp;
   va_start(argp, format);
   vsnprintf(buf, BUF_SIZE, format, argp);
   va_end(argp);
   return strdup(buf);
}

extern program p;




%}


%union {
    stmt_node * statement;
    list  * stmt;
    exp_node * exp;
	int ival;
    char * type;
    char * name;



    char * str;
    vardec_node * var;
    list * vars;
    fundec_node * func;
    list * funcs;
    list * funcCallParam;
    list * args;
    list * param;
	}

%define parse.error verbose

 %token <name>NAME
 %token <ival> INT
 %token <str> STRING
 %token <type> TYPE
 %token <ternary> TERNARY
 %token COLON
 %nonassoc INBLOCK ENDBLOCK STARTBLOCK COMMA
 %nonassoc IF WHILE REPEAT
 %nonassoc ELSE OTHERWISE
 %nonassoc VAR IMPLICIT

 %nonassoc RETURN
 %nonassoc FUN
 %nonassoc OPEN_PARENT
 %nonassoc CLOSE_PARENT
 %nonassoc SEMICOLON

 %left ASSIGN
 %left OR
 %left AND
 %left BOR
 %left XOR
 %left BAND
 %left EQ NE
 %left LT LE GT GE
 %left PLUS MINUS
 %left MUL DIV REM
 %right NOT




%type <stmt>      stmt
%type <stmt>       block
%type <statement>    statement
%type <exp>       exp

%type <vars>  variables
%type <var> variable

%type <vars>   funcVars
%type <var>   funcVar
%type <funcs>    functions
%type <func>     function
%type <args>     args
%type <param>         param
%type <exp>    funcCall
%type <funcCallParam>     funcCallParam
%type <exp>  ternaryEXP
%start program

%%

program:  variables functions stmt  {p.variables = $1;  p.functions = $2; p.statements=$3;}





variables:  /*empty*/                    {$$ = NULL;}
            |  variables  variable      {$$ = ListAddLast($2, $1);}


variable:    VAR  NAME TYPE  ASSIGN exp SEMICOLON     {$$ =VarDecNode($2, TyNode($3),$5, 0);}
            | IMPLICIT NAME TYPE  ASSIGN exp SEMICOLON     {$$ =VarDecNode($2, TyNode($3),$5, 1);}

funcVars:
            funcVar funcVars         {$$ = ListAddFirst($1, $2);}
            | funcVar                    {$$ = ListAddFirst($1, NULL);}
funcVar:
        VAR NAME TYPE  ASSIGN exp SEMICOLON     {$$ =VarDecNode($2, TyNode($3),$5, 0); }




stmt:
    statement                    {$$ = ListAddFirst($1, NULL);}
    | statement stmt               {$$ = ListAddFirst($1, $2);}

block:
    STARTBLOCK stmt ENDBLOCK         {$$ = $2;}
    | statement                      {$$ = ListAddFirst($1,NULL);}
    | STARTBLOCK stmt YYEOF          {$$ = $2;}

functions: /**/                        {$$ = NULL;}
       |  functions function         {$$ =ListAddLast($2,$1); }

function:
     FUN NAME TYPE  args STARTBLOCK funcVars stmt  ENDBLOCK      {$$= FunDecNode($2,TyNode($3),$4,$6,$7 );}
     | FUN NAME TYPE  args STARTBLOCK stmt ENDBLOCK                 {$$= FunDecNode($2,TyNode($3),$4,NULL,$6 );}

args:
    OPEN_PARENT param CLOSE_PARENT                    {$$ = $2;}
    | OPEN_PARENT CLOSE_PARENT                    {$$ = NULL;}
param:
    NAME TYPE                                       {$$ = ListAddFirst(Param(TyNode($2),$1), NULL);}
    | NAME TYPE COMMA param                                    {$$ = ListAddFirst(Param(TyNode($2),$1), $4);}



statement:
             RETURN exp SEMICOLON                                                        { $$ = RetNode($2);}
     |      RETURN SEMICOLON                                                   {$$= RetNode(voidNode());}
    |  IF OPEN_PARENT exp CLOSE_PARENT block  ELSE block         %prec ELSE                         {  $$= IfNode($3,$5,$7);}
    |  IF OPEN_PARENT exp CLOSE_PARENT block               %prec IF                                 {  $$= IfNode($3,$5,NULL);}
    |  WHILE OPEN_PARENT exp CLOSE_PARENT block OTHERWISE block       %prec OTHERWISE               {  $$= whileNode($3,$5,$7);}
    |  WHILE OPEN_PARENT exp CLOSE_PARENT block           %prec IF                                  {  $$= whileNode($3,$5,NULL);}
    |  REPEAT OPEN_PARENT exp  CLOSE_PARENT  block   %prec IF                                       {  $$= repeatNode($3,$5);}
    |  NAME ASSIGN exp SEMICOLON                                                                    {  $$= assignNode($1,$3);}
    |  funcCall          SEMICOLON                                              { $$ = funcStatementNode($1);}





ternaryEXP:
        OPEN_PARENT exp CLOSE_PARENT  TERNARY exp COLON exp %prec ELSE {$$ =  ternaryNode( $2, $5, $7);}
funcCallParam:
    exp COMMA funcCallParam                                    {$$ = ListAddFirst($1, $3);}
    |exp                                                         {$$ = ListAddFirst($1, NULL);}
funcCall:
    NAME OPEN_PARENT CLOSE_PARENT                    {$$= funcExpNode($1, NULL); }
    | NAME OPEN_PARENT funcCallParam CLOSE_PARENT                    {$$= funcExpNode($1, $3); }

exp:
    INT                        { $$ = IntNode($1); }
    | STRING                    { $$  = strConstNode($1); }
    | OPEN_PARENT exp CLOSE_PARENT  { $$= $2;}
    | exp PLUS exp                {$$= BinOpNode(plus_op, $1, $3); }
    | exp MINUS exp                { $$= BinOpNode(minus_op, $1, $3); }
    | exp MUL exp                {  $$= BinOpNode(mul_op, $1, $3); }
    | exp DIV exp                {  $$= BinOpNode(div_op, $1, $3); }
    | exp REM exp                {  $$= BinOpNode(rem_op, $1, $3);}
    | exp BOR exp                {  $$= BinOpNode(bor_op, $1, $3); }
    | exp BAND exp                { $$= BinOpNode(band_op, $1, $3); }
    | exp XOR exp                {  $$= BinOpNode(xor_op, $1, $3); }
    | exp LT exp                {   $$= BinOpNode(lt_op, $1, $3); }
    | exp LE exp                {   $$= BinOpNode(le_op, $1, $3); }
    | exp GT exp                {   $$= BinOpNode(gt_op, $1, $3);}
    | exp GE exp                {   $$= BinOpNode(ge_op, $1, $3);}
    | exp OR exp                {   $$= BinOpNode(or_op, $1, $3);}
    | exp AND exp                {  $$= BinOpNode(and_op, $1, $3);}
    | NOT exp                    { $$ = UnOpNode(not_op, $2); }
    | exp EQ exp                  { $$= BinOpNode(eq_op, $1, $3); }
    | exp NE exp                  { $$= BinOpNode(ne_op, $1, $3); }
    | NAME                            { $$ = varNode($1); }
    | funcCall                        {$$ = $1;}
    | ternaryEXP              {$$ =   $1;}
    %%
