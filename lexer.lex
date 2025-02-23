%{

#include <assert.h>
#include "ast.h"
#include <string.h>
#include <unistd.h>
#include "y.tab.h"


/* keep track of current character number */
static int col = 1;
/* keep track of current line number */
static int line = 1;

static int space = 0;

static int blockStack[20] = {0};
static int currentBlock = 0;
static int blockDepth = 0;
static int curIndent = 0;
static int blockend = 0;
static void token() {
//    yylval.col = col;
//    yylval.line = line;
//    yylval.block = blockDepth;

    col+=yyleng;
  }



static int block(){
  int status = INBLOCK;
    if (space > blockStack[currentBlock]){
      blockStack[++currentBlock]=space;
      status = STARTBLOCK;

  } else if (space < blockStack[currentBlock]) {

       status = ENDBLOCK;
         currentBlock--;


    }
    curIndent = space;
    blockDepth = currentBlock;
    return status;
}
static char Buffer[1000] = {0};
static int bufferSize = 0;
static void addString(char c){
       Buffer[bufferSize++] = c;

}

static void startString(){
//    yylval.col = col;
//    yylval.line = line;
//
//    yylval.block = blockDepth;
//    yylval.indent = curIndent;
    bufferSize = 0;
    Buffer[bufferSize] = '\0';

}

static char * endString() {
    Buffer[bufferSize] = '\0';

    return Buffer;
  }

%}

%option noyywrap

%x BLOCK
%x String
%x eof
%%

^.*       {BEGIN(BLOCK); yyless(0); space=0;}
<BLOCK>{
  " "        {space+=1;col+=yyleng;}
  "\t"         {space +=4;col+=yyleng;}
  "\n"            {yyless(0);BEGIN(INITIAL);}
  .           {yyless(0); int myBlock = block(); /*printf("current block: %d\n", myBlock);*/
                if (myBlock == STARTBLOCK){
                      BEGIN(INITIAL);
                      return myBlock;
                } else if (myBlock== INBLOCK) {
                      BEGIN(INITIAL);
                } else if (myBlock== ENDBLOCK) {
                      return myBlock;
                }
              }
}





"return"                {token();return RETURN;}
";"                       {token(); return SEMICOLON;}
" "                       {col+=yyleng;}
"\t"                    {col+=yyleng;}
"\n"                    {line+=1; col = 1;}

#.*$                        {/*empty*/}
"var"                         {token();return VAR;}
"implicit"                    {token();return IMPLICIT;}
"fun"                    {token();return FUN;}
"int"                    {token();yylval.type=strdup(yytext);return TYPE;}
"void"                    {token();yylval.type=strdup(yytext);return TYPE;}
"string"                    {token();yylval.type=strdup(yytext);return TYPE;}

"\""                        {startString();col+=1;BEGIN(String);}
<String>{
  "`n"                         {col+=2;addString('\n');}
  "\""                         {col+=1;yylval.str= strdup(endString());BEGIN(INITIAL);return STRING;}
  "`t"                         {col+=2;addString('\t');}
  "``"                         {col+=2;addString('`');}
  "`\""                        {col+=2;addString('\"');}
   "\n"                        {fprintf(stderr,"illegal token at line %d col %d: %s\n", line, col, yytext);exit(73);}
   "`".                        {fprintf(stderr,"illegal token at line %d col %d: %s\n", line, col, yytext);exit(73);}
      .                            {col+=1;addString(yytext[0]);}
}

0x[0-9A-F]+                {token();yylval.ival=strtol(yytext,NULL,16);return INT;}
0x[0-9A-F]+[G-Za-z]      {fprintf(stderr,"illegal token at line %d col %d: %s\n", line, col, yytext);exit(73);}
0[0-7]+                {token();yylval.ival=strtol(yytext,NULL,8);return INT;}
[0-9]+                       {token();yylval.ival=atoi(yytext);return INT;}

[0-9]+[A-Za-z]            {fprintf(stderr,"illegal token at line %d col %d: %s\n", line, col, yytext);exit(73);}
"?"						{token(); return TERNARY;}
":"						{token(); return COLON;}
"+"                     {token(); return PLUS;}
"-"                     {token(); return MINUS;}
"*"                     {token();return MUL;}
"/"                     {token();return DIV;}
"%"                     {token();return REM;}
"=="                     {token();return EQ;}
"<>"                     {token();return NE;}
"<"                     {token();return LT;}
"<="                     {token();return LE;}
">"                     {token();return GT;}
">="                     {token();return GE;}
"!"                     {token();return NOT;}
"&"                     {token();return BAND;}
"|"                     {token();return BOR;}
"^"                     {token();return XOR;}
"&&"                     {token();return AND;}
"||"                     {token();return OR;}
"else"                     {token();return ELSE;}
"while"                 {token();return WHILE;}
"otherwise"                 {token();return OTHERWISE;}
"if"                    {token();return IF;}
"("                     {token();return OPEN_PARENT;}
")"                     {token();return CLOSE_PARENT;}
":="                     {token();return ASSIGN;}
","                        {return COMMA;}
"repeat"                     {token();return REPEAT;}
[a-zA-Z][a-zA-Z0-9]*        {token();yylval.name=strdup(yytext); return NAME;}



.                       { fprintf(stderr,"illegal token at line %d col %d: %s\n", line, col, yytext);exit(73); }



%%