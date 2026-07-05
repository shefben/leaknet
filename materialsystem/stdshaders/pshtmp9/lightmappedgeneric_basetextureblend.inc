#if 0
    ps_1_1

#line 3 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\lightmappedgeneric_basetextureblend.psh"
    tex t0
    tex t1
    tex t2
    mov r1, t1
    lrp r0, t2.w, t0, r1

// approximately 5 instruction slots used (3 texture, 2 arithmetic)
#endif

static unsigned int pixelShader_lightmappedgeneric_basetextureblend_0[] =
{
    0xffff0101, 0x0040fffe, 0x47554244, 0x00000028, 0x000000c8, 0x00000000, 
    0x00000001, 0x0000009c, 0x00000005, 0x000000a0, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x6867696c, 0x70616d74, 0x67646570, 0x72656e65, 
    0x625f6369, 0x74657361, 0x75747865, 0x6c626572, 0x2e646e65, 0x00687370, 
    0x00000028, 0x00000003, 0x00000108, 0x00000004, 0x00000110, 0x00000005, 
    0x00000118, 0x00000007, 0x00000120, 0x00000008, 0x0000012c, 0x7263694d, 
    0x666f736f, 0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 
    0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 
    0x00000042, 0xb00f0000, 0x00000042, 0xb00f0001, 0x00000042, 0xb00f0002, 
    0x00000001, 0x800f0001, 0xb0e40001, 0x00000012, 0x800f0000, 0xb0ff0002, 
    0xb0e40000, 0x80e40001, 0x0000ffff
};
static PrecompiledShaderByteCode_t lightmappedgeneric_basetextureblend_pixel_shaders[1] = 
{
	{ pixelShader_lightmappedgeneric_basetextureblend_0, sizeof( pixelShader_lightmappedgeneric_basetextureblend_0 ) },
};
struct lightmappedgeneric_basetextureblendPixelShader_t : public PrecompiledShader_t
{
	lightmappedgeneric_basetextureblendPixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = lightmappedgeneric_basetextureblend_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "lightmappedgeneric_basetextureblend";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static lightmappedgeneric_basetextureblendPixelShader_t lightmappedgeneric_basetextureblend_PixelShaderInstance;
