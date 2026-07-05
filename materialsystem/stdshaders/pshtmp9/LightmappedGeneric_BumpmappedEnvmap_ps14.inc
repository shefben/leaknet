#if 0
    ps_1_4

#line 6 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\LightmappedGeneric_BumpmappedEnvmap_ps14.psh"
    texld r0, t0
    texcrd r1.xyz, t1
    texcrd r2.xyz, t2
    texcrd r3.xyz, t3
    texld r4, t4

#line 16
    dp3 r5.x, r1, r0_bx2
    dp3 r5.y, r2, r0_bx2
    dp3 r5.z, r3, r0_bx2
    dp3_x2 r3.xyz, r5, r4_bx2
    mul r3.xyz, r5, r3
    dp3 r2.xyz, r5, r5
    mad r2.xyz, -r4_bx2, r2, r3
    mov r5, r0.w

#line 37
    phase

#line 40
    texld r3, r2

#line 43
    dp3_sat r1, v0_bx2, r0_bx2

#line 46
    mul r0.xyz, r3, c0

#line 49
    mul r1.xyz, r0, r0
  + mul r0.w, 1-r1.w, 1-r1.w
    lrp r0.xyz, c1, r1, r0
  + mul r0.w, r0.w, r0.w
    dp3 r1.xyz, r0, c3
  + mul r0.w, r0.w, 1-r1.w
    lrp r0.xyz, c2, r0, r1
    mad r0.w, r0.w, c6.w, c4.w

#line 61
    mul r0.xyz, r0, r0.w
  + mov r0.w, r5.x

// approximately 22 instruction slots used (6 texture, 16 arithmetic)
#endif

static unsigned int pixelShader_LightmappedGeneric_BumpmappedEnvmap_ps14_0[] =
{
    0xffff0104, 0x006efffe, 0x47554244, 0x00000028, 0x00000180, 0x00000000, 
    0x00000001, 0x000000a4, 0x0000001b, 0x000000a8, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x6867694c, 0x70616d74, 0x47646570, 0x72656e65, 
    0x425f6369, 0x6d706d75, 0x65707061, 0x766e4564, 0x5f70616d, 0x34317370, 
    0x6873702e, 0xababab00, 0x00000028, 0x00000006, 0x000001c0, 0x00000008, 
    0x000001cc, 0x00000009, 0x000001d8, 0x0000000a, 0x000001e4, 0x0000000c, 
    0x000001f0, 0x00000010, 0x000001fc, 0x00000011, 0x0000020c, 0x00000012, 
    0x0000021c, 0x00000014, 0x0000022c, 0x00000015, 0x0000023c, 0x00000016, 
    0x0000024c, 0x00000017, 0x0000025c, 0x00000019, 0x00000270, 0x00000025, 
    0x0000027c, 0x00000028, 0x00000280, 0x0000002b, 0x0000028c, 0x0000002e, 
    0x0000029c, 0x00000031, 0x000002ac, 0x00000032, 0x000002bc, 0x00000034, 
    0x000002cc, 0x00000035, 0x000002e0, 0x00000037, 0x000002f0, 0x00000038, 
    0x00000300, 0x0000003a, 0x00000310, 0x0000003b, 0x00000324, 0x0000003d, 
    0x00000338, 0x0000003e, 0x00000348, 0x7263694d, 0x666f736f, 0x52282074, 
    0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 
    0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 0x800f0000, 
    0xb0e40000, 0x00000040, 0x80070001, 0xb0e40001, 0x00000040, 0x80070002, 
    0xb0e40002, 0x00000040, 0x80070003, 0xb0e40003, 0x00000042, 0x800f0004, 
    0xb0e40004, 0x00000008, 0x80010005, 0x80e40001, 0x84e40000, 0x00000008, 
    0x80020005, 0x80e40002, 0x84e40000, 0x00000008, 0x80040005, 0x80e40003, 
    0x84e40000, 0x00000008, 0x81070003, 0x80e40005, 0x84e40004, 0x00000005, 
    0x80070003, 0x80e40005, 0x80e40003, 0x00000008, 0x80070002, 0x80e40005, 
    0x80e40005, 0x00000004, 0x80070002, 0x85e40004, 0x80e40002, 0x80e40003, 
    0x00000001, 0x800f0005, 0x80ff0000, 0x0000fffd, 0x00000042, 0x800f0003, 
    0x80e40002, 0x00000008, 0x801f0001, 0x94e40000, 0x84e40000, 0x00000005, 
    0x80070000, 0x80e40003, 0xa0e40000, 0x00000005, 0x80070001, 0x80e40000, 
    0x80e40000, 0x40000005, 0x80080000, 0x86ff0001, 0x86ff0001, 0x00000012, 
    0x80070000, 0xa0e40001, 0x80e40001, 0x80e40000, 0x40000005, 0x80080000, 
    0x80ff0000, 0x80ff0000, 0x00000008, 0x80070001, 0x80e40000, 0xa0e40003, 
    0x40000005, 0x80080000, 0x80ff0000, 0x86ff0001, 0x00000012, 0x80070000, 
    0xa0e40002, 0x80e40000, 0x80e40001, 0x00000004, 0x80080000, 0x80ff0000, 
    0xa0ff0006, 0xa0ff0004, 0x00000005, 0x80070000, 0x80e40000, 0x80ff0000, 
    0x40000001, 0x80080000, 0x80000005, 0x0000ffff
};
static PrecompiledShaderByteCode_t LightmappedGeneric_BumpmappedEnvmap_ps14_pixel_shaders[1] = 
{
	{ pixelShader_LightmappedGeneric_BumpmappedEnvmap_ps14_0, sizeof( pixelShader_LightmappedGeneric_BumpmappedEnvmap_ps14_0 ) },
};
struct LightmappedGeneric_BumpmappedEnvmap_ps14PixelShader_t : public PrecompiledShader_t
{
	LightmappedGeneric_BumpmappedEnvmap_ps14PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = LightmappedGeneric_BumpmappedEnvmap_ps14_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "LightmappedGeneric_BumpmappedEnvmap_ps14";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static LightmappedGeneric_BumpmappedEnvmap_ps14PixelShader_t LightmappedGeneric_BumpmappedEnvmap_ps14_PixelShaderInstance;
