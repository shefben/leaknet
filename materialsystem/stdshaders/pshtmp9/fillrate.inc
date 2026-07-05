#if 0
    ps_1_1

#line 11 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\fillrate.psh"
    mov_d2 r0, c0

// approximately 1 instruction slot used
#endif

static unsigned int pixelShader_fillrate_0[] =
{
    0xffff0101, 0x0032fffe, 0x47554244, 0x00000028, 0x00000090, 0x00000000, 
    0x00000001, 0x00000084, 0x00000001, 0x00000088, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x6c6c6966, 0x65746172, 0x6873702e, 0xababab00, 
    0x00000028, 0x0000000b, 0x000000d0, 0x7263694d, 0x666f736f, 0x52282074, 
    0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 
    0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000001, 0x8f0f0000, 
    0xa0e40000, 0x0000ffff
};
static PrecompiledShaderByteCode_t fillrate_pixel_shaders[1] = 
{
	{ pixelShader_fillrate_0, sizeof( pixelShader_fillrate_0 ) },
};
struct fillratePixelShader_t : public PrecompiledShader_t
{
	fillratePixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = fillrate_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "fillrate";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static fillratePixelShader_t fillrate_PixelShaderInstance;
