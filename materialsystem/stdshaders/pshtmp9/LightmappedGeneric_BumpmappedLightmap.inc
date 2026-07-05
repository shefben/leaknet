#if 0
    ps_1_1

#line 17 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\LightmappedGeneric_BumpmappedLightmap.psh"
    tex t0

#line 20
    tex t1
    tex t2
    tex t3

#line 25
    dp3_sat r1, t0_bx2, c0

#line 28
    mul r0, t1, r1

#line 31
    dp3_sat r1, t0_bx2, c1
    mad r0, r1, t2, r0
    dp3_sat r1, t0_bx2, c2
    mad r0, r1, t3, r0
    mul r0, r0, c3

// approximately 11 instruction slots used (4 texture, 7 arithmetic)
#endif

static unsigned int pixelShader_LightmappedGeneric_BumpmappedLightmap_0[] =
{
    0xffff0101, 0x004dfffe, 0x47554244, 0x00000028, 0x000000fc, 0x00000000, 
    0x00000001, 0x000000a0, 0x0000000b, 0x000000a4, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x6867694c, 0x70616d74, 0x47646570, 0x72656e65, 
    0x425f6369, 0x6d706d75, 0x65707061, 0x67694c64, 0x616d7468, 0x73702e70, 
    0xabab0068, 0x00000028, 0x00000011, 0x0000013c, 0x00000014, 0x00000144, 
    0x00000015, 0x0000014c, 0x00000016, 0x00000154, 0x00000019, 0x0000015c, 
    0x0000001c, 0x0000016c, 0x0000001f, 0x0000017c, 0x00000020, 0x0000018c, 
    0x00000022, 0x000001a0, 0x00000023, 0x000001b0, 0x00000025, 0x000001c4, 
    0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 0x20395844, 0x64616853, 
    0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 
    0xab003635, 0x00000042, 0xb00f0000, 0x00000042, 0xb00f0001, 0x00000042, 
    0xb00f0002, 0x00000042, 0xb00f0003, 0x00000008, 0x801f0001, 0xb4e40000, 
    0xa0e40000, 0x00000005, 0x800f0000, 0xb0e40001, 0x80e40001, 0x00000008, 
    0x801f0001, 0xb4e40000, 0xa0e40001, 0x00000004, 0x800f0000, 0x80e40001, 
    0xb0e40002, 0x80e40000, 0x00000008, 0x801f0001, 0xb4e40000, 0xa0e40002, 
    0x00000004, 0x800f0000, 0x80e40001, 0xb0e40003, 0x80e40000, 0x00000005, 
    0x800f0000, 0x80e40000, 0xa0e40003, 0x0000ffff
};
static PrecompiledShaderByteCode_t LightmappedGeneric_BumpmappedLightmap_pixel_shaders[1] = 
{
	{ pixelShader_LightmappedGeneric_BumpmappedLightmap_0, sizeof( pixelShader_LightmappedGeneric_BumpmappedLightmap_0 ) },
};
struct LightmappedGeneric_BumpmappedLightmapPixelShader_t : public PrecompiledShader_t
{
	LightmappedGeneric_BumpmappedLightmapPixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = LightmappedGeneric_BumpmappedLightmap_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "LightmappedGeneric_BumpmappedLightmap";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static LightmappedGeneric_BumpmappedLightmapPixelShader_t LightmappedGeneric_BumpmappedLightmap_PixelShaderInstance;
