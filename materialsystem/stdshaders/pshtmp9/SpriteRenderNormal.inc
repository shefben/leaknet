#if 0
    ps_1_1

#line 3 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\SpriteRenderNormal.psh"
    tex t0
    mov r0, t0

// approximately 2 instruction slots used (1 texture, 1 arithmetic)
#endif

static unsigned int pixelShader_SpriteRenderNormal_0[] =
{
    0xffff0101, 0x0036fffe, 0x47554244, 0x00000028, 0x000000a0, 0x00000000, 
    0x00000001, 0x0000008c, 0x00000002, 0x00000090, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x69727053, 0x65526574, 0x7265646e, 0x6d726f4e, 
    0x702e6c61, 0xab006873, 0x00000028, 0x00000003, 0x000000e0, 0x00000005, 
    0x000000e8, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 0x20395844, 
    0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 
    0x36322e30, 0xab003635, 0x00000042, 0xb00f0000, 0x00000001, 0x800f0000, 
    0xb0e40000, 0x0000ffff
};
static PrecompiledShaderByteCode_t SpriteRenderNormal_pixel_shaders[1] = 
{
	{ pixelShader_SpriteRenderNormal_0, sizeof( pixelShader_SpriteRenderNormal_0 ) },
};
struct SpriteRenderNormalPixelShader_t : public PrecompiledShader_t
{
	SpriteRenderNormalPixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = SpriteRenderNormal_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "SpriteRenderNormal";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static SpriteRenderNormalPixelShader_t SpriteRenderNormal_PixelShaderInstance;
