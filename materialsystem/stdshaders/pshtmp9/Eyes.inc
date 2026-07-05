#if 0
    ps_1_1

#line 10 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\Eyes.psh"
    tex t0
    tex t1
    tex t2
    lrp r0, t1.w, t1, t0
    mad r0.xyz, r0, v0, t2
  + mov r0.w, t0.w

// approximately 5 instruction slots used (3 texture, 2 arithmetic)
#endif

static unsigned int pixelShader_Eyes_0[] =
{
    0xffff0101, 0x003bfffe, 0x47554244, 0x00000028, 0x000000b4, 0x00000000, 
    0x00000001, 0x00000080, 0x00000006, 0x00000084, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x73657945, 0x6873702e, 0xababab00, 0x00000028, 
    0x0000000a, 0x000000f4, 0x0000000b, 0x000000fc, 0x0000000c, 0x00000104, 
    0x0000000e, 0x0000010c, 0x0000000f, 0x00000120, 0x00000010, 0x00000134, 
    0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 0x20395844, 0x64616853, 
    0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 
    0xab003635, 0x00000042, 0xb00f0000, 0x00000042, 0xb00f0001, 0x00000042, 
    0xb00f0002, 0x00000012, 0x800f0000, 0xb0ff0001, 0xb0e40001, 0xb0e40000, 
    0x00000004, 0x80070000, 0x80e40000, 0x90e40000, 0xb0e40002, 0x40000001, 
    0x80080000, 0xb0ff0000, 0x0000ffff
};
static PrecompiledShaderByteCode_t Eyes_pixel_shaders[1] = 
{
	{ pixelShader_Eyes_0, sizeof( pixelShader_Eyes_0 ) },
};
struct EyesPixelShader_t : public PrecompiledShader_t
{
	EyesPixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = Eyes_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "Eyes";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static EyesPixelShader_t Eyes_PixelShaderInstance;
