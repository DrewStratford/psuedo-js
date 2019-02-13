%{
#include <stdio.h>
#include <string.h>
%}

%%
[0-9]+\.[0-9]+	yylval=atof(yytext);return FLOAT;
[0-9]+		yylval=atoi(yytext); return INT;
[a-zA-Z]	yylval=strdup(yytext); return IDENTIFIER;
let			return LET;
%%
