#if 0
    ps_1_1

#line 3 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\WorldVertexTransition.psh"
    tex t0
    tex t1
    tex t2
    mov_sat r1.w, 1-t2.w
    lrp r0, r1.w, t1, t0
    mul r0, r0, t2
    mul_x2 r0.xyz, c0, r0

// approximately 7 instruction slots used (3 texture, 4 arithmetic)
#endif

static unsigned int pixelShader_WorldVertexTransition_0[] =
{
    0xffff0101, 0x0041fffe, 0x47554244, 0x00000028, 0x000000cc, 0x00000000, 
    0x00000001, 0x00000090, 0x00000007, 0x00000094, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x6c726f57, 0x72655664, 0x54786574, 0x736e6172, 
    0x6f697469, 0x73702e6e, 0xabab0068, 0x00000028, 0x00000003, 0x0000010c, 
    0x00000004, 0x00000114, 0x00000005, 0x0000011c, 0x00000007, 0x00000124, 
    0x00000008, 0x00000130, 0x00000009, 0x00000144, 0x0000000a, 0x00000154, 
    0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 0x20395844, 0x64616853, 
    0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 
    0xab003635, 0x00000042, 0xb00f0000, 0x00000042, 0xb00f0001, 0x00000042, 
    0xb00f0002, 0x00000001, 0x80180001, 0xb6ff0002, 0x00000012, 0x800f0000, 
    0x80ff0001, 0xb0e40001, 0xb0e40000, 0x00000005, 0x800f0000, 0x80e40000, 
    0xb0e40002, 0x00000005, 0x81070000, 0xa0e40000, 0x80e40000, 0x0000ffff
};
static PrecompiledShaderByteCode_t WorldVertexTransition_pixel_shaders[1] = 
{
	{ pixelShader_WorldVertexTransition_0, sizeof( pixelShader_WorldVertexTransition_0 ) },
};
struct WorldVertexTransitionPixelShader_t : public PrecompiledShader_t
{
	WorldVertexTransitionPixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = WorldVertexTransition_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "WorldVertexTransition";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static WorldVertexTransitionPixelShader_t WorldVertexTransition_PixelShaderInstance;
