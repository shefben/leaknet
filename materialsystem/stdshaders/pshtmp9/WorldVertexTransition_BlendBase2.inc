#if 0
    ps_1_1

#line 11 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\WorldVertexTransition_BlendBase2.psh"
    tex t0
    tex t1
    mul r0.xyz, t1, t0
  + mov r0.w, 1-t1
    mul_x2 r0.xyz, c0, r0

// approximately 4 instruction slots used (2 texture, 2 arithmetic)
#endif

static unsigned int pixelShader_WorldVertexTransition_BlendBase2_0[] =
{
    0xffff0101, 0x0040fffe, 0x47554244, 0x00000028, 0x000000c8, 0x00000000, 
    0x00000001, 0x0000009c, 0x00000005, 0x000000a0, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x6c726f57, 0x72655664, 0x54786574, 0x736e6172, 
    0x6f697469, 0x6c425f6e, 0x42646e65, 0x32657361, 0x6873702e, 0xababab00, 
    0x00000028, 0x0000000b, 0x00000108, 0x0000000c, 0x00000110, 0x0000000d, 
    0x00000118, 0x0000000e, 0x00000128, 0x0000000f, 0x00000134, 0x7263694d, 
    0x666f736f, 0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 
    0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 
    0x00000042, 0xb00f0000, 0x00000042, 0xb00f0001, 0x00000005, 0x80070000, 
    0xb0e40001, 0xb0e40000, 0x40000001, 0x80080000, 0xb6e40001, 0x00000005, 
    0x81070000, 0xa0e40000, 0x80e40000, 0x0000ffff
};
static PrecompiledShaderByteCode_t WorldVertexTransition_BlendBase2_pixel_shaders[1] = 
{
	{ pixelShader_WorldVertexTransition_BlendBase2_0, sizeof( pixelShader_WorldVertexTransition_BlendBase2_0 ) },
};
struct WorldVertexTransition_BlendBase2PixelShader_t : public PrecompiledShader_t
{
	WorldVertexTransition_BlendBase2PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = WorldVertexTransition_BlendBase2_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "WorldVertexTransition_BlendBase2";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static WorldVertexTransition_BlendBase2PixelShader_t WorldVertexTransition_BlendBase2_PixelShaderInstance;
