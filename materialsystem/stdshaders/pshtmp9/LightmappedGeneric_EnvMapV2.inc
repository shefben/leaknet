#if 0
    ps_1_1

#line 13 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\LightmappedGeneric_EnvMapV2.psh"
    tex t0
    tex t1
    tex t2
    mul r0, t0, v0
    mul r0.xyz, t1, r0
    mul_x2 r0.xyz, c0, r0
    mad r0.xyz, t2, c2, r0

// approximately 7 instruction slots used (3 texture, 4 arithmetic)
#endif

static unsigned int pixelShader_LightmappedGeneric_EnvMapV2_0[] =
{
    0xffff0101, 0x0042fffe, 0x47554244, 0x00000028, 0x000000d0, 0x00000000, 
    0x00000001, 0x00000094, 0x00000007, 0x00000098, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x6867694c, 0x70616d74, 0x47646570, 0x72656e65, 
    0x455f6369, 0x614d766e, 0x2e325670, 0x00687370, 0x00000028, 0x0000000d, 
    0x00000110, 0x0000000e, 0x00000118, 0x0000000f, 0x00000120, 0x00000011, 
    0x00000128, 0x00000012, 0x00000138, 0x00000013, 0x00000148, 0x00000014, 
    0x00000158, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 0x20395844, 
    0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 
    0x36322e30, 0xab003635, 0x00000042, 0xb00f0000, 0x00000042, 0xb00f0001, 
    0x00000042, 0xb00f0002, 0x00000005, 0x800f0000, 0xb0e40000, 0x90e40000, 
    0x00000005, 0x80070000, 0xb0e40001, 0x80e40000, 0x00000005, 0x81070000, 
    0xa0e40000, 0x80e40000, 0x00000004, 0x80070000, 0xb0e40002, 0xa0e40002, 
    0x80e40000, 0x0000ffff
};
static PrecompiledShaderByteCode_t LightmappedGeneric_EnvMapV2_pixel_shaders[1] = 
{
	{ pixelShader_LightmappedGeneric_EnvMapV2_0, sizeof( pixelShader_LightmappedGeneric_EnvMapV2_0 ) },
};
struct LightmappedGeneric_EnvMapV2PixelShader_t : public PrecompiledShader_t
{
	LightmappedGeneric_EnvMapV2PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = LightmappedGeneric_EnvMapV2_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "LightmappedGeneric_EnvMapV2";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static LightmappedGeneric_EnvMapV2PixelShader_t LightmappedGeneric_EnvMapV2_PixelShaderInstance;
