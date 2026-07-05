#if 0
    ps_1_1

#line 4 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\VertexLitGeneric_DiffBumpTimesBase_Overbright1_Translucent.psh"
    tex t0

#line 7
    tex t1

#line 10
    texcoord t2

#line 13
    texcoord t3

#line 19
    dp3_sat r1, t1_bx2, c0
    mul r0.xyz, r1, t2
  + mul r0.w, t0, c3
    dp3_sat r1, t1_bx2, c1
    mad r0.xyz, r1, t3, r0
    dp3_sat r1, t1_bx2, c2
    mad r0.xyz, r1, v0, r0
    mul r0.xyz, r0, c3

#line 30
    mul r0.xyz, r0, t0

// approximately 12 instruction slots used (4 texture, 8 arithmetic)
#endif

static unsigned int pixelShader_VertexLitGeneric_DiffBumpTimesBase_Overbright1_Translucent_0[] =
{
    0xffff0101, 0x0056fffe, 0x47554244, 0x00000028, 0x00000120, 0x00000000, 
    0x00000001, 0x000000b4, 0x0000000d, 0x000000b8, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74726556, 0x694c7865, 0x6e654774, 0x63697265, 
    0x6669445f, 0x6d754266, 0x6d695470, 0x61427365, 0x4f5f6573, 0x62726576, 
    0x68676972, 0x545f3174, 0x736e6172, 0x6563756c, 0x702e746e, 0xab006873, 
    0x00000028, 0x00000004, 0x00000160, 0x00000007, 0x00000168, 0x0000000a, 
    0x00000170, 0x0000000d, 0x00000178, 0x00000013, 0x00000180, 0x00000014, 
    0x00000190, 0x00000015, 0x000001a0, 0x00000017, 0x000001b0, 0x00000018, 
    0x000001c0, 0x0000001a, 0x000001d4, 0x0000001b, 0x000001e4, 0x0000001d, 
    0x000001f8, 0x0000001e, 0x00000208, 0x7263694d, 0x666f736f, 0x52282074, 
    0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 
    0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 0xb00f0000, 
    0x00000042, 0xb00f0001, 0x00000040, 0xb00f0002, 0x00000040, 0xb00f0003, 
    0x00000008, 0x801f0001, 0xb4e40001, 0xa0e40000, 0x00000005, 0x80070000, 
    0x80e40001, 0xb0e40002, 0x40000005, 0x80080000, 0xb0e40000, 0xa0e40003, 
    0x00000008, 0x801f0001, 0xb4e40001, 0xa0e40001, 0x00000004, 0x80070000, 
    0x80e40001, 0xb0e40003, 0x80e40000, 0x00000008, 0x801f0001, 0xb4e40001, 
    0xa0e40002, 0x00000004, 0x80070000, 0x80e40001, 0x90e40000, 0x80e40000, 
    0x00000005, 0x80070000, 0x80e40000, 0xa0e40003, 0x00000005, 0x80070000, 
    0x80e40000, 0xb0e40000, 0x0000ffff
};
static PrecompiledShaderByteCode_t VertexLitGeneric_DiffBumpTimesBase_Overbright1_Translucent_pixel_shaders[1] = 
{
	{ pixelShader_VertexLitGeneric_DiffBumpTimesBase_Overbright1_Translucent_0, sizeof( pixelShader_VertexLitGeneric_DiffBumpTimesBase_Overbright1_Translucent_0 ) },
};
struct VertexLitGeneric_DiffBumpTimesBase_Overbright1_TranslucentPixelShader_t : public PrecompiledShader_t
{
	VertexLitGeneric_DiffBumpTimesBase_Overbright1_TranslucentPixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = VertexLitGeneric_DiffBumpTimesBase_Overbright1_Translucent_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "VertexLitGeneric_DiffBumpTimesBase_Overbright1_Translucent";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static VertexLitGeneric_DiffBumpTimesBase_Overbright1_TranslucentPixelShader_t VertexLitGeneric_DiffBumpTimesBase_Overbright1_Translucent_PixelShaderInstance;
