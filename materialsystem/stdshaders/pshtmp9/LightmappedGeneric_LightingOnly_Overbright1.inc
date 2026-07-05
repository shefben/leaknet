#if 0
    ps_1_1

#line 3 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\LightmappedGeneric_LightingOnly_Overbright1.psh"
    tex t1
    mov_d2 r0, t1

// approximately 2 instruction slots used (1 texture, 1 arithmetic)
#endif

static unsigned int pixelShader_LightmappedGeneric_LightingOnly_Overbright1_0[] =
{
    0xffff0101, 0x003cfffe, 0x47554244, 0x00000028, 0x000000b8, 0x00000000, 
    0x00000001, 0x000000a4, 0x00000002, 0x000000a8, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x6867694c, 0x70616d74, 0x47646570, 0x72656e65, 
    0x4c5f6369, 0x74686769, 0x4f676e69, 0x5f796c6e, 0x7265764f, 0x67697262, 
    0x2e317468, 0x00687370, 0x00000028, 0x00000003, 0x000000f8, 0x00000005, 
    0x00000100, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 0x20395844, 
    0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 
    0x36322e30, 0xab003635, 0x00000042, 0xb00f0001, 0x00000001, 0x8f0f0000, 
    0xb0e40001, 0x0000ffff
};
static PrecompiledShaderByteCode_t LightmappedGeneric_LightingOnly_Overbright1_pixel_shaders[1] = 
{
	{ pixelShader_LightmappedGeneric_LightingOnly_Overbright1_0, sizeof( pixelShader_LightmappedGeneric_LightingOnly_Overbright1_0 ) },
};
struct LightmappedGeneric_LightingOnly_Overbright1PixelShader_t : public PrecompiledShader_t
{
	LightmappedGeneric_LightingOnly_Overbright1PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = LightmappedGeneric_LightingOnly_Overbright1_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "LightmappedGeneric_LightingOnly_Overbright1";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static LightmappedGeneric_LightingOnly_Overbright1PixelShader_t LightmappedGeneric_LightingOnly_Overbright1_PixelShaderInstance;
