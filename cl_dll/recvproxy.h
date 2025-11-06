//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: implements various common send proxies
//
// $NoKeywords: $
//=============================================================================

#ifndef RECVPROXY_H
#define RECVPROXY_H


#include "dt_recv.h"

class CRecvProxyData;


// This converts the int stored in pData to an EHANDLE in pOut.
void RecvProxy_IntToEHandle( const CRecvProxyData *pData, void *pStruct, void *pOut );

void RecvProxy_IntToMoveParent( const CRecvProxyData *pData, void *pStruct, void *pOut );
void RecvProxy_IntToColor32( const CRecvProxyData *pData, void *pStruct, void *pOut );

RecvProp RecvPropTime(
	const char *pVarName,
	int offset, 
	int sizeofVar=SIZEOF_IGNORE );

RecvProp RecvPropPredictableId(
	const char *pVarName,
	int offset, 
	int sizeofVar=SIZEOF_IGNORE );

RecvProp RecvPropEHandle(
	const char *pVarName,
	int offset, 
	int sizeofVar=SIZEOF_IGNORE,
	RecvVarProxyFn proxyFn=RecvProxy_IntToEHandle );

RecvProp RecvPropBool(
	const char *pVarName,
	int offset, 
	int sizeofVar );

void RecvProxy_IntToEHandle( 
	const CRecvProxyData *pData, 
	void *pStruct, 
	void *pOut );

#endif // RECVPROXY_H

