// Garry's Mod style post-process helpers (overlay + motion blur + overlay material)
#pragma once

#ifdef BMOD_CLIENT_DLL
#include "materialsystem/imaterial.h"

class CViewRender;

// Initialize / shutdown material handles.
void GModPostProcess_Init();
void GModPostProcess_Shutdown();

// Update the current screen overlay based on gmod-style ConVars.
void GModPostProcess_Update(CViewRender &view);
#else
// Non-BMod builds keep empty stubs.
inline void GModPostProcess_Init() {}
inline void GModPostProcess_Shutdown() {}
class CViewRender;
inline void GModPostProcess_Update(CViewRender &) {}
#endif
