#if 0
    ps_1_4

#line 3 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\shadow_ps14.psh"
    def c0, 1, 1, 1, 1
    def c2, 0.200000003, 0.200000003, 0.200000003, 0.200000003
    texld r0, t0
    texld r1, t1
    texld r2, t2
    texld r3, t3
    texld r4, t4

#line 13
    mul r0.x, r0.w, c2.w
    mad r0.x, r1.w, c2.w, r0.x
    mad r0.x, r2.w, c2.w, r0.x
    mad r0.x, r3.w, c2.w, r0.x
    mad r0.x, r4.w, c2.w, r0.x

#line 21
    sub_sat r0.w, r0.x, v0.w

#line 24
    lrp r0, 1-r0.w, c0, c1

// approximately 12 instruction slots used (5 texture, 7 arithmetic)
#endif

static unsigned int pixelShader_shadow_ps14_0[] =
{
    0xffff0104, 0x004cfffe, 0x47554244, 0x00000028, 0x000000f8, 0x00000000, 
    0x00000001, 0x00000084, 0x0000000e, 0x00000088, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x64616873, 0x705f776f, 0x2e343173, 0x00687370, 
    0x00000028, 0x00000003, 0x00000138, 0x00000004, 0x00000150, 0x00000006, 
    0x00000168, 0x00000007, 0x00000174, 0x00000008, 0x00000180, 0x00000009, 
    0x0000018c, 0x0000000a, 0x00000198, 0x0000000d, 0x000001a4, 0x0000000e, 
    0x000001b4, 0x0000000f, 0x000001c8, 0x00000010, 0x000001dc, 0x00000011, 
    0x000001f0, 0x00000015, 0x00000204, 0x00000018, 0x00000214, 0x7263694d, 
    0x666f736f, 0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 
    0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 
    0x00000051, 0xa00f0000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 
    0x00000051, 0xa00f0002, 0x3e4ccccd, 0x3e4ccccd, 0x3e4ccccd, 0x3e4ccccd, 
    0x00000042, 0x800f0000, 0xb0e40000, 0x00000042, 0x800f0001, 0xb0e40001, 
    0x00000042, 0x800f0002, 0xb0e40002, 0x00000042, 0x800f0003, 0xb0e40003, 
    0x00000042, 0x800f0004, 0xb0e40004, 0x00000005, 0x80010000, 0x80ff0000, 
    0xa0ff0002, 0x00000004, 0x80010000, 0x80ff0001, 0xa0ff0002, 0x80000000, 
    0x00000004, 0x80010000, 0x80ff0002, 0xa0ff0002, 0x80000000, 0x00000004, 
    0x80010000, 0x80ff0003, 0xa0ff0002, 0x80000000, 0x00000004, 0x80010000, 
    0x80ff0004, 0xa0ff0002, 0x80000000, 0x00000003, 0x80180000, 0x80000000, 
    0x90ff0000, 0x00000012, 0x800f0000, 0x86ff0000, 0xa0e40000, 0xa0e40001, 
    0x0000ffff
};
static PrecompiledShaderByteCode_t shadow_ps14_pixel_shaders[1] = 
{
	{ pixelShader_shadow_ps14_0, sizeof( pixelShader_shadow_ps14_0 ) },
};
struct shadow_ps14PixelShader_t : public PrecompiledShader_t
{
	shadow_ps14PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = shadow_ps14_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "shadow_ps14";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static shadow_ps14PixelShader_t shadow_ps14_PixelShaderInstance;
