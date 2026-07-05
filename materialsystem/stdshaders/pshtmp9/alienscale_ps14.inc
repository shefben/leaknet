#if 0
    ps_1_4

#line 3 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\alienscale_ps14.psh"
    def c0, 1, 1, 1, 0.5
    def c1, 0, 0, 0, 0
    texld r2, t1
    texld r3, t2
    texcrd r1.xyz, t3
    texcrd r4.xyz, t4
    texcrd r5.xyz, t5

#line 13
    dp3 r0.x, r1, r2_bx2
    dp3 r0.y, r4, r2_bx2
    dp3 r0.z, r5, r2_bx2
    dp3_sat r1.w, r3_bx2, r0
    sub r0.w, r1.w, r2.w
    cmp r0, r0.w, c0, c1
    mul_sat r2, r0, r1
    phase

#line 24
    texld r0, t0
    texld r1, t0
    lrp r0, r2.x, r1, r0
    mul r0, r0, v0

// approximately 16 instruction slots used (7 texture, 9 arithmetic)
#endif

static unsigned int pixelShader_alienscale_ps14_0[] =
{
    0xffff0104, 0x0057fffe, 0x47554244, 0x00000028, 0x00000124, 0x00000000, 
    0x00000001, 0x00000088, 0x00000013, 0x0000008c, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x65696c61, 0x6163736e, 0x705f656c, 0x2e343173, 
    0x00687370, 0x00000028, 0x00000003, 0x00000164, 0x00000004, 0x0000017c, 
    0x00000006, 0x00000194, 0x00000007, 0x000001a0, 0x00000008, 0x000001ac, 
    0x00000009, 0x000001b8, 0x0000000a, 0x000001c4, 0x0000000d, 0x000001d0, 
    0x0000000e, 0x000001e0, 0x0000000f, 0x000001f0, 0x00000011, 0x00000200, 
    0x00000012, 0x00000210, 0x00000013, 0x00000220, 0x00000014, 0x00000234, 
    0x00000016, 0x00000244, 0x00000018, 0x00000248, 0x00000019, 0x00000254, 
    0x0000001b, 0x00000260, 0x0000001c, 0x00000274, 0x7263694d, 0x666f736f, 
    0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 
    0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000051, 
    0xa00f0000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f000000, 0x00000051, 
    0xa00f0001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000042, 
    0x800f0002, 0xb0e40001, 0x00000042, 0x800f0003, 0xb0e40002, 0x00000040, 
    0x80070001, 0xb0e40003, 0x00000040, 0x80070004, 0xb0e40004, 0x00000040, 
    0x80070005, 0xb0e40005, 0x00000008, 0x80010000, 0x80e40001, 0x84e40002, 
    0x00000008, 0x80020000, 0x80e40004, 0x84e40002, 0x00000008, 0x80040000, 
    0x80e40005, 0x84e40002, 0x00000008, 0x80180001, 0x84e40003, 0x80e40000, 
    0x00000003, 0x80080000, 0x80ff0001, 0x80ff0002, 0x00000058, 0x800f0000, 
    0x80ff0000, 0xa0e40000, 0xa0e40001, 0x00000005, 0x801f0002, 0x80e40000, 
    0x80e40001, 0x0000fffd, 0x00000042, 0x800f0000, 0xb0e40000, 0x00000042, 
    0x800f0001, 0xb0e40000, 0x00000012, 0x800f0000, 0x80000002, 0x80e40001, 
    0x80e40000, 0x00000005, 0x800f0000, 0x80e40000, 0x90e40000, 0x0000ffff
};
static PrecompiledShaderByteCode_t alienscale_ps14_pixel_shaders[1] = 
{
	{ pixelShader_alienscale_ps14_0, sizeof( pixelShader_alienscale_ps14_0 ) },
};
struct alienscale_ps14PixelShader_t : public PrecompiledShader_t
{
	alienscale_ps14PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = alienscale_ps14_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "alienscale_ps14";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static alienscale_ps14PixelShader_t alienscale_ps14_PixelShaderInstance;
