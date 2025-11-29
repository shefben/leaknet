//========= Copyright 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#include "cbase.h"
#include "vgui_int.h"
#include "ienginevgui.h"
#include "derma_manager.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void VGUI_CreateClientDLLRootPanel( void )
{
    // Attach Derma overlay to the client DLL panel
    vgui::VPANEL root = enginevgui->GetPanel( PANEL_CLIENTDLL );
    CDermaManager::Get().Init( root );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void VGUI_DestroyClientDLLRootPanel( void )
{
    CDermaManager::Get().Shutdown();
}

//-----------------------------------------------------------------------------
// Purpose: Game specific root panel
// Output : vgui::Panel
//-----------------------------------------------------------------------------
vgui::VPANEL VGui_GetClientDLLRootPanel( void )
{
    vgui::VPANEL root = enginevgui->GetPanel( PANEL_CLIENTDLL );
    return root;
}
