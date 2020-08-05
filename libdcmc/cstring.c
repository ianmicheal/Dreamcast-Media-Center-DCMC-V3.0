#include <string.h>
#include <malloc.h>

#include "cstring.h"

unsigned char char_is_break( char c )
{
    return (c=='\n' || c=='\0');
}

void cstr_copy( char *src, char *dst )
{
     int i = -1;
     while(!char_is_break(src[++i]))
         dst[i] = src[i];
     dst[i] = '\0'; 
}

void cstr_copy_len( char *src, char *dst, unsigned int len )
{
     int i;
     for(i=0;i<len;i++)
         dst[i] = src[i];
     dst[len] = '\0'; 
}

unsigned int cstr_len( char *src )
{
     unsigned int i = 0;
     while(!char_is_break(src[i++]));
     return i;
}

unsigned int cstr_find_last( char *src, char c )
{
     unsigned int i = 0, last_index = i;
     
     while(!char_is_break(src[i]))
     {
         if(src[i]==c)
            last_index = i;
         i++;
     }
     return last_index;
}

unsigned int cstr_find_2nd_to_last( char *src, char c )
{
     unsigned int i = 0, last_index = i;
     
     while(!char_is_break(src[i]))
     {
         if(src[i]==c)
            last_index = i;
         i++;
     }
     
     i-=1;
     
     while(--i)
     {
         if(src[i]==c)
             return i;
     }
     
     return last_index;
}

/* This is my cheapo C++ String class for plain C (C) PH3NOM 2013 */

String * NewString()
{
    String *s = malloc(sizeof(String));
    s->Size=0;
    s->String=NULL;
    return s;
}

void StringSet( String * s, char * str )
{
    s->Size = cstr_len(str);
    
    if(s->String!=NULL)
        free(s->String);
        
    s->String = malloc(sizeof(char)*s->Size);
    
    cstr_copy(str, s->String);
}

void StringSetLen( String * s, char * str, unsigned int len )
{
    s->Size = len;
    
    if(s->String!=NULL)
        free(s->String);
        
    s->String = malloc(sizeof(char)*s->Size);
    
    cstr_copy_len(str, s->String, len);
}

void StringConcat( String * s, char * str )
{
    String * temp = NewString();
    StringSet(temp, s->String);
    
    s->Size = cstr_len(str);
    
    free(s->String);    
    s->String = malloc(sizeof(char)*s->Size+sizeof(char)*temp->Size);
    
    cstr_copy(temp->String, s->String);
    cstr_copy(str, s->String+temp->Size-1);
    s->Size+=temp->Size-1;
}

void StringInsert( String * s, char * str )
{
    String * temp = NewString();
    StringSet(temp, s->String);
    
    s->Size = cstr_len(str);
    
    free(s->String);    
    s->String = malloc(sizeof(char)*s->Size+sizeof(char)*temp->Size);
    
    cstr_copy(str, s->String);
    cstr_copy(temp->String, s->String+s->Size-1);
    s->Size+=temp->Size;
}

void StringDeconstruct( String * s )
{
    free(s->String);
    free(s);
    s=NULL;
}
