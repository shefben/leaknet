//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#include "cbase.h"
#include "proxyentity.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"

#include <cmath>

// VXP: Based on this: https://gist.github.com/kuathadianto/200148f53616cbd226d993b400214a7f
void HueToRGB(float frac, Vector& color)
{
	// VXP: Just pretend that saturation and value are 1.0f
	float S, V;
	S = V = 1.0f;

	float C = S * V;
	float X = C * (1 - abs(fmod(frac / 60.0, 2) - 1));
	float m = V - C;
	float Rs, Gs, Bs;

	if(frac >= 0 && frac < 60) {
		Rs = C;
		Gs = X;
		Bs = 0;	
	}
	else if(frac >= 60 && frac < 120) {	
		Rs = X;
		Gs = C;
		Bs = 0;	
	}
	else if(frac >= 120 && frac < 180) {
		Rs = 0;
		Gs = C;
		Bs = X;	
	}
	else if(frac >= 180 && frac < 240) {
		Rs = 0;
		Gs = X;
		Bs = C;	
	}
	else if(frac >= 240 && frac < 300) {
		Rs = X;
		Gs = 0;
		Bs = C;	
	}
	else {
		Rs = C;
		Gs = 0;
		Bs = X;	
	}

	color[0] = (Rs + m) * 255;
	color[1] = (Gs + m) * 255;
	color[2] = (Bs + m) * 255;
}

// $ThermalVar : name of variable to run Thermal wave on (can either be a color or a float)
// $ThermalPeriod: time that it takes to go through whole Thermal wave in seconds (default: 1.0f)
// $ThermalMax : the max value for the Thermal wave (default: 1.0f )
// $ThermalMin: the min value for the Thermal wave  (default: 0.0f )
class CThermalMaterialProxy : public CEntityMaterialProxy
{
public:
	CThermalMaterialProxy();
	virtual ~CThermalMaterialProxy();
	virtual bool Init( IMaterial *pMaterial, KeyValues* pKeyValues );
	virtual void OnBind( C_BaseEntity *pEntity );

private:
	IMaterialVar *m_ThermalVar;
	IMaterialVar *m_ThermalPeriod;
	IMaterialVar *m_ThermalMax;
	IMaterialVar *m_ThermalMin;
};

CThermalMaterialProxy::CThermalMaterialProxy()
{
	m_ThermalVar = NULL;
	m_ThermalPeriod = NULL;
	m_ThermalMax = NULL;
	m_ThermalMin = NULL;
}

CThermalMaterialProxy::~CThermalMaterialProxy()
{
}


bool CThermalMaterialProxy::Init( IMaterial *pMaterial, KeyValues* pKeyValues  )
{
	bool foundVar;

	m_ThermalVar = pMaterial->FindVar( "$color", &foundVar, false );
	if( !foundVar )
	{
		m_ThermalVar = NULL;
		return false;
	}

	m_ThermalPeriod = pMaterial->FindVar( "$ThermalPeriod", &foundVar, false );
	if( !foundVar )
	{
		m_ThermalPeriod = NULL;
	}

	m_ThermalMax = pMaterial->FindVar( "$ThermalMax", &foundVar, false );
	if( !foundVar )
	{
		m_ThermalMax = NULL;
	}

	m_ThermalMin = pMaterial->FindVar( "$ThermalMin", &foundVar, false );
	if( !foundVar )
	{
		m_ThermalMin = NULL;
	}
	return true;
}

void CThermalMaterialProxy::OnBind( C_BaseEntity *pEntity )
{
// FIXME, enable this later
return;

	if( !m_ThermalVar )
	{
		return;
	}

	float min, max, period, value;

	// set default values if these variables don't exist.
	min		= m_ThermalMin		? m_ThermalMin->GetFloatValue()	: 0.0f;
	max		= m_ThermalMax		? m_ThermalMax->GetFloatValue()	: 1.0f;
	period	= m_ThermalPeriod	? m_ThermalPeriod->GetFloatValue() : 1.0f;
	
	// get a value in [0,1]
	value = ( sin( 2.0f * M_PI * gpGlobals->curtime / period ) * 0.5f ) + 0.5f;
	// get a value in [min,max]	
	value = ( max - min ) * value + min;
	
	Vector color;
	HueToRGB( 360.f * value, color );

	m_ThermalVar->SetVecValue( color[0], color[1], color[2] );
}

EXPOSE_INTERFACE( CThermalMaterialProxy, IMaterialProxy, "Thermal" IMATERIAL_PROXY_INTERFACE_VERSION );
