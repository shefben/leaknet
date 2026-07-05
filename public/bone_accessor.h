//========= Copyright (c) 1996-2005, Valve Corporation, All rights reserved. ============//
// Purpose: Thread-safe bone accessor for reading/writing bone transforms
// From 2007 Source Engine for v44+ model support
//=============================================================================//

#ifndef BONE_ACCESSOR_H
#define BONE_ACCESSOR_H
#ifdef _WIN32
#pragma once
#endif

#include "mathlib/mathlib.h"  // For matrix3x4_t - v44+ isolated, no v37 dependencies

class C_BaseAnimating;

//-----------------------------------------------------------------------------
// CBoneAccessor - Thread-safe wrapper for bone matrix access
//
// Provides debug validation of bone read/write access in debug builds
// and allows tracking which bones are readable/writable at any time.
//-----------------------------------------------------------------------------
class CBoneAccessor
{
public:
	CBoneAccessor();
	CBoneAccessor( matrix3x4_t *pBones );  // Access to all bones

#if defined( CLIENT_DLL )
	void Init( const C_BaseAnimating *pAnimating, matrix3x4_t *pBones );
#endif

	// Get readable/writable bone flags
	int GetReadableBones();
	void SetReadableBones( int flags );

	int GetWritableBones();
	void SetWritableBones( int flags );

	// Get bones for read or write access
	const matrix3x4_t&	GetBone( int iBone ) const;
	const matrix3x4_t&	operator[]( int iBone ) const;
	matrix3x4_t&		GetBoneForWrite( int iBone );
	matrix3x4_t*		GetBoneArrayForWrite( ) const;

private:
#if defined( CLIENT_DLL ) && defined( _DEBUG )
	void SanityCheckBone( int iBone, bool bReadable ) const;
#endif

	// Only used in the client DLL for debug verification
	const C_BaseAnimating *m_pAnimating;
	matrix3x4_t *m_pBones;
	int m_ReadableBones;	// Which bones can be read (flag mask)
	int m_WritableBones;	// Which bones can be written (flag mask)
};

// ========== Inline Implementations ==========

inline CBoneAccessor::CBoneAccessor()
{
	m_pAnimating = NULL;
	m_pBones = NULL;
	m_ReadableBones = m_WritableBones = 0;
}

inline CBoneAccessor::CBoneAccessor( matrix3x4_t *pBones )
{
	m_pAnimating = NULL;
	m_pBones = pBones;
	m_ReadableBones = m_WritableBones = 0;
}

#if defined( CLIENT_DLL )
inline void CBoneAccessor::Init( const C_BaseAnimating *pAnimating, matrix3x4_t *pBones )
{
	m_pAnimating = pAnimating;
	m_pBones = pBones;
}
#endif

inline int CBoneAccessor::GetReadableBones()
{
	return m_ReadableBones;
}

inline void CBoneAccessor::SetReadableBones( int flags )
{
	m_ReadableBones = flags;
}

inline int CBoneAccessor::GetWritableBones()
{
	return m_WritableBones;
}

inline void CBoneAccessor::SetWritableBones( int flags )
{
	m_WritableBones = flags;
}

inline const matrix3x4_t& CBoneAccessor::GetBone( int iBone ) const
{
#if defined( CLIENT_DLL ) && defined( _DEBUG )
	SanityCheckBone( iBone, true );
#endif
	return m_pBones[iBone];
}

inline const matrix3x4_t& CBoneAccessor::operator[]( int iBone ) const
{
#if defined( CLIENT_DLL ) && defined( _DEBUG )
	SanityCheckBone( iBone, true );
#endif
	return m_pBones[iBone];
}

inline matrix3x4_t& CBoneAccessor::GetBoneForWrite( int iBone )
{
#if defined( CLIENT_DLL ) && defined( _DEBUG )
	SanityCheckBone( iBone, false );
#endif
	return m_pBones[iBone];
}

inline matrix3x4_t *CBoneAccessor::GetBoneArrayForWrite( void ) const
{
	return m_pBones;
}

#endif // BONE_ACCESSOR_H
