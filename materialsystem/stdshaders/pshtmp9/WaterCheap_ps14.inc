#if 0
    ps_1_4

#line 4 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\WaterCheap_ps14.psh"
    texld r0, t0
    texcrd r1.xyz, t1
    texcrd r2.xyz, t2
    texcrd r3.xyz, t3
    texld r4, t4

#line 13
    dp3 r5.x, r1, r0_bx2
    dp3 r5.y, r2, r0_bx2
    dp3 r5.z, r3, r0_bx2
    dp3_x2 r3.xyz, r5, r4_bx2
    mul r3.xyz, r5, r3
    dp3 r2.xyz, r5, r5
    mad r2.xyz, -r4_bx2, r2, r3

#line 23
    phase

#line 26
    texld r3, r2

#line 29
    dp3_sat r1, v0_bx2, r0_bx2

#line 32
    mul r3.xyz, r3, c1

#line 35
    mul r0.w, 1-r1.w, 1-r1.w
    mul r0.w, r0.w, r0.w
    mul_sat r0.w, r0.w, 1-r1.w

#line 40
    lrp r0.xyz, r0.w, r3, c0
  + mov r0.w, v0.w

// approximately 19 instruction slots used (6 texture, 13 arithmetic)
#endif

static unsigned int pixelShader_WaterCheap_ps14_0[] =
{
    0xffff0104, 0x005bfffe, 0x47554244, 0x00000028, 0x00000134, 0x00000000, 
    0x00000001, 0x00000088, 0x00000015, 0x0000008c, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x65746157, 0x65684372, 0x705f7061, 0x2e343173, 
    0x00687370, 0x00000028, 0x00000004, 0x00000174, 0x00000006, 0x00000180, 
    0x00000007, 0x0000018c, 0x00000008, 0x00000198, 0x0000000a, 0x000001a4, 
    0x0000000d, 0x000001b0, 0x0000000e, 0x000001c0, 0x0000000f, 0x000001d0, 
    0x00000011, 0x000001e0, 0x00000012, 0x000001f0, 0x00000013, 0x00000200, 
    0x00000014, 0x00000210, 0x00000017, 0x00000224, 0x0000001a, 0x00000228, 
    0x0000001d, 0x00000234, 0x00000020, 0x00000244, 0x00000023, 0x00000254, 
    0x00000024, 0x00000264, 0x00000025, 0x00000274, 0x00000028, 0x00000284, 
    0x00000029, 0x00000298, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 
    0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 
    0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 0x800f0000, 0xb0e40000, 
    0x00000040, 0x80070001, 0xb0e40001, 0x00000040, 0x80070002, 0xb0e40002, 
    0x00000040, 0x80070003, 0xb0e40003, 0x00000042, 0x800f0004, 0xb0e40004, 
    0x00000008, 0x80010005, 0x80e40001, 0x84e40000, 0x00000008, 0x80020005, 
    0x80e40002, 0x84e40000, 0x00000008, 0x80040005, 0x80e40003, 0x84e40000, 
    0x00000008, 0x81070003, 0x80e40005, 0x84e40004, 0x00000005, 0x80070003, 
    0x80e40005, 0x80e40003, 0x00000008, 0x80070002, 0x80e40005, 0x80e40005, 
    0x00000004, 0x80070002, 0x85e40004, 0x80e40002, 0x80e40003, 0x0000fffd, 
    0x00000042, 0x800f0003, 0x80e40002, 0x00000008, 0x801f0001, 0x94e40000, 
    0x84e40000, 0x00000005, 0x80070003, 0x80e40003, 0xa0e40001, 0x00000005, 
    0x80080000, 0x86ff0001, 0x86ff0001, 0x00000005, 0x80080000, 0x80ff0000, 
    0x80ff0000, 0x00000005, 0x80180000, 0x80ff0000, 0x86ff0001, 0x00000012, 
    0x80070000, 0x80ff0000, 0x80e40003, 0xa0e40000, 0x40000001, 0x80080000, 
    0x90ff0000, 0x0000ffff
};
static PrecompiledShaderByteCode_t WaterCheap_ps14_pixel_shaders[1] = 
{
	{ pixelShader_WaterCheap_ps14_0, sizeof( pixelShader_WaterCheap_ps14_0 ) },
};
struct WaterCheap_ps14PixelShader_t : public PrecompiledShader_t
{
	WaterCheap_ps14PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = WaterCheap_ps14_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "WaterCheap_ps14";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static WaterCheap_ps14PixelShader_t WaterCheap_ps14_PixelShaderInstance;
