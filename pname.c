/*
 * /localstorage/z5284381/testing/Pname.c
 *
 ******************************************************************************
  This file contains routines that can be bound to a Postgres backend and
  called by the backend in the process of processing queries.  The calling
  format for these routines is dictated by Postgres architecture.
******************************************************************************/

#include "postgres.h"
#include <ctype.h>
#include <stdint.h>

#include "fmgr.h"
#include "libpq/pqformat.h"		/* needed for send/recv functions */

PG_MODULE_MAGIC;

typedef struct PersonName {
    int32 PersonNameLength;
    char PersonNameVal[FLEXIBLE_ARRAY_MEMBER];
} PersonName;

typedef struct NameParts {
    char * givenName;
	char * familyName;
} NameParts;

bool check_valid_input(char * str);
bool isValidWord(char token);
NameParts* getGivenandFamilyName(char* PersonNameStr);
uint32_t hash_any(char * str, int len);
char * getFirstGiVenName(char *str, int start);

/*****************************************************************************
*  Helper functions
 *****************************************************************************/

// Checks whether the character in the game is within the 
// punctuation of the PersonNames allowed.
bool isValidWord(char token) {

	bool valid = true;

	if (token == '\'' || token == '-' || (isalpha(token) != 0) ) {
		valid = true;
	} else {
		valid = false;
	}

	return valid;
}

char * getFirstGiVenName(char *str, int start) {
	
	int length;
	char *title, *cpy_str;

	length = strlen(str);
	cpy_str = (char *) palloc(length);
	
	strcpy(cpy_str, str);

	cpy_str = strtok(cpy_str, ",");
	cpy_str = strtok(NULL, ",");
	title = strtok(cpy_str, " ");
	
	return title;
}

// Checks the validity of the input.
bool check_valid_input(char * str) {

	bool validStr = true;
	int length = strlen(str);
	int lastSpace = 0;
	int commaCount = 0;
	int commaPlace = 0;

	for (int i = 0; i < length; i++) {
		if (str[i] == ',') {
			commaCount = commaCount + 1;
			commaPlace = i;
		} 
	}

	if (commaCount != 1) {
		validStr = false;
	} else if (isupper(str[0]) == 0) {
		validStr = false;
	} else if ((commaPlace > 0 && commaCount == 1) && (
		isValidWord(str[commaPlace-1]) == 0)) {
		validStr = false;
	}	
	else {
		
		char * title;

		for (int i = 0; i < commaPlace; i++) {
			if (str[i] == 32) {
				i = i + 1;
			}
		
			if (isValidWord(str[i]) == false) {
				validStr = false;
				break;	
			}
		}

		if (validStr == true) {
			
			int start = commaPlace + 1;

			if (str[start] == 32) {
				start = start + 1;
			}

			if (isupper(str[start]) == 0) {
				validStr = false;
			}

			title = getFirstGiVenName(str, start);
			
			if ((strcmp(title, "Dr") == 0) || (strcmp(title, "Prof") == 0) 
			|| (strcmp(title, "Mr") == 0) || (strcmp(title, "Ms") == 0) || 
			(strcmp(title, "Mrs") == 0)) {
				validStr = false;
			}

			else {
				for (int i = start; i < length; i++) {
					if (str[i] == 32) {
						if(i == lastSpace + 2) {
							validStr = false;
							break;
						}
						lastSpace = i;
						i = i + 1;
					}
					if (isValidWord(str[i]) == false) {
						validStr = false;
						break;
					}
				}

				if (validStr == true) {
					if (length - lastSpace == 2) {
						validStr = false;
					}
				}
			}
			
		}

	}
	
	return validStr;
}


NameParts* getGivenandFamilyName(char* PersonNameStr) {
	
	int length, givenLen, familyLen; 
	char *str;
	char *givenName; 
	char *familyName;
	NameParts *part;
	
	length = strlen(PersonNameStr) + 1;
	str = (char *) palloc(length);
	
	strcpy(str, PersonNameStr);

	familyName = strtok(str, ",");
	givenName = strtok(NULL, ",");

	givenLen = strlen(givenName) + 1;
	familyLen = strlen(familyName) + 1;;

	if (givenName[0] == 32) {
		for (int i = 0; i < givenLen - 1; i++) {
			givenName[i] = givenName[i+1];
		}
	}

	part = (NameParts*) palloc(sizeof(NameParts)); 
	
	part->familyName = (char*) palloc(familyLen);
	strcpy(part->familyName, familyName);

	part->givenName = (char*) palloc(givenLen);
	strcpy(part->givenName, givenName);

	pfree(str);

	return part;
}

/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

/* Basic input function which checks the input string matches all the 
   constraints levied in the specification and then converts it into
   the internal representation with further returning that.
*/

PG_FUNCTION_INFO_V1(PersonName_in);

Datum
PersonName_in(PG_FUNCTION_ARGS) {
	
	char *str = PG_GETARG_CSTRING(0);

	int lengthStr; 
	char *cpy_str;
	bool valid_input;
	int32_t length;
	PersonName *result;
	
	lengthStr = strlen(str) + 1;
	cpy_str = (char *) palloc(lengthStr);
	
	strcpy(cpy_str, str);

	valid_input  = check_valid_input(cpy_str);

	if (valid_input == false) {
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("invalid input syntax for type %s: \"%s\"",
						"PersonName", str)));
	}

	length = strlen(str) + 1;
	result = (PersonName *) palloc(VARHDRSZ + length);

	SET_VARSIZE(result, VARHDRSZ + length);
	memcpy(result->PersonNameVal, str, length);

	pfree(cpy_str);
	PG_RETURN_POINTER(result);

}

/* Given the internal representation of the PersonName output the PersonName. */
PG_FUNCTION_INFO_V1(PersonName_out);

Datum
PersonName_out(PG_FUNCTION_ARGS) {
	
	PersonName *a = (PersonName *) PG_GETARG_POINTER(0);
	char *result;

	NameParts * part;

	part = getGivenandFamilyName(a->PersonNameVal);
	result = psprintf("%s,%s", part->familyName, part->givenName);

	PG_RETURN_CSTRING(result);
}

// /*****************************************************************************
//	 						Hash based indexing function						*
//  *****************************************************************************/

// Hashing function : used the following link as a reference : 
// https://stackoverflow.com/questions/7666509/hash-function-for-string

uint32_t hash_any(char *key, int len)
{
    uint32_t hash, i;
    
	for (hash = i = 0; i < len; ++i) {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

PG_FUNCTION_INFO_V1(Person_hash);

Datum
Person_hash(PG_FUNCTION_ARGS)
{
	PersonName *a = (PersonName *) PG_GETARG_POINTER(0);

	int length;
	char * str;
	bool spaceFound;
	uint32_t val;

	length = strlen(a->PersonNameVal);
	str = (char *) palloc(length + 1);
	strcpy(str, a->PersonNameVal);

	spaceFound = false;

	for (int i = 0; i < length; i++) {
		if (str[i] == 44) {
			if (str[i+1] == 32) {
				spaceFound = true;
				for (int j = i + 1; j < length; j++) {
					str[j] = str[j+1];
				}
			}
			break;
		}
	}

	if (spaceFound == true) {
		length = length - 1;
	}

	val = hash_any(str, length);
	val = val & 0x7fffffff;

	pfree(str);
	PG_RETURN_UINT32(val);
}

/****************************************************************************
*             Internal comparator function for personNames				    * 												
*****************************************************************************/

static int Person_comp_internal (PersonName * a, PersonName *b) {

	NameParts * partA = getGivenandFamilyName(a->PersonNameVal);
	NameParts * partB = getGivenandFamilyName(b->PersonNameVal);
	
	int cmpA  = strcmp((const char*) partA->familyName, 
				(const char*) partB->familyName);
	int cmpB  = strcmp((const char*) partA->givenName, 
				(const char*) partB->givenName);


	if (cmpA == 0 && cmpB == 0) {
		return 0;
	}  else if (cmpA > 0 || (cmpA == 0 && cmpB > 0)) {
		return 1;
	} else if (cmpA > 0 || (cmpA == 0 && cmpB >= 0)) {
		return 1;
	} else if (cmpA < 0 || (cmpA == 0 && cmpB < 0)) {
		return -1;
	} else if (cmpA < 0 || (cmpA == 0 && cmpB <= 0)) {
		return -1;
	} else if (cmpA != 0 && cmpB != 0) {
		return 1;
	}
 
	return 0;

}

/****************************************************************************
*                     All of the defined operators for the 					*
*		                    PersonName data Type                            *
*				         (>, <, =, <>, >=, <=)								* 												
*****************************************************************************/

PG_FUNCTION_INFO_V1(Pname_equal);

Datum
Pname_equal(PG_FUNCTION_ARGS)
{	
	PersonName *a = (PersonName *) PG_GETARG_POINTER(0);
	PersonName *b = (PersonName *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(Person_comp_internal(a, b) == 0);

}

PG_FUNCTION_INFO_V1(Pname_notequal);

Datum
Pname_notequal(PG_FUNCTION_ARGS)
{
	PersonName *a = (PersonName *) PG_GETARG_POINTER(0);
	PersonName *b = (PersonName *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(Person_comp_internal(a,b) != 0);
}


PG_FUNCTION_INFO_V1(Pname_greaterthan);

Datum
Pname_greaterthan(PG_FUNCTION_ARGS)
{
	PersonName *a = (PersonName *) PG_GETARG_POINTER(0);
	PersonName *b = (PersonName *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(Person_comp_internal(a,b) > 0);
}


PG_FUNCTION_INFO_V1(Pname_greaterthanEquals);

Datum
Pname_greaterthanEquals(PG_FUNCTION_ARGS)
{
	PersonName *a = (PersonName *) PG_GETARG_POINTER(0);
	PersonName *b = (PersonName *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(Person_comp_internal(a,b) >= 0);

}


PG_FUNCTION_INFO_V1(Pname_lessthan);

Datum
Pname_lessthan(PG_FUNCTION_ARGS)
{
	PersonName *a = (PersonName *) PG_GETARG_POINTER(0);
	PersonName *b = (PersonName *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(Person_comp_internal(a,b) < 0);
}


PG_FUNCTION_INFO_V1(Pname_lessthanEquals);

Datum
Pname_lessthanEquals(PG_FUNCTION_ARGS)
{
	PersonName *a = (PersonName *) PG_GETARG_POINTER(0);
	PersonName *b = (PersonName *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(Person_comp_internal(a,b) <= 0);
}

// /****************************************************************************
// *                     Functions for the Pname type                          *
// *				  (family(Pname), given(Pname), show(Pname) 				* 												
// *****************************************************************************/

PG_FUNCTION_INFO_V1(family);

Datum
family(PG_FUNCTION_ARGS)
{
	PersonName *PersonNamestr = (PersonName *) PG_GETARG_POINTER(0);
	
	int lengthFam, length;
	char *familyName;
	char * str;

	text* textFam;
	
	length = strlen(PersonNamestr->PersonNameVal) + 1;
	str = (char *) palloc(length);
	
	strcpy(str, PersonNamestr->PersonNameVal);

	familyName = strtok(str, ",");
	lengthFam = strlen(familyName) + 1;

	textFam = (text *) palloc(lengthFam + VARHDRSZ);
    SET_VARSIZE(textFam, lengthFam + VARHDRSZ);

	memcpy((void *) VARDATA(textFam), /* destination */
           (const void*) familyName, /* source */
           lengthFam);
	
	pfree(str);

	PG_RETURN_TEXT_P(textFam);
}

PG_FUNCTION_INFO_V1(given);

Datum
given(PG_FUNCTION_ARGS)
{
	PersonName *PersonNamestr = (PersonName *) PG_GETARG_POINTER(0);

	int length, givenLen, lengthGiven; 
	char *str;
	char *givenName;

	text* textGiven;

	length = strlen(PersonNamestr->PersonNameVal) + 1;
	str = (char *) palloc(length);
	
	strcpy(str, PersonNamestr->PersonNameVal);

	givenName = strtok(str, ",");
	givenName = strtok(NULL, ",");
	givenLen = strlen(givenName) + 1;

	if (givenName[0] == 32) {
		for (int i = 0; i < givenLen - 1; i++) {
			givenName[i] = givenName[i+1];
		}
	}

	lengthGiven = strlen(givenName) + 1;

	textGiven = (text *) palloc(lengthGiven + VARHDRSZ);
    SET_VARSIZE(textGiven, lengthGiven + VARHDRSZ);

	memcpy((void *) VARDATA(textGiven), /* destination */
           (const void*) givenName, /* source */
           lengthGiven);
	
	pfree(str);

	PG_RETURN_TEXT_P(textGiven);
}

PG_FUNCTION_INFO_V1(show);

Datum
show(PG_FUNCTION_ARGS)
{
	PersonName *PersonNamestr = (PersonName *) PG_GETARG_POINTER(0);

	int lengthOverall, givenLen, familyLen;
	char *familyName;
	char *givenName;

	NameParts *part;
	text* textGiven;
	
	part = getGivenandFamilyName(PersonNamestr->PersonNameVal);
	
	givenLen = strlen(part->givenName) + 1;
	familyLen = strlen(part->familyName) + 1;

	familyName = (char *) palloc(familyLen);
	strcpy(familyName, part->familyName);

	givenName = (char *) palloc(givenLen + familyLen + 1);
	strcpy(givenName, part->givenName);

	strtok(givenName, " ");
	strcat(givenName, " ");
	strcat(givenName, familyName);

	lengthOverall = strlen(givenName) + 1;

	textGiven = (text *) palloc(lengthOverall + VARHDRSZ);
    SET_VARSIZE(textGiven, lengthOverall + VARHDRSZ);

	memcpy((void *) VARDATA(textGiven), /* destination */
           (const void*) givenName, /* source */
           lengthOverall);
	
	PG_RETURN_TEXT_P(textGiven);
}

// Comparator function used by the B-tree indexes which uses
// the internal comparaison functions.
PG_FUNCTION_INFO_V1(Person_comp);

Datum
Person_comp(PG_FUNCTION_ARGS)
{
	PersonName *a = (PersonName *) PG_GETARG_POINTER(0);
	PersonName *b = (PersonName *) PG_GETARG_POINTER(1);

	PG_RETURN_INT32(Person_comp_internal(a,b));
}

