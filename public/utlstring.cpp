//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: CUtlString implementation, ported from later Source SDK for the
// GMod bmod systems (see utlstring.h for why this is trimmed down).
//=============================================================================

#include "utlstring.h"
#include <stdlib.h>
#include <string.h>

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Either allocates or reallocates memory to the length.
//
// Allocates space for length characters.  It automatically adds space for the
// nul at the end.  Will adjust m_pString and explicitly set the nul at the
// end before returning.
void *CUtlString::AllocMemory( unsigned int length )
{
	void *pMemoryBlock;
	if ( m_pString )
	{
		pMemoryBlock = realloc( m_pString, length + 1 );
	}
	else
	{
		pMemoryBlock = malloc( length + 1 );
	}
	m_pString = (char*)pMemoryBlock;
	m_pString[ length ] = 0;

	return pMemoryBlock;
}

//-----------------------------------------------------------------------------
void CUtlString::SetDirect( const char *pValue, int nChars )
{
	if ( pValue && nChars > 0 )
	{
		if ( pValue == m_pString )
		{
			AssertMsg( nChars == Q_strlen(m_pString), "CUtlString::SetDirect does not support resizing strings in place." );
			return; // Do nothing. Realloc in AllocMemory might move pValue's location resulting in a bad memcpy.
		}

		AllocMemory( nChars );
		Q_memcpy( m_pString, pValue, nChars );
	}
	else
	{
		Purge();
	}
}

void CUtlString::Set( const char *pValue )
{
	int length = pValue ? Q_strlen( pValue ) : 0;
	SetDirect( pValue, length );
}

// Sets the length (used to serialize into the buffer)
void CUtlString::SetLength( int nLen )
{
	if ( nLen > 0 )
	{
		AllocMemory( nLen );
	}
	else
	{
		Purge();
	}
}

const char *CUtlString::Get( ) const
{
	if (!m_pString)
	{
		return "";
	}
	return m_pString;
}

char *CUtlString::GetForModify()
{
	if ( !m_pString )
	{
		// In general, we optimise away small mallocs for empty strings
		// but if you ask for the non-const bytes, they must be writable
		// so we can't return "" here, like we do for the const version.
		void *pMemoryBlock = malloc( 1 );
		m_pString = (char *)pMemoryBlock;
		*m_pString = 0;
	}

	return m_pString;
}

char CUtlString::operator[]( int i ) const
{
	if ( !m_pString )
		return '\0';

	if ( i >= Length() )
	{
		return '\0';
	}

	return m_pString[i];
}

void CUtlString::Clear()
{
	Purge();
}

void CUtlString::Purge()
{
	free( m_pString );
	m_pString = NULL;
}

bool CUtlString::IsEqual_CaseSensitive( const char *src ) const
{
	if ( !src )
	{
		return (Length() == 0);
	}
	return ( Q_strcmp( Get(), src ) == 0 );
}

bool CUtlString::IsEqual_CaseInsensitive( const char *src ) const
{
	if ( !src )
	{
		return (Length() == 0);
	}
	return ( Q_stricmp( Get(), src ) == 0 );
}

void CUtlString::ToLower()
{
	if ( !m_pString )
	{
		return;
	}

	Q_strlower( m_pString );
}

void CUtlString::ToUpper()
{
	if ( !m_pString )
	{
		return;
	}

	Q__strupr( m_pString );
}

CUtlString &CUtlString::operator=( const CUtlString &src )
{
	SetDirect( src.Get(), src.Length() );
	return *this;
}

CUtlString &CUtlString::operator=( const char *src )
{
	Set( src );
	return *this;
}

bool CUtlString::operator==( const CUtlString &src ) const
{
	if ( IsEmpty() )
	{
		return src.IsEmpty();
	}
	else
	{
		if ( src.IsEmpty() )
		{
			return false;
		}

		return Q_strcmp( m_pString, src.m_pString ) == 0;
	}
}

CUtlString &CUtlString::operator+=( const CUtlString &rhs )
{
	const int lhsLength( Length() );
	const int rhsLength( rhs.Length() );

	if (!rhsLength)
	{
		return *this;
	}

	const int requestedLength( lhsLength + rhsLength );

	AllocMemory( requestedLength );
	Q_memcpy( m_pString + lhsLength, rhs.m_pString, rhsLength );

	return *this;
}

CUtlString &CUtlString::operator+=( const char *rhs )
{
	const int lhsLength( Length() );
	const int rhsLength( Q_strlen( rhs ) );
	const int requestedLength( lhsLength + rhsLength );

	if (!requestedLength)
	{
		return *this;
	}

	AllocMemory( requestedLength );
	Q_memcpy( m_pString + lhsLength, rhs, rhsLength );

	return *this;
}

CUtlString &CUtlString::operator+=( char c )
{
	const int lhsLength( Length() );

	AllocMemory( lhsLength + 1 );
	m_pString[ lhsLength ] = c;

	return *this;
}

CUtlString &CUtlString::operator+=( int rhs )
{
	Assert( sizeof( rhs ) == 4 );

	char tmpBuf[ 12 ];	// Sufficient for a signed 32 bit integer [ -2147483648 to +2147483647 ]
	Q_snprintf( tmpBuf, sizeof( tmpBuf ), "%d", rhs );
	tmpBuf[ sizeof( tmpBuf ) - 1 ] = '\0';

	return operator+=( tmpBuf );
}

bool CUtlString::MatchesPattern( const CUtlString &Pattern, int nFlags ) const
{
	const char *pszSource = String();
	const char *pszPattern = Pattern.String();
	bool	bExact = true;

	while( 1 )
	{
		if ( ( *pszPattern ) == 0 )
		{
			return ( (*pszSource ) == 0 );
		}

		if ( ( *pszPattern ) == '*' )
		{
			pszPattern++;

			if ( ( *pszPattern ) == 0 )
			{
				return true;
			}

			bExact = false;
			continue;
		}

		int nLength = 0;

		while( ( *pszPattern ) != '*' && ( *pszPattern ) != 0 )
		{
			nLength++;
			pszPattern++;
		}

		while( 1 )
		{
			const char *pszStartPattern = pszPattern - nLength;
			const char *pszSearch = pszSource;

			for( int i = 0; i < nLength; i++, pszSearch++, pszStartPattern++ )
			{
				if ( ( *pszSearch ) == 0 )
				{
					return false;
				}

				if ( ( *pszSearch ) != ( *pszStartPattern ) )
				{
					break;
				}
			}

			if ( pszSearch - pszSource == nLength )
			{
				break;
			}

			if ( bExact == true )
			{
				return false;
			}

			if ( ( nFlags & PATTERN_DIRECTORY ) != 0 )
			{
				if ( ( *pszPattern ) != '/' && ( *pszSource ) == '/' )
				{
					return false;
				}
			}

			pszSource++;
		}

		pszSource += nLength;
	}
}

int CUtlString::Format( const char *pFormat, ... )
{
	va_list marker;

	va_start( marker, pFormat );
	int len = FormatV( pFormat, marker );
	va_end( marker );

	return len;
}

//--------------------------------------------------------------------------------------------------
// This can be called from functions that take varargs.
//--------------------------------------------------------------------------------------------------
int CUtlString::FormatV( const char *pFormat, va_list marker )
{
	char tmpBuf[ 4096 ];
	int len = Q_vsnprintf( tmpBuf, sizeof( tmpBuf ), pFormat, marker );
	Set( tmpBuf );
	return len;
}

CUtlString CUtlString::operator+( const char *pOther ) const
{
	CUtlString s = *this;
	s += pOther;
	return s;
}

CUtlString CUtlString::operator+( const CUtlString &other ) const
{
	CUtlString s = *this;
	s += other;
	return s;
}

//-----------------------------------------------------------------------------
// Purpose: concatenate the provided string to our current content
//-----------------------------------------------------------------------------
void CUtlString::Append( const char *pchAddition )
{
	(*this) += pchAddition;
}

void CUtlString::Append( const char *pchAddition, int nChars )
{
	int srcLen = Q_strlen( pchAddition );
	if ( nChars > srcLen )
	{
		nChars = srcLen;
	}

	const int lhsLength( Length() );
	const int requestedLength( lhsLength + nChars );

	AllocMemory( requestedLength );
	Q_memcpy( GetForModify() + lhsLength, pchAddition, nChars );
	m_pString[ requestedLength ] = '\0';
}

// Shared static empty string.
const CUtlString &CUtlString::GetEmptyString()
{
	static const CUtlString s_emptyString;

	return s_emptyString;
}
