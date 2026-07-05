#if 0
    ps_1_1

#line 13 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\VertexLitGeneric_SelfIlluminated.psh"
    tex t0

#line 19
    mul r0.xyz, t0, c3
  + mov r0.w, c3.w
    mul r0.xyz, v0, r0
    mul_x2 r0.xyz, c0, r0
    mul r1, t0, c1
    lrp r0.xyz, t0.w, r1, r0

// approximately 6 instruction slots used (1 texture, 5 arithmetic)
#endif

static unsigned int pixelShader_VertexLitGeneric_SelfIlluminated_0[] =
{
    0xffff0101, 0x0044fffe, 0x47554244, 0x00000028, 0x000000d8, 0x00000000, 
    0x00000001, 0x0000009c, 0x00000007, 0x000000a0, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74726556, 0x694c7865, 0x6e654774, 0x63697265, 
    0x6c65535f, 0x6c6c4966, 0x6e696d75, 0x64657461, 0x6873702e, 0xababab00, 
    0x00000028, 0x0000000d, 0x00000118, 0x00000013, 0x00000120, 0x00000014, 
    0x00000130, 0x00000016, 0x0000013c, 0x00000017, 0x0000014c, 0x00000019, 
    0x0000015c, 0x0000001a, 0x0000016c, 0x7263694d, 0x666f736f, 0x52282074, 
    0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 
    0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 0xb00f0000, 
    0x00000005, 0x80070000, 0xb0e40000, 0xa0e40003, 0x40000001, 0x80080000, 
    0xa0ff0003, 0x00000005, 0x80070000, 0x90e40000, 0x80e40000, 0x00000005, 
    0x81070000, 0xa0e40000, 0x80e40000, 0x00000005, 0x800f0001, 0xb0e40000, 
    0xa0e40001, 0x00000012, 0x80070000, 0xb0ff0000, 0x80e40001, 0x80e40000, 
    0x0000ffff
};
#if 0
    ps_1_1

#line 13 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\VertexLitGeneric_SelfIlluminated.psh"
    tex t0
    texkill t3

#line 19
    mul r0.xyz, t0, c3
  + mov r0.w, c3.w
    mul r0.xyz, v0, r0
    mul_x2 r0.xyz, c0, r0
    mul r1, t0, c1
    lrp r0.xyz, t0.w, r1, r0

// approximately 7 instruction slots used (1 texture, 6 arithmetic)
#endif

static unsigned int pixelShader_VertexLitGeneric_SelfIlluminated_1[] =
{
    0xffff0101, 0x0046fffe, 0x47554244, 0x00000028, 0x000000e0, 0x00000000, 
    0x00000001, 0x0000009c, 0x00000008, 0x000000a0, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74726556, 0x694c7865, 0x6e654774, 0x63697265, 
    0x6c65535f, 0x6c6c4966, 0x6e696d75, 0x64657461, 0x6873702e, 0xababab00, 
    0x00000028, 0x0000000d, 0x00000120, 0x0000000f, 0x00000128, 0x00000013, 
    0x00000130, 0x00000014, 0x00000140, 0x00000016, 0x0000014c, 0x00000017, 
    0x0000015c, 0x00000019, 0x0000016c, 0x0000001a, 0x0000017c, 0x7263694d, 
    0x666f736f, 0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 
    0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 
    0x00000042, 0xb00f0000, 0x00000041, 0xb00f0003, 0x00000005, 0x80070000, 
    0xb0e40000, 0xa0e40003, 0x40000001, 0x80080000, 0xa0ff0003, 0x00000005, 
    0x80070000, 0x90e40000, 0x80e40000, 0x00000005, 0x81070000, 0xa0e40000, 
    0x80e40000, 0x00000005, 0x800f0001, 0xb0e40000, 0xa0e40001, 0x00000012, 
    0x80070000, 0xb0ff0000, 0x80e40001, 0x80e40000, 0x0000ffff
};
static PrecompiledShaderByteCode_t VertexLitGeneric_SelfIlluminated_pixel_shaders[2] = 
{
	{ pixelShader_VertexLitGeneric_SelfIlluminated_0, sizeof( pixelShader_VertexLitGeneric_SelfIlluminated_0 ) },
	{ pixelShader_VertexLitGeneric_SelfIlluminated_1, sizeof( pixelShader_VertexLitGeneric_SelfIlluminated_1 ) },
};
struct VertexLitGeneric_SelfIlluminatedPixelShader_t : public PrecompiledShader_t
{
	VertexLitGeneric_SelfIlluminatedPixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = VertexLitGeneric_SelfIlluminated_pixel_shaders;
		m_nShaderCount = 2;
		m_pName = "VertexLitGeneric_SelfIlluminated";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static VertexLitGeneric_SelfIlluminatedPixelShader_t VertexLitGeneric_SelfIlluminated_PixelShaderInstance;
