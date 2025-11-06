//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: A base class for all material proxies in the client dll
//
// $NoKeywords: $
//=============================================================================

#ifndef PROXY_ENTITY_H
#define PROXY_ENTITY_H

#include "materialsystem/imaterialproxy.h"
#include "materialsystem/imaterialvar.h"


class IMaterialVar;

//-----------------------------------------------------------------------------
// Base class all material proxies should inherit from
//-----------------------------------------------------------------------------
class CEntityMaterialProxy : public IMaterialProxy
{
public:
	virtual void Release( void );
	void OnBind( void *pC_BaseEntity );

protected:
	// base classes should implement this
	virtual void OnBind( C_BaseEntity *pBaseEntity ) = 0;
};


#endif // PROXY_ENTITY_H

