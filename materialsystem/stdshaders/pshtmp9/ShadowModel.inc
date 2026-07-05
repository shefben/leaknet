#if 0
    ps_1_1

#line 11 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\ShadowModel.psh"
    def c0, 1, 1, 1, 1
    tex t0
    texkill t1
    texkill t2
    texkill t3

#line 20
    lrp r0.xyz, t0.w, v0, c0
  + mov r0.w, c0.w

// approximately 5 instruction slots used (1 texture, 4 arithmetic)
#endif

static unsigned int pixelShader_ShadowModel_0[] =
{
    0xffff0101, 0x003efffe, 0x47554244, 0x00000028, 0x000000c0, 0x00000000, 
    0x00000001, 0x00000084, 0x00000007, 0x00000088, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x64616853, 0x6f4d776f, 0x2e6c6564, 0x00687370, 
    0x00000028, 0x0000000b, 0x00000100, 0x0000000d, 0x00000118, 0x0000000e, 
    0x00000120, 0x0000000f, 0x00000128, 0x00000010, 0x00000130, 0x00000014, 
    0x00000138, 0x00000015, 0x0000014c, 0x7263694d, 0x666f736f, 0x52282074, 
    0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 
    0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000051, 0xa00f0000, 
    0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x00000042, 0xb00f0000, 
    0x00000041, 0xb00f0001, 0x00000041, 0xb00f0002, 0x00000041, 0xb00f0003, 
    0x00000012, 0x80070000, 0xb0ff0000, 0x90e40000, 0xa0e40000, 0x40000001, 
    0x80080000, 0xa0ff0000, 0x0000ffff
};
static PrecompiledShaderByteCode_t ShadowModel_pixel_shaders[1] = 
{
	{ pixelShader_ShadowModel_0, sizeof( pixelShader_ShadowModel_0 ) },
};
struct ShadowModelPixelShader_t : public PrecompiledShader_t
{
	ShadowModelPixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = ShadowModel_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "ShadowModel";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static ShadowModelPixelShader_t ShadowModel_PixelShaderInstance;
