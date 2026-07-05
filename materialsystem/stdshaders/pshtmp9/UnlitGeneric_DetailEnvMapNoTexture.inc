#if 0
    ps_1_1

#line 12 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\UnlitGeneric_DetailEnvMapNoTexture.psh"
    tex t1
    tex t3

#line 17
    mul r0, v0, t1
    mul r0.xyz, r0, c2
    mul_x2 r0.xyz, r0, t3

// approximately 5 instruction slots used (2 texture, 3 arithmetic)
#endif

static unsigned int pixelShader_UnlitGeneric_DetailEnvMapNoTexture_0[] =
{
    0xffff0101, 0x0040fffe, 0x47554244, 0x00000028, 0x000000c8, 0x00000000, 
    0x00000001, 0x0000009c, 0x00000005, 0x000000a0, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x696c6e55, 0x6e654774, 0x63697265, 0x7465445f, 
    0x456c6961, 0x614d766e, 0x546f4e70, 0x75747865, 0x702e6572, 0xab006873, 
    0x00000028, 0x0000000c, 0x00000108, 0x0000000d, 0x00000110, 0x00000011, 
    0x00000118, 0x00000012, 0x00000128, 0x00000013, 0x00000138, 0x7263694d, 
    0x666f736f, 0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 
    0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 
    0x00000042, 0xb00f0001, 0x00000042, 0xb00f0003, 0x00000005, 0x800f0000, 
    0x90e40000, 0xb0e40001, 0x00000005, 0x80070000, 0x80e40000, 0xa0e40002, 
    0x00000005, 0x81070000, 0x80e40000, 0xb0e40003, 0x0000ffff
};
static PrecompiledShaderByteCode_t UnlitGeneric_DetailEnvMapNoTexture_pixel_shaders[1] = 
{
	{ pixelShader_UnlitGeneric_DetailEnvMapNoTexture_0, sizeof( pixelShader_UnlitGeneric_DetailEnvMapNoTexture_0 ) },
};
struct UnlitGeneric_DetailEnvMapNoTexturePixelShader_t : public PrecompiledShader_t
{
	UnlitGeneric_DetailEnvMapNoTexturePixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = UnlitGeneric_DetailEnvMapNoTexture_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "UnlitGeneric_DetailEnvMapNoTexture";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static UnlitGeneric_DetailEnvMapNoTexturePixelShader_t UnlitGeneric_DetailEnvMapNoTexture_PixelShaderInstance;
