#if 0
    ps_1_1

#line 19 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\BumpmappedLightmap_OverBright2.psh"
    tex t0

#line 22
    tex t1
    tex t2
    tex t3

#line 27
    dp3_sat r1, t0_bx2, c0

#line 30
    mul r0, t1, r1

#line 33
    dp3_sat r1, t0_bx2, c1
    mad r0, r1, t2, r0
    dp3_sat r1, t0_bx2, c2
    mad r0, r1, t3, r0

// approximately 10 instruction slots used (4 texture, 6 arithmetic)
#endif

static unsigned int pixelShader_BumpmappedLightmap_OverBright2_0[] =
{
    0xffff0101, 0x0049fffe, 0x47554244, 0x00000028, 0x000000ec, 0x00000000, 
    0x00000001, 0x00000098, 0x0000000a, 0x0000009c, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x706d7542, 0x7070616d, 0x694c6465, 0x6d746867, 
    0x4f5f7061, 0x42726576, 0x68676972, 0x702e3274, 0xab006873, 0x00000028, 
    0x00000013, 0x0000012c, 0x00000016, 0x00000134, 0x00000017, 0x0000013c, 
    0x00000018, 0x00000144, 0x0000001b, 0x0000014c, 0x0000001e, 0x0000015c, 
    0x00000021, 0x0000016c, 0x00000022, 0x0000017c, 0x00000024, 0x00000190, 
    0x00000025, 0x000001a0, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 
    0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 
    0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 0xb00f0000, 0x00000042, 
    0xb00f0001, 0x00000042, 0xb00f0002, 0x00000042, 0xb00f0003, 0x00000008, 
    0x801f0001, 0xb4e40000, 0xa0e40000, 0x00000005, 0x800f0000, 0xb0e40001, 
    0x80e40001, 0x00000008, 0x801f0001, 0xb4e40000, 0xa0e40001, 0x00000004, 
    0x800f0000, 0x80e40001, 0xb0e40002, 0x80e40000, 0x00000008, 0x801f0001, 
    0xb4e40000, 0xa0e40002, 0x00000004, 0x800f0000, 0x80e40001, 0xb0e40003, 
    0x80e40000, 0x0000ffff
};
static PrecompiledShaderByteCode_t BumpmappedLightmap_OverBright2_pixel_shaders[1] = 
{
	{ pixelShader_BumpmappedLightmap_OverBright2_0, sizeof( pixelShader_BumpmappedLightmap_OverBright2_0 ) },
};
struct BumpmappedLightmap_OverBright2PixelShader_t : public PrecompiledShader_t
{
	BumpmappedLightmap_OverBright2PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = BumpmappedLightmap_OverBright2_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "BumpmappedLightmap_OverBright2";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static BumpmappedLightmap_OverBright2PixelShader_t BumpmappedLightmap_OverBright2_PixelShaderInstance;
