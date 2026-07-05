#if 0
    ps_1_1

#line 3 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\WorldVertexAlpha.psh"
    tex t0
    tex t1

#line 9
    mov r0.w, 1-t1.w

#line 12
    mul r0.xyz, t0, t1
    mul_x2 r0.xyz, c0, r0

// approximately 5 instruction slots used (2 texture, 3 arithmetic)
#endif

static unsigned int pixelShader_WorldVertexAlpha_0[] =
{
    0xffff0101, 0x003cfffe, 0x47554244, 0x00000028, 0x000000b8, 0x00000000, 
    0x00000001, 0x0000008c, 0x00000005, 0x00000090, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x6c726f57, 0x72655664, 0x41786574, 0x6168706c, 
    0x6873702e, 0xababab00, 0x00000028, 0x00000003, 0x000000f8, 0x00000004, 
    0x00000100, 0x00000009, 0x00000108, 0x0000000c, 0x00000114, 0x0000000d, 
    0x00000124, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 0x20395844, 
    0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 
    0x36322e30, 0xab003635, 0x00000042, 0xb00f0000, 0x00000042, 0xb00f0001, 
    0x00000001, 0x80080000, 0xb6ff0001, 0x00000005, 0x80070000, 0xb0e40000, 
    0xb0e40001, 0x00000005, 0x81070000, 0xa0e40000, 0x80e40000, 0x0000ffff
};
static PrecompiledShaderByteCode_t WorldVertexAlpha_pixel_shaders[1] = 
{
	{ pixelShader_WorldVertexAlpha_0, sizeof( pixelShader_WorldVertexAlpha_0 ) },
};
struct WorldVertexAlphaPixelShader_t : public PrecompiledShader_t
{
	WorldVertexAlphaPixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = WorldVertexAlpha_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "WorldVertexAlpha";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static WorldVertexAlphaPixelShader_t WorldVertexAlpha_PixelShaderInstance;
