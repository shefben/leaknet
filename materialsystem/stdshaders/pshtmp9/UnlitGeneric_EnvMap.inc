#if 0
    ps_1_1

#line 12 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\UnlitGeneric_EnvMap.psh"
    tex t0
    tex t1
    mul r1, c2, t1
    mad r0.xyz, t0, v0, r1
  + mul r0.w, t0, v0

// approximately 4 instruction slots used (2 texture, 2 arithmetic)
#endif

static unsigned int pixelShader_UnlitGeneric_EnvMap_0[] =
{
    0xffff0101, 0x003cfffe, 0x47554244, 0x00000028, 0x000000b8, 0x00000000, 
    0x00000001, 0x0000008c, 0x00000005, 0x00000090, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x696c6e55, 0x6e654774, 0x63697265, 0x766e455f, 
    0x2e70614d, 0x00687370, 0x00000028, 0x0000000c, 0x000000f8, 0x0000000d, 
    0x00000100, 0x0000000f, 0x00000108, 0x00000010, 0x00000118, 0x00000011, 
    0x0000012c, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 0x20395844, 
    0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 
    0x36322e30, 0xab003635, 0x00000042, 0xb00f0000, 0x00000042, 0xb00f0001, 
    0x00000005, 0x800f0001, 0xa0e40002, 0xb0e40001, 0x00000004, 0x80070000, 
    0xb0e40000, 0x90e40000, 0x80e40001, 0x40000005, 0x80080000, 0xb0e40000, 
    0x90e40000, 0x0000ffff
};
static PrecompiledShaderByteCode_t UnlitGeneric_EnvMap_pixel_shaders[1] = 
{
	{ pixelShader_UnlitGeneric_EnvMap_0, sizeof( pixelShader_UnlitGeneric_EnvMap_0 ) },
};
struct UnlitGeneric_EnvMapPixelShader_t : public PrecompiledShader_t
{
	UnlitGeneric_EnvMapPixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = UnlitGeneric_EnvMap_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "UnlitGeneric_EnvMap";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static UnlitGeneric_EnvMapPixelShader_t UnlitGeneric_EnvMap_PixelShaderInstance;
