//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef NETWORKSTRINGTABLE_GAMEDLL_H
#define NETWORKSTRINGTABLE_GAMEDLL_H
#ifdef _WIN32
#pragma once
#endif

#include "inetworkstringtableserver.h"
#include "isaverestore.h"



extern INetworkStringTableServer *networkstringtable;

// String tables used by the game DLL
#define MAX_VGUI_SCREEN_STRING_BITS		8
#define MAX_VGUI_SCREEN_STRINGS			( 1 << MAX_VGUI_SCREEN_STRING_BITS )
#define VGUI_SCREEN_INVALID_STRING		( MAX_VGUI_SCREEN_STRINGS - 1 )

#define MAX_MATERIAL_STRING_BITS		10
#define MAX_MATERIAL_STRINGS			( 1 << MAX_MATERIAL_STRING_BITS )
#define OVERLAY_MATERIAL_INVALID_STRING	( MAX_MATERIAL_STRINGS - 1 )

extern TABLEID g_StringTableVguiScreen;


extern TABLEID g_StringTableEffectDispatch;



class CStringTableSaveRestoreOps : public CDefSaveRestoreOps
{
public:
	void Init( TABLEID pNetworkStringTableID)
	{
		m_hStringTable = pNetworkStringTableID;
	}

	// save data type interface
	virtual void Save(const SaveRestoreFieldInfo_t &fieldInfo, ISave *pSave)
	{
		int *pStringIndex = (int *)fieldInfo.pField;
	//	const char *pString = m_hStringTable->GetString(g_StringTableVguiScreen, *pStringIndex);
		const char *pString = networkstringtable->GetString(m_hStringTable, *pStringIndex);
		int nLen = Q_strlen(pString) + 1;
		pSave->WriteInt(&nLen);
		pSave->WriteString(pString);
	}

	virtual void Restore(const SaveRestoreFieldInfo_t &fieldInfo, IRestore *pRestore)
	{
		int *pStringIndex = (int *)fieldInfo.pField;
		int nLen = pRestore->ReadInt();
		char *pTemp = (char *)stackalloc(nLen);
		pRestore->ReadString(pTemp, nLen, nLen);
	//	*pStringIndex = m_hStringTable->AddString(g_StringTableVguiScreen, pTemp);
		*pStringIndex = networkstringtable->AddString(m_hStringTable, pTemp);
	}

	virtual void MakeEmpty(const SaveRestoreFieldInfo_t &fieldInfo)
	{
		int *pStringIndex = (int *)fieldInfo.pField;
		*pStringIndex = INVALID_STRING_INDEX;
	}

	virtual bool IsEmpty(const SaveRestoreFieldInfo_t &fieldInfo)
	{
		int *pStringIndex = (int *)fieldInfo.pField;
		return *pStringIndex == INVALID_STRING_INDEX;
	}

private:
	TABLEID m_hStringTable;
};



// save/load
extern CStringTableSaveRestoreOps g_VguiScreenStringOps;


#endif // NETWORKSTRINGTABLE_GAMEDLL_H
