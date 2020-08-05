/*
**
** cstring.c (C) Josh PH3NOM Pearson 2013
** Cheapo C++ syle String Lib for plain C
**
*/

#ifndef CSTRING_H
#define CSTRING_H

typedef struct
{
    unsigned int Size;
    char * String;
} String;

unsigned char char_is_break( char c );
void cstr_copy( char *src, char *dst );
unsigned int cstr_len( char *src );
String * NewString();
void StringSet( String * s, char * str );
void StringConcat( String * s, char * str );
void StringInsert( String * s, char * str );
void StringDeconstruct( String * s );

#endif
