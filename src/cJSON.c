/*
  Copyright (c) 2009 Dave Gamble
  Copyright (c) 2009 The OpenTyrian Development Team
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

// cJSON
// JSON parser in C.

#include "cJSON.h"
#include "mingw_fixes.h"

#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void *(*cJSON_malloc)( size_t ) = malloc;
static void *(*cJSON_realloc)( void *, size_t ) = realloc;
static void (*cJSON_free)( void *ptr ) = free;

// helper for compilers without strdup
static char *cJSON_strdup( const char *str )
{
	size_t size = strlen(str) + 1;
	char *copy = (char *)cJSON_malloc(size);
	
	if (copy != NULL)
		memcpy(copy, str, size);
	
	return copy;
}

// helper for compilers without strcasecmp
static int cJSON_strcasecmp( const char *s1, const char *s2 )
{
	for(; tolower(*(const unsigned char *)s1) == tolower(*(const unsigned char *)s2); ++s1, ++s2)
		if (*s1 == 0)
			return 0;
	return tolower(*(const unsigned char *)s1) - tolower(*(const unsigned char *)s2);
}

// construct empty item
static cJSON *cJSON_NewItem( cJSON_Type type )
{
	cJSON *node = (cJSON *)cJSON_malloc(sizeof(cJSON));
	
	if (node != NULL)
	{
		node->type = type;
		node->child = 
		node->prev =
		node->next = NULL;
		
		node->string = NULL;
		
		node->valuestring = NULL;
		node->valueint =
		node->valuedouble = 0;
	}
	
	return node;
}

// destroy item chain
void cJSON_Delete( cJSON *item )
{
	while (item != NULL)
	{
		if (item->child)
			cJSON_Delete(item->child);
		
		cJSON *next = item->next;
		
		if (item->string)
			cJSON_free(item->string);
		if (item->valuestring)
			cJSON_free(item->valuestring);
		cJSON_free(item);
		
		item = next;
	}
}

// parser/emitter prototypes

static const char *parse_value( cJSON *item, const char *in );
static char *print_value(cJSON *item,int depth);

static const char *parse_number( cJSON *item, const char *in );
static char *print_number( cJSON *item );

static const char *parse_string(cJSON *item,const char *str);
static char *print_string(cJSON *item);

static const char *parse_array_or_object( cJSON *item, const char *in, cJSON_Type type );
static char *print_array(cJSON *item,int depth);
static char *print_object(cJSON *item,int depth);

static inline const char *parse_array( cJSON *item, const char *in )
{
	return parse_array_or_object(item, in, cJSON_Array);
}
static inline const char *parse_object( cJSON *item, const char *in )
{
	return parse_array_or_object(item, in, cJSON_Object);
}

// helper to skip whitespace
static inline const char *skip_space( const char *str )
{
	if (str != NULL)
		while (isspace(*str))
			++str;
	return str;
}

// parse root of JSON into cJSON item
cJSON *cJSON_Parse( const char *in )
{
	cJSON *item = cJSON_NewItem(cJSON_NULL);
	
	if (item != NULL)
	{
		if (parse_value(item, skip_space(in)) == NULL)  // if malformed or out-of-memory
		{
			cJSON_Delete(item);
			item = NULL;
		}
	}
	
	return item;
}

// emit cJSON item as JSON value
char *cJSON_Print( cJSON *item )
{
	return print_value(item, 0);
}

// parse JSON value into cJSON item
static const char *parse_value( cJSON *item, const char *in )
{
	if (in == NULL)
		return in;
	
	if (!strncmp(in, "null", 4))
	{
		item->type = cJSON_NULL;
		return in + 4;
	}
	if (!strncmp(in, "false", 5))
	{
		item->type = cJSON_False;
		return in + 5;
	}
	if (!strncmp(in, "true", 4))
	{
		item->type = cJSON_True;
		return in + 4;
	}
	if (*in == '\"')
		return parse_string(item, in);
	if (*in == '-' || (*in >= '0' && *in <= '9'))
		return parse_number(item, in);
	if (*in == '[')
		return parse_array(item, in);
	if (*in == '{')
		return parse_object(item, in);
	
	return NULL;  // malformed: expected value
}

// emit cJSON item as JSON value
static char *print_value( cJSON *item, int depth )
{
	char *out = NULL;
	
	switch (item->type)
	{
	case cJSON_NULL:
		out = cJSON_strdup("null");
		break;
	case cJSON_False:
		out = cJSON_strdup("false");
		break;
	case cJSON_True:
		out = cJSON_strdup("true");
		break;
	case cJSON_Number:
		out = print_number(item);
		break;
	case cJSON_String:
		out = print_string(item);
		break;
	case cJSON_Array:
		out = print_array(item, depth);
		break;
	case cJSON_Object:
		out = print_object(item, depth);
		break;
	}
	
	return out;
}

// parse JSON number value into cJSON item
static const char *parse_number( cJSON *item, const char *in )
{
	double n = 0;
	int sign = 1, decimal_shift = 0;
	int exponent_sign = 1, exponent = 0;
	
	if (*in == '-')
		sign = -1, ++in;
	
	// integer part
	if (*in == '0')
		++in;
	else if (*in >= '1' && *in <= '9')
		do
			n = (n * 10.0) + (*(in++) - '0');
		while (*in >= '0' && *in <= '9');
	
	// fractional part
	if (*in == '.')
	{
		++in;
		
		while (*in >= '0' && *in <= '9')
			n = (n * 10.0) + (*(in++) - '0'), decimal_shift--;
	}
	
	// exponent part
	if (*in == 'e' || *in == 'E')
	{
		++in;
		
		if (*in == '+')
			++in;
		else if (*in == '-')
			exponent_sign = -1, ++in;
		
		while (*in >= '0' && *in <= '9')
			exponent = (exponent * 10) + (*(in++) - '0');
	}
	
	// number = +/- number.fraction * (10 ^ +/- exponent)
	n = sign * n * pow(10.0, decimal_shift + exponent_sign * exponent);
	
	item->valuedouble = n;
	item->valueint = n;
	item->type = cJSON_Number;
	
	return in;
}

// emit string containing numeric value of cJSON item
static char *print_number( cJSON *item )
{
	char *str = (char *)cJSON_malloc(DBL_DIG + 10);
	snprintf(str, DBL_DIG + 10, "%.*g", DBL_DIG, item->valuedouble);
	return str;
}

// Parse the input text into an unescaped cstring, and populate item.
static const unsigned char firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
static const char *parse_string(cJSON *item,const char *str)
{
	const char *ptr=str+1;char *ptr2;char *out;int len=0;unsigned uc;
	if (*str!='\"') return 0;	// not a string!
	
	while (*ptr!='\"' && *ptr>31 && ++len) if (*ptr++ == '\\') ptr++;	// skip escaped quotes.
	
	out=(char*)cJSON_malloc(len+1);	// This is how long we need for the string, roughly.
	if (!out) return 0;
	
	ptr=str+1;ptr2=out;
	while (*ptr!='\"' && *ptr>31)
	{
		if (*ptr!='\\') *ptr2++=*ptr++;
		else
		{
			ptr++;
			switch (*ptr)
			{
				case 'b': *ptr2++='\b';	break;
				case 'f': *ptr2++='\f';	break;
				case 'n': *ptr2++='\n';	break;
				case 'r': *ptr2++='\r';	break;
				case 't': *ptr2++='\t';	break;
				case 'u':	 // transcode utf16 to utf8. DOES NOT SUPPORT SURROGATE PAIRS CORRECTLY.
					sscanf(ptr+1,"%4x",&uc);	// get the unicode char.
					len=3;if (uc<0x80) len=1;else if (uc<0x800) len=2;ptr2+=len;
					
					switch (len) {
						case 3: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
						case 2: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
						case 1: *--ptr2 =(uc | firstByteMark[len]);
					}
					ptr2+=len;ptr+=4;
					break;
				default:  *ptr2++=*ptr; break;
			}
			ptr++;
		}
	}
	*ptr2=0;
	if (*ptr=='\"') ptr++;
	item->valuestring=out;
	item->type=cJSON_String;
	return ptr;
}

// Render the cstring provided to an escaped version that can be printed.
static char *print_string_ptr(const char *str)
{
	const char *ptr;char *ptr2,*out;int len=0;
	
	ptr=str;while (*ptr && ++len) {if (*ptr<32 || *ptr=='\"' || *ptr=='\\') len++;ptr++;}
	
	out=(char*)cJSON_malloc(len+3);
	ptr2=out;ptr=str;
	*ptr2++='\"';
	while (*ptr)
	{
		if (*ptr>31 && *ptr!='\"' && *ptr!='\\') *ptr2++=*ptr++;
		else
		{
			*ptr2++='\\';
			switch (*ptr++)
			{
				case '\\':	*ptr2++='\\';	break;
				case '\"':	*ptr2++='\"';	break;
				case '\b':	*ptr2++='b';	break;
				case '\f':	*ptr2++='f';	break;
				case '\n':	*ptr2++='n';	break;
				case '\r':	*ptr2++='r';	break;
				case '\t':	*ptr2++='t';	break;
				default: ptr2--;	break;	// eviscerate with prejudice.
			}
		}
	}
	*ptr2++='\"';*ptr2++=0;
	return out;
}
// Invote print_string_ptr (which is useful) on an item.
static char *print_string(cJSON *item)
{
	return (item->valuestring != NULL) ? print_string_ptr(item->valuestring) : cJSON_strdup("");
}

// parse JSON array/object into cJSON item chain
static const char *parse_array_or_object( cJSON *const item, const char *in, cJSON_Type type )
{
	assert(type == cJSON_Array || type == cJSON_Object);
	
	const char opening = (type == cJSON_Object) ? '{' : '[',
	           closing = (type == cJSON_Object) ? '}' : ']';
	
	if (*in != opening)  // not an array/object!
		return NULL;
	else
		in = skip_space(++in);
	
	item->type = type;
	
	if (*in == closing)  // empty array/object
		return ++in;
	
	cJSON *prev_child = NULL;
	for (; ; )
	{
		cJSON *child = cJSON_NewItem(cJSON_NULL);
		if (child == NULL)  // memory fail
			return NULL;
		
		if (prev_child == NULL)
		{
			// attach first child to parent
			item->child = child;
		}
		else
		{
			// attach other children to older sibling
			prev_child->next = child;
			child->prev = prev_child;
		}
		
		if (type == cJSON_Object)
		{
			// object children have identifier string
			
			in = skip_space(parse_string(child, skip_space(in)));
			if (in == NULL)  // malformed or memory fail
				return NULL;
			
			// parse_string parses into the item's value; we can use it to parse the identifier string, we just have to move the results
			child->string = child->valuestring;
			child->valuestring = NULL;
			
			if (*in != ':')  // malformed
				return NULL;
			else
				++in;
		}
		
		in = skip_space(parse_value(child, skip_space(in)));
		if (in == NULL)  // malformed or memory fail
			return NULL;
		
		prev_child = child;
		
		if (*in == ',')
			++in;
		else
			break;
	}
	
	if (*in == closing)  // end of array/object
		return ++in;
	
	return NULL;  // malformed
}

// Render an array to text
static char *print_array(cJSON *item,int depth)
{
	char *out, *ptr;
	size_t len = 3;  // minimum needed to print empty array
	
	ptr = out = (char*)cJSON_malloc(len);
	
	strcpy(ptr, "[");
	ptr += 1;
	
	cJSON *child = item->child;
	
	while (child)
	{
		char *ret = print_value(child, depth + 1);
		if (!ret)
		{
			cJSON_free(out);
			return NULL;
		}
		size_t ret_len = strlen(ret);
		
		len += ret_len + 2;
		ptr = out = (char*)cJSON_realloc(out, len);
		ptr += strlen(out);
		
		strcpy(ptr, ret);  // strcat(out, ret);
		ptr += ret_len;
		
		cJSON_free(ret);
		
		if (child->next)
		{
			strcpy(ptr, ", ");  // strcat(out, ", ");
			ptr += 2;
		}
		
		child = child->next;
	}
	
	strcpy(ptr, "]");  // strcat(out, "]");
	
	return out;
}

// Render an object to text.
static char *print_object(cJSON *item,int depth)
{
	char *out, *ptr;
	size_t len = 4 + depth;  // minimum needed to print empty object
	
	++depth;
	
	ptr = out = (char*)cJSON_malloc(len);
	
	strcpy(ptr, "{\n");
	ptr += 2;
	
	cJSON *child = item->child;
	
	while (child)
	{
		char *str = print_string_ptr(child->string);
		if (!str)
		{
			cJSON_free(out);
			return NULL;
		}
		size_t str_len = strlen(str);
		
		char *ret = print_value(child, depth);
		if (!ret)
		{
			cJSON_free(str);
			cJSON_free(out);
			return NULL;
		}
		size_t ret_len = strlen(ret);
		
		len += depth + str_len + ret_len + 4;
		out = (char*)cJSON_realloc(out, len);
		ptr = out + strlen(out);
		
		for (int i = 0; i < depth; ++i)
			*(ptr++) = '\t';
		
		strcpy(ptr, str);  // strcat(out, str);
		ptr += str_len;
		
		cJSON_free(str);
		
		strcpy(ptr, ":\t");  // strcat(out, ":\t");
		ptr += 2;
		
		strcpy(ptr, ret);  // strcat(out, ret);
		ptr += ret_len;
		
		cJSON_free(ret);
		
		if (child->next)
		{
			strcpy(ptr, ",\n");  // strcat(out, ",\n");
			ptr += 2;
		}
		else
		{
			strcpy(ptr, "\n");  // strcat(out, "\n");
			ptr += 1;
		}
		
		child = child->next;
	}
	
	--depth;
	
	for (int i = 0; i < depth; ++i)
		*(ptr++) = '\t';
	
	strcpy(ptr, "}");  // strcat(out, "}");
	
	return out;
}

// Get Array size/item / object item.
int cJSON_GetArraySize( cJSON *array )
{
	int size = 0;
	cJSON *item = array->child;
	while (item != NULL)
		item = item->next, ++size;
	return size;
}
cJSON *cJSON_GetArrayItem( cJSON *array, int index )
{
	cJSON *item = array->child;
	while (item != NULL && index > 0)
		item = item->next, --index;
	return item;
}
cJSON *cJSON_GetObjectItem( cJSON *object, const char *string)
{
	cJSON *item=object->child;
	while (item != NULL && cJSON_strcasecmp(item->string, string) != 0)
		item = item->next;
	return item;
}

cJSON *cJSON_CreateOrGetObjectItem( cJSON *object, const char *string )
{
	cJSON *child = cJSON_GetObjectItem(object, string);
	if (child == NULL)
		cJSON_AddItemToObject(object, string, child = cJSON_CreateNull());
	
	return child;
}

// Utility for array list handling.
static void suffix_object(cJSON *prev,cJSON *item) {prev->next=item;item->prev=prev;}

// Add item to array/object.
void   cJSON_AddItemToArray(cJSON *array, cJSON *item)						{cJSON *c=array->child;if (!c) {array->child=item;} else {while (c && c->next) c=c->next; suffix_object(c,item);}}
void   cJSON_AddItemToObject(cJSON *object,const char *string,cJSON *item)	{if (item->string) cJSON_free(item->string);item->string=cJSON_strdup(string);cJSON_AddItemToArray(object,item);}

// remove all items from array/object
void cJSON_ClearArray( cJSON *array )
{
	cJSON_Delete(array->child);
	array->child = NULL;
}

// create basic types

cJSON *cJSON_CreateNull( void )
{
	return cJSON_NewItem(cJSON_NULL);
}
cJSON *cJSON_CreateBoolean( bool value )
{
	return cJSON_NewItem(value ? cJSON_True : cJSON_False);
}
cJSON *cJSON_CreateNumber( double value )
{
	cJSON *item = cJSON_NewItem(cJSON_Number);
	item->valueint = item->valuedouble = value;
	return item;
}
cJSON *cJSON_CreateString( const char *value )
{
	cJSON *item = cJSON_NewItem(cJSON_String);
	item->valuestring = cJSON_strdup(value);
	return item;
}
cJSON *cJSON_CreateArray( void )
{
	return cJSON_NewItem(cJSON_Array);
}
cJSON *cJSON_CreateObject( void )
{
	return cJSON_NewItem(cJSON_Object);
}

void cJSON_ForceType( cJSON *item, cJSON_Type type )
{
	if (item->type != type)
	{
		cJSON_Delete(item->child);
		item->child = NULL;
		
		item->type = type;
	}
}

void cJSON_SetBoolean( cJSON *item, bool value )
{
	cJSON_ForceType(item, value ? cJSON_True : cJSON_False);
}
void cJSON_SetNumber( cJSON *item, double value )
{
	cJSON_ForceType(item, cJSON_Number);
	item->valueint = item->valuedouble = value;
}
void cJSON_SetString( cJSON *item, const char *value )
{
	cJSON_ForceType(item, cJSON_String);
	cJSON_free(item->valuestring);
	item->valuestring = cJSON_strdup(value);
}

// Create Arrays:
cJSON *cJSON_CreateIntArray(int *numbers,int count)				{int i;cJSON *n=0,*p=0,*a=cJSON_CreateArray();for(i=0;i<count;i++){n=cJSON_CreateNumber(numbers[i]);if(!i)a->child=n;else suffix_object(p,n);p=n;}return a;}
cJSON *cJSON_CreateFloatArray(float *numbers,int count)			{int i;cJSON *n=0,*p=0,*a=cJSON_CreateArray();for(i=0;i<count;i++){n=cJSON_CreateNumber(numbers[i]);if(!i)a->child=n;else suffix_object(p,n);p=n;}return a;}
cJSON *cJSON_CreateDoubleArray(double *numbers,int count)		{int i;cJSON *n=0,*p=0,*a=cJSON_CreateArray();for(i=0;i<count;i++){n=cJSON_CreateNumber(numbers[i]);if(!i)a->child=n;else suffix_object(p,n);p=n;}return a;}
cJSON *cJSON_CreateStringArray(const char **strings,int count)	{int i;cJSON *n=0,*p=0,*a=cJSON_CreateArray();for(i=0;i<count;i++){n=cJSON_CreateString(strings[i]);if(!i)a->child=n;else suffix_object(p,n);p=n;}return a;}
