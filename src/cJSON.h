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

#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdlib.h>

typedef enum
{
	cJSON_NULL,
	cJSON_False,
	cJSON_True,
	cJSON_Number,
	cJSON_String,
	cJSON_Array,
	cJSON_Object
}
cJSON_Type;

typedef struct cJSON
{
	cJSON_Type type;            // type of this item
	
	// next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem
	struct cJSON *next, *prev;  // sibling items in chain (parent item is array or object)
	struct cJSON *child;        // chain of child items (this item is array or object)
	
	char *string;               // identifying string of this item (parent item is object)
	
	char *valuestring;			// The item's string, if type==cJSON_String
	int valueint;				// The item's number, if type==cJSON_Number
	double valuedouble;			// The item's number, if type==cJSON_Number
}
cJSON;


// Supply a block of JSON, and this returns a cJSON object you can interrogate. Call cJSON_Delete when finished.
cJSON *cJSON_Parse(const char *value);
// Render a cJSON entity to text for transfer/storage. Free the char* when finished.
char *cJSON_Print(cJSON *item);
// Delete a cJSON entity and all subentities.
void cJSON_Delete(cJSON *c);

// Returns the number of items in an array (or object).
int cJSON_GetArraySize(cJSON *array);
// Retrieve item number "item" from array "array". Returns NULL if unsuccessful.
cJSON *cJSON_GetArrayItem(cJSON *array,int item);
// Get item "string" from object. Case insensitive.
cJSON *cJSON_GetObjectItem(cJSON *object,const char *string);

cJSON *cJSON_CreateOrGetObjectItem( cJSON *object, const char *string );

// These calls create a cJSON item of the appropriate type.
cJSON *cJSON_CreateNull( void );
cJSON *cJSON_CreateBoolean( bool );
cJSON *cJSON_CreateNumber( double );
cJSON *cJSON_CreateString( const char * );
cJSON *cJSON_CreateArray( void );
cJSON *cJSON_CreateObject( void );

void cJSON_ForceType( cJSON *, cJSON_Type );

void cJSON_SetBoolean( cJSON *, bool );
void cJSON_SetNumber( cJSON *, double );
void cJSON_SetString( cJSON *, const char * );

// These utilities create an Array of count items.
cJSON *cJSON_CreateIntArray(int *numbers,int count);
cJSON *cJSON_CreateFloatArray(float *numbers,int count);
cJSON *cJSON_CreateDoubleArray(double *numbers,int count);
cJSON *cJSON_CreateStringArray(const char **strings,int count);

// Append item to the specified array/object.
void cJSON_AddItemToArray(cJSON *array, cJSON *item);
void cJSON_AddItemToObject(cJSON *object,const char *string,cJSON *item);

#define cJSON_AddNullToObject(object,name)	cJSON_AddItemToObject(object, name, cJSON_CreateNull())
#define cJSON_AddTrueToObject(object,name)	cJSON_AddItemToObject(object, name, cJSON_CreateTrue())
#define cJSON_AddFalseToObject(object,name)		cJSON_AddItemToObject(object, name, cJSON_CreateFalse())
#define cJSON_AddNumberToObject(object,name,n)	cJSON_AddItemToObject(object, name, cJSON_CreateNumber(n))
#define cJSON_AddStringToObject(object,name,s)	cJSON_AddItemToObject(object, name, cJSON_CreateString(s))

void cJSON_ClearArray( cJSON *array );
#define cJSON_ClearObject( object ) cJSON_ClearArray(object)

#ifdef __cplusplus
}
#endif

#endif
