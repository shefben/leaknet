#if 0
    ps_1_1

#line 3 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\Shadow.psh"
    def c0, 1, 1, 1, 1
    tex t0

#line 9
    sub_sat r0.w, t0.w, v0.w

#line 12
    lrp r0, 1-r0.w, c0, c1

// approximately 3 instruction slots used (1 texture, 2 arithmetic)
#endif

static unsigned int pixelShader_Shadow_0[] =
{
    0xffff0101, 0x0037fffe, 0x47554244, 0x00000028, 0x000000a4, 0x00000000, 
    0x00000001, 0x00000080, 0x00000004, 0x00000084, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x64616853, 0x702e776f, 0xab006873, 0x00000028, 
    0x00000003, 0x000000e4, 0x00000005, 0x000000fc, 0x00000009, 0x00000104, 
    0x0000000c, 0x00000114, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 
    0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 
    0x35392e34, 0x36322e30, 0xab003635, 0x00000051, 0xa00f0000, 0x3f800000, 
    0x3f800000, 0x3f800000, 0x3f800000, 0x00000042, 0xb00f0000, 0x00000003, 
    0x80180000, 0xb0ff0000, 0x90ff0000, 0x00000012, 0x800f0000, 0x86ff0000, 
    0xa0e40000, 0xa0e40001, 0x0000ffff
};
static PrecompiledShaderByteCode_t Shadow_pixel_shaders[1] = 
{
	{ pixelShader_Shadow_0, sizeof( pixelShader_Shadow_0 ) },
};
struct ShadowPixelShader_t : public PrecompiledShader_t
{
	ShadowPixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = Shadow_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "Shadow";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static ShadowPixelShader_t Shadow_PixelShaderInstance;
