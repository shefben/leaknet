// Client-side initialization helpers for BarrysMod on the 2003 engine.
#include "cbase.h"
#include "gmod_message.h"
#include "gmod_postprocess.h"

// Auto game system to bootstrap client-only subsystems (no client Lua yet).
class CGModClientInit : public CAutoGameSystem
{
public:
	CGModClientInit() : CAutoGameSystem( "CGModClientInit" ) {}

	virtual bool Init()
	{
		InitGModMessageSystem();
        GModPostProcess_Init();
		return true;
	}

	virtual void Shutdown()
	{
		ShutdownGModMessageSystem();
        GModPostProcess_Shutdown();
	}
};

static CGModClientInit g_GModClientInit;
