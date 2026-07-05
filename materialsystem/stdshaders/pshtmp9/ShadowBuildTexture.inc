#if 0
    ps_1_1

#line 7 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\ShadowBuildTexture.psh"
    def c0, 1, 1, 1, 1
    tex t0
    mov r0.xyz, c0
  + mul r0.w, v0.w, t0.w

// approximately 2 instruction slots used (1 texture, 1 arithmetic)
#endif

static unsigned int pixelShader_ShadowBuildTexture_0[] =
{
    0xffff0101, 0x003afffe, 0x47554244, 0x00000028, 0x000000b0, 0x00000000, 
    0x00000001, 0x0000008c, 0x00000004, 0x00000090, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x64616853, 0x7542776f, 0x54646c69, 0x75747865, 
    0x702e6572, 0xab006873, 0x00000028, 0x00000007, 0x000000f0, 0x00000009, 
    0x00000108, 0x0000000b, 0x00000110, 0x0000000c, 0x0000011c, 0x7263694d, 
    0x666f736f, 0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 
    0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 
    0x00000051, 0xa00f0000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 
    0x00000042, 0xb00f0000, 0x00000001, 0x80070000, 0xa0e40000, 0x40000005, 
    0x80080000, 0x90ff0000, 0xb0ff0000, 0x0000ffff
};
static PrecompiledShaderByteCode_t ShadowBuildTexture_pixel_shaders[1] = 
{
	{ pixelShader_ShadowBuildTexture_0, sizeof( pixelShader_ShadowBuildTexture_0 ) },
};
struct ShadowBuildTexturePixelShader_t : public PrecompiledShader_t
{
	ShadowBuildTexturePixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = ShadowBuildTexture_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "ShadowBuildTexture";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static ShadowBuildTexturePixelShader_t ShadowBuildTexture_PixelShaderInstance;
