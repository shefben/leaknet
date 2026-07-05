#if 0
    ps_1_1

#line 3 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\WorldVertexTransition_Micros_EditMode.psh"
    tex t0
    tex t1
    tex t2
    tex t3

#line 9
    mov_sat r0.xyz, t3
    mov_sat r0.w, 1-v0
    lrp r1, r0.w, t1, t0
    mul r0.xyz, r0, r1
    mov r1.w, t3.z
    lrp r1.xyz, r0.w, r1.w, t3.w
    mul_x2 r0.xyz, r0, r1
    mul_x4 r0.xyz, c0, r0

// approximately 12 instruction slots used (4 texture, 8 arithmetic)
#endif

static unsigned int pixelShader_WorldVertexTransition_Micros_EditMode_0[] =
{
    0xffff0101, 0x004ffffe, 0x47554244, 0x00000028, 0x00000104, 0x00000000, 
    0x00000001, 0x000000a0, 0x0000000c, 0x000000a4, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x6c726f57, 0x72655664, 0x54786574, 0x736e6172, 
    0x6f697469, 0x694d5f6e, 0x736f7263, 0x6964455f, 0x646f4d74, 0x73702e65, 
    0xabab0068, 0x00000028, 0x00000003, 0x00000144, 0x00000004, 0x0000014c, 
    0x00000005, 0x00000154, 0x00000006, 0x0000015c, 0x00000009, 0x00000164, 
    0x0000000a, 0x00000170, 0x0000000c, 0x0000017c, 0x0000000d, 0x00000190, 
    0x0000000f, 0x000001a0, 0x00000010, 0x000001ac, 0x00000011, 0x000001c0, 
    0x00000013, 0x000001d0, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 
    0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 
    0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 0xb00f0000, 0x00000042, 
    0xb00f0001, 0x00000042, 0xb00f0002, 0x00000042, 0xb00f0003, 0x00000001, 
    0x80170000, 0xb0e40003, 0x00000001, 0x80180000, 0x96e40000, 0x00000012, 
    0x800f0001, 0x80ff0000, 0xb0e40001, 0xb0e40000, 0x00000005, 0x80070000, 
    0x80e40000, 0x80e40001, 0x00000001, 0x80080001, 0xb0aa0003, 0x00000012, 
    0x80070001, 0x80ff0000, 0x80ff0001, 0xb0ff0003, 0x00000005, 0x81070000, 
    0x80e40000, 0x80e40001, 0x00000005, 0x82070000, 0xa0e40000, 0x80e40000, 
    0x0000ffff
};
static PrecompiledShaderByteCode_t WorldVertexTransition_Micros_EditMode_pixel_shaders[1] = 
{
	{ pixelShader_WorldVertexTransition_Micros_EditMode_0, sizeof( pixelShader_WorldVertexTransition_Micros_EditMode_0 ) },
};
struct WorldVertexTransition_Micros_EditModePixelShader_t : public PrecompiledShader_t
{
	WorldVertexTransition_Micros_EditModePixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = WorldVertexTransition_Micros_EditMode_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "WorldVertexTransition_Micros_EditMode";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static WorldVertexTransition_Micros_EditModePixelShader_t WorldVertexTransition_Micros_EditMode_PixelShaderInstance;
