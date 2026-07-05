#if 0
    ps_1_1

#line 12 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\VertexLitTexture_Overbright2.psh"
    tex t0
    mul_x2 r0.xyz, t0, v0
  + mul r0.w, t0, v0

// approximately 2 instruction slots used (1 texture, 1 arithmetic)
#endif

static unsigned int pixelShader_VertexLitTexture_Overbright2_0[] =
{
    0xffff0101, 0x003bfffe, 0x47554244, 0x00000028, 0x000000b4, 0x00000000, 
    0x00000001, 0x00000098, 0x00000003, 0x0000009c, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74726556, 0x694c7865, 0x78655474, 0x65727574, 
    0x65764f5f, 0x69726272, 0x32746867, 0x6873702e, 0xababab00, 0x00000028, 
    0x0000000c, 0x000000f4, 0x0000000e, 0x000000fc, 0x0000000f, 0x0000010c, 
    0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 0x20395844, 0x64616853, 
    0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 
    0xab003635, 0x00000042, 0xb00f0000, 0x00000005, 0x81070000, 0xb0e40000, 
    0x90e40000, 0x40000005, 0x80080000, 0xb0e40000, 0x90e40000, 0x0000ffff
};
static PrecompiledShaderByteCode_t VertexLitTexture_Overbright2_pixel_shaders[1] = 
{
	{ pixelShader_VertexLitTexture_Overbright2_0, sizeof( pixelShader_VertexLitTexture_Overbright2_0 ) },
};
struct VertexLitTexture_Overbright2PixelShader_t : public PrecompiledShader_t
{
	VertexLitTexture_Overbright2PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = VertexLitTexture_Overbright2_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "VertexLitTexture_Overbright2";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static VertexLitTexture_Overbright2PixelShader_t VertexLitTexture_Overbright2_PixelShaderInstance;
