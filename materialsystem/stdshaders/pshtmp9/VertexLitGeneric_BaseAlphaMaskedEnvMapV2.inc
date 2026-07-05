#if 0
    ps_1_1

#line 12 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\VertexLitGeneric_BaseAlphaMaskedEnvMapV2.psh"
    tex t0
    tex t1
    tex t2

#line 19
    mul r0, t0, c3
    mul r1, t1, 1-t2.w
    mul r0.xyz, v0, r0
    mul_x2 r0.xyz, c0, r0
    mad r0.xyz, r1, c2, r0

// approximately 8 instruction slots used (3 texture, 5 arithmetic)
#endif

static unsigned int pixelShader_VertexLitGeneric_BaseAlphaMaskedEnvMapV2_0[] =
{
    0xffff0101, 0x0048fffe, 0x47554244, 0x00000028, 0x000000e8, 0x00000000, 
    0x00000001, 0x000000a4, 0x00000008, 0x000000a8, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74726556, 0x694c7865, 0x6e654774, 0x63697265, 
    0x7361425f, 0x706c4165, 0x614d6168, 0x64656b73, 0x4d766e45, 0x32567061, 
    0x6873702e, 0xababab00, 0x00000028, 0x0000000c, 0x00000128, 0x0000000d, 
    0x00000130, 0x0000000e, 0x00000138, 0x00000013, 0x00000140, 0x00000014, 
    0x00000150, 0x00000015, 0x00000160, 0x00000016, 0x00000170, 0x00000017, 
    0x00000180, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 0x20395844, 
    0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 
    0x36322e30, 0xab003635, 0x00000042, 0xb00f0000, 0x00000042, 0xb00f0001, 
    0x00000042, 0xb00f0002, 0x00000005, 0x800f0000, 0xb0e40000, 0xa0e40003, 
    0x00000005, 0x800f0001, 0xb0e40001, 0xb6ff0002, 0x00000005, 0x80070000, 
    0x90e40000, 0x80e40000, 0x00000005, 0x81070000, 0xa0e40000, 0x80e40000, 
    0x00000004, 0x80070000, 0x80e40001, 0xa0e40002, 0x80e40000, 0x0000ffff
};
#if 0
    ps_1_1

#line 12 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\VertexLitGeneric_BaseAlphaMaskedEnvMapV2.psh"
    tex t0
    tex t1
    tex t2
    texkill t3

#line 19
    mul r0, t0, c3
    mul r1, t1, 1-t2.w
    mul r0.xyz, v0, r0
    mul_x2 r0.xyz, c0, r0
    mad r0.xyz, r1, c2, r0

// approximately 9 instruction slots used (3 texture, 6 arithmetic)
#endif

static unsigned int pixelShader_VertexLitGeneric_BaseAlphaMaskedEnvMapV2_1[] =
{
    0xffff0101, 0x004afffe, 0x47554244, 0x00000028, 0x000000f0, 0x00000000, 
    0x00000001, 0x000000a4, 0x00000009, 0x000000a8, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74726556, 0x694c7865, 0x6e654774, 0x63697265, 
    0x7361425f, 0x706c4165, 0x614d6168, 0x64656b73, 0x4d766e45, 0x32567061, 
    0x6873702e, 0xababab00, 0x00000028, 0x0000000c, 0x00000130, 0x0000000d, 
    0x00000138, 0x0000000e, 0x00000140, 0x00000010, 0x00000148, 0x00000013, 
    0x00000150, 0x00000014, 0x00000160, 0x00000015, 0x00000170, 0x00000016, 
    0x00000180, 0x00000017, 0x00000190, 0x7263694d, 0x666f736f, 0x52282074, 
    0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 
    0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 0xb00f0000, 
    0x00000042, 0xb00f0001, 0x00000042, 0xb00f0002, 0x00000041, 0xb00f0003, 
    0x00000005, 0x800f0000, 0xb0e40000, 0xa0e40003, 0x00000005, 0x800f0001, 
    0xb0e40001, 0xb6ff0002, 0x00000005, 0x80070000, 0x90e40000, 0x80e40000, 
    0x00000005, 0x81070000, 0xa0e40000, 0x80e40000, 0x00000004, 0x80070000, 
    0x80e40001, 0xa0e40002, 0x80e40000, 0x0000ffff
};
static PrecompiledShaderByteCode_t VertexLitGeneric_BaseAlphaMaskedEnvMapV2_pixel_shaders[2] = 
{
	{ pixelShader_VertexLitGeneric_BaseAlphaMaskedEnvMapV2_0, sizeof( pixelShader_VertexLitGeneric_BaseAlphaMaskedEnvMapV2_0 ) },
	{ pixelShader_VertexLitGeneric_BaseAlphaMaskedEnvMapV2_1, sizeof( pixelShader_VertexLitGeneric_BaseAlphaMaskedEnvMapV2_1 ) },
};
struct VertexLitGeneric_BaseAlphaMaskedEnvMapV2PixelShader_t : public PrecompiledShader_t
{
	VertexLitGeneric_BaseAlphaMaskedEnvMapV2PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = VertexLitGeneric_BaseAlphaMaskedEnvMapV2_pixel_shaders;
		m_nShaderCount = 2;
		m_pName = "VertexLitGeneric_BaseAlphaMaskedEnvMapV2";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static VertexLitGeneric_BaseAlphaMaskedEnvMapV2PixelShader_t VertexLitGeneric_BaseAlphaMaskedEnvMapV2_PixelShaderInstance;
