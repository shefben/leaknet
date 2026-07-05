#if 0
    ps_1_1

#line 12 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\VertexLitGeneric_Detail.psh"
    tex t0
    tex t3
    mul r0, t0, c3
    mul r0.xyz, v0, r0
    mul_x2 r0.xyz, c0, r0
    mul_x2 r0.xyz, r0, t3

// approximately 6 instruction slots used (2 texture, 4 arithmetic)
#endif

static unsigned int pixelShader_VertexLitGeneric_Detail_0[] =
{
    0xffff0101, 0x003ffffe, 0x47554244, 0x00000028, 0x000000c4, 0x00000000, 
    0x00000001, 0x00000090, 0x00000006, 0x00000094, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74726556, 0x694c7865, 0x6e654774, 0x63697265, 
    0x7465445f, 0x2e6c6961, 0x00687370, 0x00000028, 0x0000000c, 0x00000104, 
    0x0000000d, 0x0000010c, 0x0000000f, 0x00000114, 0x00000010, 0x00000124, 
    0x00000011, 0x00000134, 0x00000012, 0x00000144, 0x7263694d, 0x666f736f, 
    0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 
    0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 
    0xb00f0000, 0x00000042, 0xb00f0003, 0x00000005, 0x800f0000, 0xb0e40000, 
    0xa0e40003, 0x00000005, 0x80070000, 0x90e40000, 0x80e40000, 0x00000005, 
    0x81070000, 0xa0e40000, 0x80e40000, 0x00000005, 0x81070000, 0x80e40000, 
    0xb0e40003, 0x0000ffff
};
static PrecompiledShaderByteCode_t VertexLitGeneric_Detail_pixel_shaders[1] = 
{
	{ pixelShader_VertexLitGeneric_Detail_0, sizeof( pixelShader_VertexLitGeneric_Detail_0 ) },
};
struct VertexLitGeneric_DetailPixelShader_t : public PrecompiledShader_t
{
	VertexLitGeneric_DetailPixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = VertexLitGeneric_Detail_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "VertexLitGeneric_Detail";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static VertexLitGeneric_DetailPixelShader_t VertexLitGeneric_Detail_PixelShaderInstance;
