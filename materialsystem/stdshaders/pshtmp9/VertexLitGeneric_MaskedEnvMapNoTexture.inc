#if 0
    ps_1_1

#line 12 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\VertexLitGeneric_MaskedEnvMapNoTexture.psh"
    tex t1
    tex t2

#line 18
    mul r1, t1, t2
    mul r0.xyz, r1, c2
    mul r0.xyz, v0, r0
    mul_x2 r0.xyz, c0, r0
  + mul r0.w, c3.w, t2.w

// approximately 6 instruction slots used (2 texture, 4 arithmetic)
#endif

static unsigned int pixelShader_VertexLitGeneric_MaskedEnvMapNoTexture_0[] =
{
    0xffff0101, 0x0045fffe, 0x47554244, 0x00000028, 0x000000dc, 0x00000000, 
    0x00000001, 0x000000a0, 0x00000007, 0x000000a4, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74726556, 0x694c7865, 0x6e654774, 0x63697265, 
    0x73614d5f, 0x4564656b, 0x614d766e, 0x546f4e70, 0x75747865, 0x702e6572, 
    0xab006873, 0x00000028, 0x0000000c, 0x0000011c, 0x0000000d, 0x00000124, 
    0x00000012, 0x0000012c, 0x00000013, 0x0000013c, 0x00000014, 0x0000014c, 
    0x00000015, 0x0000015c, 0x00000016, 0x0000016c, 0x7263694d, 0x666f736f, 
    0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 
    0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 
    0xb00f0001, 0x00000042, 0xb00f0002, 0x00000005, 0x800f0001, 0xb0e40001, 
    0xb0e40002, 0x00000005, 0x80070000, 0x80e40001, 0xa0e40002, 0x00000005, 
    0x80070000, 0x90e40000, 0x80e40000, 0x00000005, 0x81070000, 0xa0e40000, 
    0x80e40000, 0x40000005, 0x80080000, 0xa0ff0003, 0xb0ff0002, 0x0000ffff
};
#if 0
    ps_1_1

#line 12 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\VertexLitGeneric_MaskedEnvMapNoTexture.psh"
    tex t1
    tex t2
    texkill t3

#line 18
    mul r1, t1, t2
    mul r0.xyz, r1, c2
    mul r0.xyz, v0, r0
    mul_x2 r0.xyz, c0, r0
  + mul r0.w, c3.w, t2.w

// approximately 7 instruction slots used (2 texture, 5 arithmetic)
#endif

static unsigned int pixelShader_VertexLitGeneric_MaskedEnvMapNoTexture_1[] =
{
    0xffff0101, 0x0047fffe, 0x47554244, 0x00000028, 0x000000e4, 0x00000000, 
    0x00000001, 0x000000a0, 0x00000008, 0x000000a4, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74726556, 0x694c7865, 0x6e654774, 0x63697265, 
    0x73614d5f, 0x4564656b, 0x614d766e, 0x546f4e70, 0x75747865, 0x702e6572, 
    0xab006873, 0x00000028, 0x0000000c, 0x00000124, 0x0000000d, 0x0000012c, 
    0x0000000f, 0x00000134, 0x00000012, 0x0000013c, 0x00000013, 0x0000014c, 
    0x00000014, 0x0000015c, 0x00000015, 0x0000016c, 0x00000016, 0x0000017c, 
    0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 0x20395844, 0x64616853, 
    0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 
    0xab003635, 0x00000042, 0xb00f0001, 0x00000042, 0xb00f0002, 0x00000041, 
    0xb00f0003, 0x00000005, 0x800f0001, 0xb0e40001, 0xb0e40002, 0x00000005, 
    0x80070000, 0x80e40001, 0xa0e40002, 0x00000005, 0x80070000, 0x90e40000, 
    0x80e40000, 0x00000005, 0x81070000, 0xa0e40000, 0x80e40000, 0x40000005, 
    0x80080000, 0xa0ff0003, 0xb0ff0002, 0x0000ffff
};
static PrecompiledShaderByteCode_t VertexLitGeneric_MaskedEnvMapNoTexture_pixel_shaders[2] = 
{
	{ pixelShader_VertexLitGeneric_MaskedEnvMapNoTexture_0, sizeof( pixelShader_VertexLitGeneric_MaskedEnvMapNoTexture_0 ) },
	{ pixelShader_VertexLitGeneric_MaskedEnvMapNoTexture_1, sizeof( pixelShader_VertexLitGeneric_MaskedEnvMapNoTexture_1 ) },
};
struct VertexLitGeneric_MaskedEnvMapNoTexturePixelShader_t : public PrecompiledShader_t
{
	VertexLitGeneric_MaskedEnvMapNoTexturePixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = VertexLitGeneric_MaskedEnvMapNoTexture_pixel_shaders;
		m_nShaderCount = 2;
		m_pName = "VertexLitGeneric_MaskedEnvMapNoTexture";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static VertexLitGeneric_MaskedEnvMapNoTexturePixelShader_t VertexLitGeneric_MaskedEnvMapNoTexture_PixelShaderInstance;
