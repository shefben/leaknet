#if 0
    ps_1_1

#line 21 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\LightmappedGeneric_BumpmappedEnvmap.psh"
    tex t0

#line 26
    texm3x3pad t1, t0_bx2
    texm3x3pad t2, t0_bx2
    texm3x3vspec t3, t0_bx2

#line 37
    mul r0.xyz, t3, c0

#line 40
    dp3_sat r1, v0_bx2, t0_bx2

#line 43
    mul r1.xyz, r0, r0
  + mul r0.w, 1-r1.w, 1-r1.w
    lrp r0.xyz, c1, r1, r0
  + mul r0.w, r0.w, r0.w
    dp3 r1.xyz, r0, c3
  + mul r0.w, r0.w, 1-r1.w

#line 53
    lrp r0.xyz, c2, r0, r1
    mad r0.w, r0.w, c6.w, c4.w
    mul r0.xyz, r0, r0.w
  + mov r0.w, t0.w

// approximately 12 instruction slots used (4 texture, 8 arithmetic)
#endif

static unsigned int pixelShader_LightmappedGeneric_BumpmappedEnvmap_0[] =
{
    0xffff0101, 0x0056fffe, 0x47554244, 0x00000028, 0x00000120, 0x00000000, 
    0x00000001, 0x0000009c, 0x00000010, 0x000000a0, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x6867694c, 0x70616d74, 0x47646570, 0x72656e65, 
    0x425f6369, 0x6d706d75, 0x65707061, 0x766e4564, 0x2e70616d, 0x00687370, 
    0x00000028, 0x00000015, 0x00000160, 0x0000001a, 0x00000168, 0x0000001b, 
    0x00000174, 0x0000001c, 0x00000180, 0x00000025, 0x0000018c, 0x00000028, 
    0x0000019c, 0x0000002b, 0x000001ac, 0x0000002c, 0x000001bc, 0x0000002e, 
    0x000001cc, 0x0000002f, 0x000001e0, 0x00000031, 0x000001f0, 0x00000032, 
    0x00000200, 0x00000035, 0x00000210, 0x00000036, 0x00000224, 0x00000038, 
    0x00000238, 0x00000039, 0x00000248, 0x7263694d, 0x666f736f, 0x52282074, 
    0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 
    0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 0xb00f0000, 
    0x00000049, 0xb00f0001, 0xb4e40000, 0x00000049, 0xb00f0002, 0xb4e40000, 
    0x0000004d, 0xb00f0003, 0xb4e40000, 0x00000005, 0x80070000, 0xb0e40003, 
    0xa0e40000, 0x00000008, 0x801f0001, 0x94e40000, 0xb4e40000, 0x00000005, 
    0x80070001, 0x80e40000, 0x80e40000, 0x40000005, 0x80080000, 0x86ff0001, 
    0x86ff0001, 0x00000012, 0x80070000, 0xa0e40001, 0x80e40001, 0x80e40000, 
    0x40000005, 0x80080000, 0x80ff0000, 0x80ff0000, 0x00000008, 0x80070001, 
    0x80e40000, 0xa0e40003, 0x40000005, 0x80080000, 0x80ff0000, 0x86ff0001, 
    0x00000012, 0x80070000, 0xa0e40002, 0x80e40000, 0x80e40001, 0x00000004, 
    0x80080000, 0x80ff0000, 0xa0ff0006, 0xa0ff0004, 0x00000005, 0x80070000, 
    0x80e40000, 0x80ff0000, 0x40000001, 0x80080000, 0xb0ff0000, 0x0000ffff
};
static PrecompiledShaderByteCode_t LightmappedGeneric_BumpmappedEnvmap_pixel_shaders[1] = 
{
	{ pixelShader_LightmappedGeneric_BumpmappedEnvmap_0, sizeof( pixelShader_LightmappedGeneric_BumpmappedEnvmap_0 ) },
};
struct LightmappedGeneric_BumpmappedEnvmapPixelShader_t : public PrecompiledShader_t
{
	LightmappedGeneric_BumpmappedEnvmapPixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = LightmappedGeneric_BumpmappedEnvmap_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "LightmappedGeneric_BumpmappedEnvmap";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static LightmappedGeneric_BumpmappedEnvmapPixelShader_t LightmappedGeneric_BumpmappedEnvmap_PixelShaderInstance;
