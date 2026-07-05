#if 0
    ps_1_1

#line 12 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\UnlitGeneric_DetailBaseAlphaMaskedEnvMap.psh"
    tex t0
    tex t1
    tex t2
    tex t3

#line 25
    mul r0, t0, v0
    mul_x2 r0.xyz, r0, t3
    mul r1, t1, 1-t2.w
    mul r1, c2, r1
    add r0.xyz, r0, r1

// approximately 9 instruction slots used (4 texture, 5 arithmetic)
#endif

static unsigned int pixelShader_UnlitGeneric_DetailBaseAlphaMaskedEnvMap_0[] =
{
    0xffff0101, 0x004afffe, 0x47554244, 0x00000028, 0x000000f0, 0x00000000, 
    0x00000001, 0x000000a4, 0x00000009, 0x000000a8, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x696c6e55, 0x6e654774, 0x63697265, 0x7465445f, 
    0x426c6961, 0x41657361, 0x6168706c, 0x6b73614d, 0x6e456465, 0x70614d76, 
    0x6873702e, 0xababab00, 0x00000028, 0x0000000c, 0x00000130, 0x0000000d, 
    0x00000138, 0x0000000e, 0x00000140, 0x0000000f, 0x00000148, 0x00000019, 
    0x00000150, 0x0000001a, 0x00000160, 0x0000001b, 0x00000170, 0x0000001c, 
    0x00000180, 0x0000001d, 0x00000190, 0x7263694d, 0x666f736f, 0x52282074, 
    0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 
    0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 0xb00f0000, 
    0x00000042, 0xb00f0001, 0x00000042, 0xb00f0002, 0x00000042, 0xb00f0003, 
    0x00000005, 0x800f0000, 0xb0e40000, 0x90e40000, 0x00000005, 0x81070000, 
    0x80e40000, 0xb0e40003, 0x00000005, 0x800f0001, 0xb0e40001, 0xb6ff0002, 
    0x00000005, 0x800f0001, 0xa0e40002, 0x80e40001, 0x00000002, 0x80070000, 
    0x80e40000, 0x80e40001, 0x0000ffff
};
static PrecompiledShaderByteCode_t UnlitGeneric_DetailBaseAlphaMaskedEnvMap_pixel_shaders[1] = 
{
	{ pixelShader_UnlitGeneric_DetailBaseAlphaMaskedEnvMap_0, sizeof( pixelShader_UnlitGeneric_DetailBaseAlphaMaskedEnvMap_0 ) },
};
struct UnlitGeneric_DetailBaseAlphaMaskedEnvMapPixelShader_t : public PrecompiledShader_t
{
	UnlitGeneric_DetailBaseAlphaMaskedEnvMapPixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = UnlitGeneric_DetailBaseAlphaMaskedEnvMap_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "UnlitGeneric_DetailBaseAlphaMaskedEnvMap";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static UnlitGeneric_DetailBaseAlphaMaskedEnvMapPixelShader_t UnlitGeneric_DetailBaseAlphaMaskedEnvMap_PixelShaderInstance;
