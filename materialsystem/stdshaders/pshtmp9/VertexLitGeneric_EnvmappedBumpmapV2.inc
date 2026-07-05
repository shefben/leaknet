#if 0
    ps_1_1

#line 18 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\VertexLitGeneric_EnvmappedBumpmapV2.psh"
    tex t0

#line 23
    texm3x3pad t1, t0_bx2
    texm3x3pad t2, t0_bx2
    texm3x3vspec t3, t0_bx2

#line 28
    mul r0.xyz, t3, c0
  + mov r0.w, c0.w
    mul r1.xyz, r0, r0
    lrp r0.xyz, c1, r1, r0
    dp3 r1.xyz, r0, c3
    lrp r0.xyz, c2, r0, r1

// approximately 9 instruction slots used (4 texture, 5 arithmetic)
#endif

static unsigned int pixelShader_VertexLitGeneric_EnvmappedBumpmapV2_0[] =
{
    0xffff0101, 0x004afffe, 0x47554244, 0x00000028, 0x000000f0, 0x00000000, 
    0x00000001, 0x0000009c, 0x0000000a, 0x000000a0, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74726556, 0x694c7865, 0x6e654774, 0x63697265, 
    0x766e455f, 0x7070616d, 0x75426465, 0x616d706d, 0x2e325670, 0x00687370, 
    0x00000028, 0x00000012, 0x00000130, 0x00000017, 0x00000138, 0x00000018, 
    0x00000144, 0x00000019, 0x00000150, 0x0000001c, 0x0000015c, 0x0000001d, 
    0x0000016c, 0x0000001f, 0x00000178, 0x00000020, 0x00000188, 0x00000021, 
    0x0000019c, 0x00000022, 0x000001ac, 0x7263694d, 0x666f736f, 0x52282074, 
    0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 
    0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 0xb00f0000, 
    0x00000049, 0xb00f0001, 0xb4e40000, 0x00000049, 0xb00f0002, 0xb4e40000, 
    0x0000004d, 0xb00f0003, 0xb4e40000, 0x00000005, 0x80070000, 0xb0e40003, 
    0xa0e40000, 0x40000001, 0x80080000, 0xa0ff0000, 0x00000005, 0x80070001, 
    0x80e40000, 0x80e40000, 0x00000012, 0x80070000, 0xa0e40001, 0x80e40001, 
    0x80e40000, 0x00000008, 0x80070001, 0x80e40000, 0xa0e40003, 0x00000012, 
    0x80070000, 0xa0e40002, 0x80e40000, 0x80e40001, 0x0000ffff
};
static PrecompiledShaderByteCode_t VertexLitGeneric_EnvmappedBumpmapV2_pixel_shaders[1] = 
{
	{ pixelShader_VertexLitGeneric_EnvmappedBumpmapV2_0, sizeof( pixelShader_VertexLitGeneric_EnvmappedBumpmapV2_0 ) },
};
struct VertexLitGeneric_EnvmappedBumpmapV2PixelShader_t : public PrecompiledShader_t
{
	VertexLitGeneric_EnvmappedBumpmapV2PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = VertexLitGeneric_EnvmappedBumpmapV2_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "VertexLitGeneric_EnvmappedBumpmapV2";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static VertexLitGeneric_EnvmappedBumpmapV2PixelShader_t VertexLitGeneric_EnvmappedBumpmapV2_PixelShaderInstance;
