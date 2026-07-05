#if 0
    ps_1_1

#line 3 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\alienscale_ps11.psh"
    def c0, 1, 1, 1, 0.5
    def c1, 0, 0, 0, 0
    tex t0
    tex t1
    tex t2
    texcoord t3
    dp3_sat r1, t3, t2_bx2
    sub r0.w, r1.w, t2.w
    add r0.w, r0.w, c0.w

#line 14
    cnd r0, r0.w, c0, c1
    mul_sat r0, r0, r1
    lrp r0, r0, t1, t0
    mul r0, r0, v0

// approximately 11 instruction slots used (4 texture, 7 arithmetic)
#endif

static unsigned int pixelShader_alienscale_ps11_0[] =
{
    0xffff0101, 0x004bfffe, 0x47554244, 0x00000028, 0x000000f4, 0x00000000, 
    0x00000001, 0x00000088, 0x0000000d, 0x0000008c, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x65696c61, 0x6163736e, 0x705f656c, 0x2e313173, 
    0x00687370, 0x00000028, 0x00000003, 0x00000134, 0x00000004, 0x0000014c, 
    0x00000006, 0x00000164, 0x00000007, 0x0000016c, 0x00000008, 0x00000174, 
    0x00000009, 0x0000017c, 0x0000000b, 0x00000184, 0x0000000c, 0x00000194, 
    0x0000000d, 0x000001a4, 0x0000000e, 0x000001b4, 0x0000000f, 0x000001c8, 
    0x00000010, 0x000001d8, 0x00000011, 0x000001ec, 0x7263694d, 0x666f736f, 
    0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 
    0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000051, 
    0xa00f0000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f000000, 0x00000051, 
    0xa00f0001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000042, 
    0xb00f0000, 0x00000042, 0xb00f0001, 0x00000042, 0xb00f0002, 0x00000040, 
    0xb00f0003, 0x00000008, 0x801f0001, 0xb0e40003, 0xb4e40002, 0x00000003, 
    0x80080000, 0x80ff0001, 0xb0ff0002, 0x00000002, 0x80080000, 0x80ff0000, 
    0xa0ff0000, 0x00000050, 0x800f0000, 0x80ff0000, 0xa0e40000, 0xa0e40001, 
    0x00000005, 0x801f0000, 0x80e40000, 0x80e40001, 0x00000012, 0x800f0000, 
    0x80e40000, 0xb0e40001, 0xb0e40000, 0x00000005, 0x800f0000, 0x80e40000, 
    0x90e40000, 0x0000ffff
};
static PrecompiledShaderByteCode_t alienscale_ps11_pixel_shaders[1] = 
{
	{ pixelShader_alienscale_ps11_0, sizeof( pixelShader_alienscale_ps11_0 ) },
};
struct alienscale_ps11PixelShader_t : public PrecompiledShader_t
{
	alienscale_ps11PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = alienscale_ps11_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "alienscale_ps11";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static alienscale_ps11PixelShader_t alienscale_ps11_PixelShaderInstance;
