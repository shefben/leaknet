//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: User message parsing utilities (2007 protocol compatible)
//          Provides READ_* macros that work with bf_read buffers
//
//=============================================================================//

#if !defined( PARSEMSG_H )
#define PARSEMSG_H
#ifdef _WIN32
#pragma once
#endif

#include "bitbuf.h"

class QAngle;
class Vector;

// Setup functions - use one of these before READ_* functions
void	BEGIN_READ( bf_read &buf );					// 2007 protocol - use bf_read directly
void	BEGIN_READ( void *buf, int size );			// Legacy support - wraps void* in bf_read

// Byte-wise reading functions
int		READ_CHAR( void );
int		READ_BYTE( void );
int		READ_SHORT( void );
int		READ_WORD( void );
int		READ_LONG( void );
float	READ_FLOAT( void );
char	*READ_STRING( void );
float	READ_COORD( void );
float	READ_ANGLE( void );
void	READ_VEC3COORD( Vector& vec );
void	READ_VEC3NORMAL( Vector& vec );
float	READ_HIRESANGLE( void );
void	READ_ANGLES( QAngle &angles );

// Bit-wise reading functions
bool	READ_BOOL( void );
unsigned int READ_UBITLONG( int numbits );
int		READ_SBITLONG( int numbits );
void	READ_BITS( void *buffer, int nbits );

#endif // PARSEMSG_H
