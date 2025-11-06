//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================

#ifndef BUMPVECTS_H
#define BUMPVECTS_H
#pragma once

#include "mathlib.h"

// VXP: Taken from Source SDK Depot 2
#define OO_SQRT_2			0.70710676908493042f
#define OO_SQRT_3			0.57735025882720947f
#define OO_SQRT_6			0.40824821591377258f
#define OO_SQRT_2_OVER_3	0.81649661064147949f // sqrt( 2 / 3 )

#define NUM_BUMP_VECTS 3

const Vector g_localBumpBasis[NUM_BUMP_VECTS] = {
	Vector(	 OO_SQRT_2_OVER_3,	 0.0f,			OO_SQRT_3 ),
	Vector(	-OO_SQRT_6,			 OO_SQRT_2,		OO_SQRT_3 ),
	Vector(	-OO_SQRT_6,			-OO_SQRT_2,		OO_SQRT_3 )
};

void GetBumpNormals( const Vector& sVect, const Vector& tVect, const Vector& flatNormal, 
					 const Vector& phongNormal, Vector bumpNormals[NUM_BUMP_VECTS] );

#endif // BUMPVECTS_H
