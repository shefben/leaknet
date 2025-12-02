//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: User message parsing utilities (2007 protocol compatible)
//          Provides READ_* functions that work with bf_read buffers
//
//=============================================================================//

#include "cbase.h"
#include "parsemsg.h"

// Global buffer pointer - points to current message being read
static bf_read *g_pReadBuf = NULL;
// Fallback buffer for legacy BEGIN_READ(void*, int) calls
static bf_read g_LegacyBuf;

//-----------------------------------------------------------------------------
// Purpose: Initialize reading from a bf_read buffer (2007 protocol)
//-----------------------------------------------------------------------------
void BEGIN_READ( bf_read &buf )
{
	g_pReadBuf = &buf;
}

//-----------------------------------------------------------------------------
// Purpose: Initialize reading from a void* buffer (legacy compatibility)
//-----------------------------------------------------------------------------
void BEGIN_READ( void *buf, int size )
{
	g_LegacyBuf.StartReading( buf, size );
	g_pReadBuf = &g_LegacyBuf;
}

int READ_CHAR( void )
{
	return g_pReadBuf ? g_pReadBuf->ReadChar() : 0;
}

int READ_BYTE( void )
{
	return g_pReadBuf ? g_pReadBuf->ReadByte() : 0;
}

int READ_SHORT( void )
{
	return g_pReadBuf ? g_pReadBuf->ReadShort() : 0;
}

int READ_WORD( void )
{
	return g_pReadBuf ? g_pReadBuf->ReadWord() : 0;
}

int READ_LONG( void )
{
	return g_pReadBuf ? g_pReadBuf->ReadLong() : 0;
}

float READ_FLOAT( void )
{
	return g_pReadBuf ? g_pReadBuf->ReadFloat() : 0.0f;
}

char* READ_STRING( void )
{
	static char string[2048];
	if ( g_pReadBuf )
	{
		g_pReadBuf->ReadString( string, sizeof(string) );
	}
	else
	{
		string[0] = '\0';
	}
	return string;
}

float READ_COORD( void )
{
	return g_pReadBuf ? g_pReadBuf->ReadBitCoord() : 0.0f;
}

float READ_ANGLE( void )
{
	return (float)(READ_CHAR() * (360.0 / 256));
}

float READ_HIRESANGLE( void )
{
	return (float)(READ_SHORT() * (360.0 / 65536));
}

void READ_VEC3COORD( Vector& vec )
{
	if ( g_pReadBuf )
	{
		g_pReadBuf->ReadBitVec3Coord( vec );
	}
	else
	{
		vec.Init();
	}
}

void READ_VEC3NORMAL( Vector& vec )
{
	if ( g_pReadBuf )
	{
		g_pReadBuf->ReadBitVec3Normal( vec );
	}
	else
	{
		vec.Init();
	}
}

void READ_ANGLES( QAngle &angles )
{
	if ( g_pReadBuf )
	{
		g_pReadBuf->ReadBitAngles( angles );
	}
	else
	{
		angles.Init();
	}
}

bool READ_BOOL( void )
{
	return g_pReadBuf ? (g_pReadBuf->ReadOneBit() != 0) : false;
}

unsigned int READ_UBITLONG( int numbits )
{
	return g_pReadBuf ? g_pReadBuf->ReadUBitLong( numbits ) : 0;
}

int READ_SBITLONG( int numbits )
{
	return g_pReadBuf ? g_pReadBuf->ReadSBitLong( numbits ) : 0;
}

void READ_BITS( void *buffer, int nbits )
{
	if ( g_pReadBuf )
	{
		g_pReadBuf->ReadBits( buffer, nbits );
	}
}
