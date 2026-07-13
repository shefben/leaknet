//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: Simple string class, ported from later Source SDK (tier1/utlstring.h)
// for use by the GMod bmod systems. Trimmed to the subset that has working
// dependencies in this 2003-era engine (no V_* path helpers here, only Q_*).
//=============================================================================

#ifndef UTLSTRING_H
#define UTLSTRING_H

#ifdef _WIN32
#pragma once
#endif

#include "vstdlib/strtools.h"
#include "tier0/platform.h"

// Matched with the memdbgoff at end of header
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Simple string class.
// NOTE: This is *not* optimal! Use in tools, but not runtime-hot code.
//-----------------------------------------------------------------------------
class CUtlString
{
public:
	typedef enum
	{
		PATTERN_NONE		= 0x00000000,
		PATTERN_DIRECTORY	= 0x00000001
	} TUtlStringPattern;

public:
	CUtlString();
	CUtlString( const char *pString );
	CUtlString( const char *pString, int length );
	CUtlString( const CUtlString& string );
	~CUtlString();

	const char	*Get( ) const;
	void		Set( const char *pValue );
	operator const char*() const;

	// Set directly and don't look for a null terminator in pValue.
	// nChars does not include the nul and this will only copy
	// at most nChars (even if pValue is longer).
	void		SetDirect( const char *pValue, int nChars );

	// for compatibility switching items from UtlSymbol
	const char  *String() const { return Get(); }

	// Returns strlen
	int			Length() const;
	// IsEmpty() is more efficient than Length() == 0
	bool		IsEmpty() const;

	// Sets the length (used to serialize into the buffer)
	void		SetLength( int nLen );
	char		*GetForModify();
	void		Clear();
	void		Purge();

	// Case Change
	void		ToLower();
	void		ToUpper();
	void		Append( const char *pAddition, int nChars );
	void		Append( const char *pchAddition );
	void		Append( const char chAddition ) { char temp[2] = { chAddition, 0 }; Append( temp ); }

	bool		IsEqual_CaseSensitive( const char *src ) const;
	bool		IsEqual_CaseInsensitive( const char *src ) const;

	CUtlString &operator=( const CUtlString &src );
	CUtlString &operator=( const char *src );

	// Test for equality
	bool operator==( const CUtlString &src ) const;
	bool operator!=( const CUtlString &src ) const { return !operator==( src ); }

	CUtlString &operator+=( const CUtlString &rhs );
	CUtlString &operator+=( const char *rhs );
	CUtlString &operator+=( char c );
	CUtlString &operator+=( int rhs );

	CUtlString operator+( const char *pOther ) const;
	CUtlString operator+( const CUtlString &other ) const;

	bool MatchesPattern( const CUtlString &Pattern, int nFlags = 0 ) const;		// case SENSITIVE, use * for wildcard in pattern string

	char operator[]( int i ) const;

	int Format( PRINTF_FORMAT_STRING const char *pFormat, ... ) FMTFUNCTION( 2, 3 );
	int FormatV( PRINTF_FORMAT_STRING const char *pFormat, va_list marker );

	// Defining AltArgumentType_t hints that associative container classes should
	// also implement Find/Insert/Remove functions that take const char* params.
	typedef const char *AltArgumentType_t;

	// These can be used for utlvector sorts.
	static int __cdecl SortCaseInsensitive( const CUtlString *pString1, const CUtlString *pString2 );
	static int __cdecl SortCaseSensitive( const CUtlString *pString1, const CUtlString *pString2 );

	// Empty string for those times when you need to return an empty string and
	// either don't want to pay the construction cost, or are returning a
	// const CUtlString& and cannot just return "".
	static const CUtlString &GetEmptyString();

private:
	// INTERNALS
	// AllocMemory allocates enough space for length characters plus a terminating zero.
	void *AllocMemory( unsigned int length );

	// If m_pString is not NULL, it points to the start of the string, and the memory allocation.
	char *m_pString;
};

inline bool operator==( const char *pString, const CUtlString &utlString )
{
	return utlString.IsEqual_CaseSensitive( pString );
}

inline bool operator!=( const char *pString, const CUtlString &utlString )
{
	return !utlString.IsEqual_CaseSensitive( pString );
}

inline bool operator==( const CUtlString &utlString, const char *pString )
{
	return utlString.IsEqual_CaseSensitive( pString );
}

inline bool operator!=( const CUtlString &utlString, const char *pString )
{
	return !utlString.IsEqual_CaseSensitive( pString );
}

//-----------------------------------------------------------------------------
// Inline methods
//-----------------------------------------------------------------------------
inline CUtlString::CUtlString()
: m_pString( NULL )
{
}

inline CUtlString::CUtlString( const char *pString )
: m_pString( NULL )
{
	Set( pString );
}

inline CUtlString::CUtlString( const char *pString, int length )
: m_pString( NULL )
{
	SetDirect( pString, length );
}

inline CUtlString::CUtlString( const CUtlString& string )
: m_pString( NULL )
{
	Set( string.Get() );
}

inline CUtlString::~CUtlString()
{
	Purge();
}

inline int CUtlString::Length() const
{
	if (m_pString)
	{
		return Q_strlen( m_pString );
	}
	return 0;
}

inline bool CUtlString::IsEmpty() const
{
	return !m_pString || m_pString[0] == 0;
}

inline int __cdecl CUtlString::SortCaseInsensitive( const CUtlString *pString1, const CUtlString *pString2 )
{
	return Q_stricmp( pString1->String(), pString2->String() );
}

inline int __cdecl CUtlString::SortCaseSensitive( const CUtlString *pString1, const CUtlString *pString2 )
{
	return Q_strcmp( pString1->String(), pString2->String() );
}

// Converts to c-strings
inline CUtlString::operator const char*() const
{
	return Get();
}

// Helper function for CUtlMaps/CUtlDict with a CUtlString key
inline bool UtlStringLessFunc( const CUtlString &lhs, const CUtlString &rhs ) { return Q_strcmp( lhs.Get(), rhs.Get() ) < 0; }
inline bool UtlStringCaseInsensitiveLessFunc( const CUtlString &lhs, const CUtlString &rhs ) { return Q_stricmp( lhs.Get(), rhs.Get() ) < 0; }

#include "tier0/memdbgoff.h"

#endif // UTLSTRING_H
