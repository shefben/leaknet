#if 0
    ps_1_1

#line 7 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\UnlitGeneric_DetailNoTexture.psh"
    tex t3
    mul_x2 r0.xyz, v0, t3
  + mov r0.w, v0.w

// approximately 2 instruction slots used (1 texture, 1 arithmetic)
#endif

static unsigned int pixelShader_UnlitGeneric_DetailNoTexture_0[] =
{
    0xffff0101, 0x003bfffe, 0x47554244, 0x00000028, 0x000000b4, 0x00000000, 
    0x00000001, 0x00000098, 0x00000003, 0x0000009c, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x696c6e55, 0x6e654774, 0x63697265, 0x7465445f, 
    0x4e6c6961, 0x7865546f, 0x65727574, 0x6873702e, 0xababab00, 0x00000028, 
    0x00000007, 0x000000f4, 0x00000009, 0x000000fc, 0x0000000a, 0x0000010c, 
    0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 0x20395844, 0x64616853, 
    0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 
    0xab003635, 0x00000042, 0xb00f0003, 0x00000005, 0x81070000, 0x90e40000, 
    0xb0e40003, 0x40000001, 0x80080000, 0x90ff0000, 0x0000ffff
};
static PrecompiledShaderByteCode_t UnlitGeneric_DetailNoTexture_pixel_shaders[1] = 
{
	{ pixelShader_UnlitGeneric_DetailNoTexture_0, sizeof( pixelShader_UnlitGeneric_DetailNoTexture_0 ) },
};
struct UnlitGeneric_DetailNoTexturePixelShader_t : public PrecompiledShader_t
{
	UnlitGeneric_DetailNoTexturePixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = UnlitGeneric_DetailNoTexture_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "UnlitGeneric_DetailNoTexture";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static UnlitGeneric_DetailNoTexturePixelShader_t UnlitGeneric_DetailNoTexture_PixelShaderInstance;
