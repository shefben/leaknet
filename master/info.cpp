// info.c
#include "info.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_KV_LEN 127
/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
===============
*/
const char *Info_ValueForKey ( const char *s, const char *key )
{
	char	pkey[512];
	static	char value[4][512];	// use two buffers so compares
								// work without stomping on each other
	static	int	valueindex;
	char	*o;
	
	valueindex = (valueindex + 1) % 4;
	if (*s == '\\')
		s++;
	while (1)
	{
		int nCount;

		nCount = 0;
		o = pkey;
		while ( (*s != '\\') && (nCount < sizeof(pkey)-1) )
		{
			if (!*s)
				return "";

			*o++ = *s++;
			nCount++;
		}
		*o = 0;
		s++;

		nCount = 0;
		o = value[valueindex];
		while ( (*s != '\\') && *s && (nCount < sizeof(value[valueindex])-1) )
		{
			*o++ = *s++;
			nCount++;
		}
		*o = 0;

		if (!strcmp (key, pkey) )
			return value[valueindex];

		if (!*s)
			return "";
		s++;
	}
}

void Info_RemoveKey ( char *s, const char *key )
{
	char	*start;
	char	pkey[512];
	char	value[512];
	char	*o;

	if (strstr (key, "\\"))
	{
		return;
	}

	while (1)
	{
		int nCount;

		start = s;
		if (*s == '\\')
			s++;

		nCount = 0;
		o = pkey;
		while ( (*s != '\\') && (nCount < sizeof(pkey)-1) )
		{
			if (!*s)
				return;

			*o++ = *s++;
			nCount++;
		}
		*o = 0;
		s++;

		nCount = 0;
		o = value;
		while ( (*s != '\\') && *s && (nCount < sizeof(value)-1) )
		{
			if (!*s)
				return;

			*o++ = *s++;
			nCount++;
		}
		*o = 0;

		if (!strcmp (key, pkey) )
		{
			strcpy (start, s);	// remove this part
			return;
		}

		if (!*s)
			return;
	}

}

void Info_RemovePrefixedKeys (char *start, char prefix)
{
	char	*s;
	char	pkey[512];
	char	value[512];
	char	*o;

	s = start;

	while (1)
	{
		int nCount;

		if (*s == '\\')
			s++;

		nCount = 0;
		o = pkey;
		while ( (*s != '\\') && (nCount < sizeof(pkey)-1) )
		{
			if (!*s)
				return;

			*o++ = *s++;
			nCount++;
		}
		*o = 0;
		s++;

		nCount = 0;
		o = value;
		while ( (*s != '\\') && *s && (nCount < sizeof(value)-1) )
		{
			if (!*s)
				return;

			*o++ = *s++;
			nCount++;
		}
		*o = 0;

		if (pkey[0] == prefix)
		{
			Info_RemoveKey (start, pkey);
			s = start;
		}

		if (!*s)
			return;
	}
}

char *Info_FindLargestKey( char *s, int maxsize )
{
	char	key[256];
	char	value[256];
	char	*o;
	static char largest_key[256];
	int     largest_size = 0;

	*largest_key = 0;

	if (*s == '\\')
		s++;
	while (*s)
	{
		int nCount;
		int size = 0;

		nCount = 0;
		o = key;
		while ( *s && (*s != '\\') && (nCount < sizeof(key)-1) )
		{
			*o++ = *s++;
			nCount++;
		}

		*o = 0;
		size = strlen( key );

		if (!*s)
		{
			return largest_key;
		}

		nCount = 0;
		o = value;
		s++;
		while ( *s && (*s != '\\') && (nCount < sizeof(value)-1) )
		{
			*o++ = *s++;
			nCount++;
		}
		*o = 0;

		if (*s)
			s++;

		size += strlen( value );

		if ( (size > largest_size) )
		{
			largest_size = size;
			strcpy( largest_key, key );
		}
	}

	return largest_key;
}