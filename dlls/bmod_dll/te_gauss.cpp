#include "cbase.h"
#include "te_effect_dispatch.h"

// Full implementation for the Gauss explosion temp entity.
void TE_GaussExplosion(IRecipientFilter& filter, float delay, const Vector& pos, const Vector& dir, int type)
{
	CEffectData data;
	data.m_vOrigin = pos;
	data.m_vNormal = dir;
	data.m_nHitBox = type;

	DispatchEffect("GaussExplosion", data);
}
