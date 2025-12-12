// Copyright (c) BarrysMod.
// Minimal Derma skin + factory manager for client-side UI controls.
#pragma once

#include <vgui/VGUI.h>
#include <vgui_controls/Panel.h>
#include <utlvector.h>
#include <utlsymbol.h>

class KeyValues;

// Simple skin container loaded from resource/skins/*.res
class CDermaSkin
{
public:
	CDermaSkin();
	~CDermaSkin();

	bool            LoadFromFile( const char *pszFilename );
	const char*     GetName() const { return m_szName; }

	// Convenience lookups
	Color           GetColor( const char *pszPath, const Color &fallback ) const;
	const char*     GetFont( const char *pszPath ) const;

private:
	char            m_szName[64];
	KeyValues       *m_pKV;
};

typedef vgui::Panel* (*DermaCreateFn)( vgui::Panel *pParent, const char *pName );

// Registry for Derma controls + skins
class CDermaManager
{
public:
	static CDermaManager& Get();

	bool        Init( vgui::VPANEL parent );
	void        Shutdown();
	void        ReloadSkins();

	// Factory
	void        RegisterControl( const char *pszClassName, DermaCreateFn fn );
	vgui::Panel*CreatePanel( const char *pszClassName, vgui::Panel *pParent, const char *pName );

	// Skinning
	CDermaSkin* GetSkin( const char *pszSkinName );
	CDermaSkin* GetDefaultSkin();
	void        ApplySkinToPanel( vgui::Panel *pPanel, const char *pszSkin );

	// Root overlay to paint Derma tree
	void        AttachRoot( vgui::VPANEL parent );
	void        DetachRoot();
	vgui::Panel*GetRootPanel() { return m_pRootPanel; }

private:
	struct ControlFactory_t
	{
		DermaCreateFn	fn;
		CUtlSymbol		sClass;
	};

	struct SkinEntry_t
	{
		CDermaSkin *pSkin;
		CUtlSymbol sName;
	};

	CDermaManager();
	~CDermaManager();

	CDermaManager( const CDermaManager& ) = delete;
	CDermaManager& operator=( const CDermaManager& ) = delete;

	vgui::Panel			    *m_pRootPanel;
	CUtlVector<ControlFactory_t> m_ControlFactories;
	CUtlVector<SkinEntry_t>      m_Skins;
	char                        m_szDefaultSkin[64];
};

// Global helper accessors
CDermaSkin* Derma_GetSkin( const char *pszSkinName );
vgui::Panel*Derma_Create( const char *pszClass, vgui::Panel *pParent, const char *pName );
void        Derma_ApplySkin( vgui::Panel *pPanel, const char *pszSkin );
