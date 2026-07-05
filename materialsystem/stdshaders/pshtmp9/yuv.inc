#if 0
    ps_1_1

#line 11 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\yuv.psh"
    def c0, 0.298999995, 0.587000012, 0.114, 1

#line 14
    tex t0

#line 19
    dp3 r1, c0, t0
    mov r0.xyz, r1.w
    mov r0.w, c0.w

// approximately 4 instruction slots used (1 texture, 3 arithmetic)
#endif

static unsigned int pixelShader_yuv_0[] =
{
    0xffff0101, 0x0038fffe, 0x47554244, 0x00000028, 0x000000a8, 0x00000000, 
    0x00000001, 0x0000007c, 0x00000005, 0x00000080, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x2e767579, 0x00687370, 0x00000028, 0x0000000b, 
    0x000000e8, 0x0000000e, 0x00000100, 0x00000013, 0x00000108, 0x00000014, 
    0x00000118, 0x00000015, 0x00000124, 0x7263694d, 0x666f736f, 0x52282074, 
    0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 
    0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000051, 0xa00f0000, 
    0x3e991687, 0x3f1645a2, 0x3de978d5, 0x3f800000, 0x00000042, 0xb00f0000, 
    0x00000008, 0x800f0001, 0xa0e40000, 0xb0e40000, 0x00000001, 0x80070000, 
    0x80ff0001, 0x00000001, 0x80080000, 0xa0ff0000, 0x0000ffff
};
static PrecompiledShaderByteCode_t yuv_pixel_shaders[1] = 
{
	{ pixelShader_yuv_0, sizeof( pixelShader_yuv_0 ) },
};
struct yuvPixelShader_t : public PrecompiledShader_t
{
	yuvPixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = yuv_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "yuv";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static yuvPixelShader_t yuv_PixelShaderInstance;
