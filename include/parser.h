#ifndef LEXER_HPP
#define LEXER_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/*
 * No operations will alter the string
 * and the structure is cheap to copy, so
 * when alternate paths are needed making 
 * a copy is a viable strategy.
 */
typedef struct{
	char *string;
	int index;
} Parser;

void new_parser(Parser *parser, char *string);

void skip_whitespace(Parser * parser);

bool at_end(Parser *parser);

bool expect( Parser *parser, char *match);

/*
 * Returns either the character that matched or
 * 0 for no match
 */
char oneof_char(Parser *parser, char *options);

int take_while(Parser *parser, char **out, bool (*cond)(char c));

bool is_digit(char c);

#endif
