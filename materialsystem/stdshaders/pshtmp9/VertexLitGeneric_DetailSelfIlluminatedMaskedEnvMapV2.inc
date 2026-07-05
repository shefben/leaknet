#if 0
    ps_1_1

#line 12 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\VertexLitGeneric_DetailSelfIlluminatedMaskedEnvMapV2.psh"
    tex t0
    tex t1
    tex t2
    tex t3
    mul r0.xyz, t0, c3
  + mul r0.w, c3.w, t2.w
    mul r0.xyz, v0, r0
    mul_x2 r0.xyz, c0, r0

#line 23
    mul_x2 r0.xyz, r0, t3
    mul r1, t0, c1
    lrp r0.xyz, t0.w, r1, r0
    mul r1, t2, t1
    mad r0.xyz, r1, c2, r0

// approximately 12 instruction slots used (4 texture, 8 arithmetic)
#endif

static unsigned int pixelShader_VertexLitGeneric_DetailSelfIlluminatedMaskedEnvMapV2_0[] =
{
    0xffff0101, 0x0055fffe, 0x47554244, 0x00000028, 0x0000011c, 0x00000000, 
    0x00000001, 0x000000b0, 0x0000000d, 0x000000b4, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74726556, 0x694c7865, 0x6e654774, 0x63697265, 
    0x7465445f, 0x536c6961, 0x49666c65, 0x6d756c6c, 0x74616e69, 0x614d6465, 
    0x64656b73, 0x4d766e45, 0x32567061, 0x6873702e, 0xababab00, 0x00000028, 
    0x0000000c, 0x0000015c, 0x0000000d, 0x00000164, 0x0000000e, 0x0000016c, 
    0x0000000f, 0x00000174, 0x00000011, 0x0000017c, 0x00000012, 0x0000018c, 
    0x00000014, 0x0000019c, 0x00000015, 0x000001ac, 0x00000017, 0x000001bc, 
    0x00000019, 0x000001cc, 0x0000001a, 0x000001dc, 0x0000001c, 0x000001f0, 
    0x0000001d, 0x00000200, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 
    0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 
    0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 0xb00f0000, 0x00000042, 
    0xb00f0001, 0x00000042, 0xb00f0002, 0x00000042, 0xb00f0003, 0x00000005, 
    0x80070000, 0xb0e40000, 0xa0e40003, 0x40000005, 0x80080000, 0xa0ff0003, 
    0xb0ff0002, 0x00000005, 0x80070000, 0x90e40000, 0x80e40000, 0x00000005, 
    0x81070000, 0xa0e40000, 0x80e40000, 0x00000005, 0x81070000, 0x80e40000, 
    0xb0e40003, 0x00000005, 0x800f0001, 0xb0e40000, 0xa0e40001, 0x00000012, 
    0x80070000, 0xb0ff0000, 0x80e40001, 0x80e40000, 0x00000005, 0x800f0001, 
    0xb0e40002, 0xb0e40001, 0x00000004, 0x80070000, 0x80e40001, 0xa0e40002, 
    0x80e40000, 0x0000ffff
};
static PrecompiledShaderByteCode_t VertexLitGeneric_DetailSelfIlluminatedMaskedEnvMapV2_pixel_shaders[1] = 
{
	{ pixelShader_VertexLitGeneric_DetailSelfIlluminatedMaskedEnvMapV2_0, sizeof( pixelShader_VertexLitGeneric_DetailSelfIlluminatedMaskedEnvMapV2_0 ) },
};
struct VertexLitGeneric_DetailSelfIlluminatedMaskedEnvMapV2PixelShader_t : public PrecompiledShader_t
{
	VertexLitGeneric_DetailSelfIlluminatedMaskedEnvMapV2PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = VertexLitGeneric_DetailSelfIlluminatedMaskedEnvMapV2_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "VertexLitGeneric_DetailSelfIlluminatedMaskedEnvMapV2";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static VertexLitGeneric_DetailSelfIlluminatedMaskedEnvMapV2PixelShader_t VertexLitGeneric_DetailSelfIlluminatedMaskedEnvMapV2_PixelShaderInstance;
