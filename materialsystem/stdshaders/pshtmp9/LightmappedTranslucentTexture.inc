#if 0
    ps_1_1

#line 14 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\LightmappedTranslucentTexture.psh"
    tex t0
    tex t1
    mul r0.xyz, t0, t1
    mul_x2 r0.xyz, c0, r0
    mov r0.w, t0.w

// approximately 5 instruction slots used (2 texture, 3 arithmetic)
#endif

static unsigned int pixelShader_LightmappedTranslucentTexture_0[] =
{
    0xffff0101, 0x003ffffe, 0x47554244, 0x00000028, 0x000000c4, 0x00000000, 
    0x00000001, 0x00000098, 0x00000005, 0x0000009c, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x6867694c, 0x70616d74, 0x54646570, 0x736e6172, 
    0x6563756c, 0x6554746e, 0x72757478, 0x73702e65, 0xabab0068, 0x00000028, 
    0x0000000e, 0x00000104, 0x0000000f, 0x0000010c, 0x00000010, 0x00000114, 
    0x00000011, 0x00000124, 0x00000013, 0x00000134, 0x7263694d, 0x666f736f, 
    0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 
    0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 
    0xb00f0000, 0x00000042, 0xb00f0001, 0x00000005, 0x80070000, 0xb0e40000, 
    0xb0e40001, 0x00000005, 0x81070000, 0xa0e40000, 0x80e40000, 0x00000001, 
    0x80080000, 0xb0ff0000, 0x0000ffff
};
static PrecompiledShaderByteCode_t LightmappedTranslucentTexture_pixel_shaders[1] = 
{
	{ pixelShader_LightmappedTranslucentTexture_0, sizeof( pixelShader_LightmappedTranslucentTexture_0 ) },
};
struct LightmappedTranslucentTexturePixelShader_t : public PrecompiledShader_t
{
	LightmappedTranslucentTexturePixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = LightmappedTranslucentTexture_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "LightmappedTranslucentTexture";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static LightmappedTranslucentTexturePixelShader_t LightmappedTranslucentTexture_PixelShaderInstance;
