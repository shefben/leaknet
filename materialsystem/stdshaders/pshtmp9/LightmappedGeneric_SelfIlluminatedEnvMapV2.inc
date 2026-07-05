#if 0
    ps_1_1

#line 13 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\LightmappedGeneric_SelfIlluminatedEnvMapV2.psh"
    tex t0
    tex t1
    tex t2
    mul r0.xyz, t0, v0
  + mov r0.w, v0.w
    mul r1, t0.w, t0
    mad r1, c1, r1, t1
    mul r0.xyz, r1, r0

#line 24
    mul_x2 r0.xyz, c0, r0
    mad r0.xyz, t2, c2, r0

// approximately 9 instruction slots used (3 texture, 6 arithmetic)
#endif

static unsigned int pixelShader_LightmappedGeneric_SelfIlluminatedEnvMapV2_0[] =
{
    0xffff0101, 0x004cfffe, 0x47554244, 0x00000028, 0x000000f8, 0x00000000, 
    0x00000001, 0x000000a4, 0x0000000a, 0x000000a8, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x6867694c, 0x70616d74, 0x47646570, 0x72656e65, 
    0x535f6369, 0x49666c65, 0x6d756c6c, 0x74616e69, 0x6e456465, 0x70614d76, 
    0x702e3256, 0xab006873, 0x00000028, 0x0000000d, 0x00000138, 0x0000000e, 
    0x00000140, 0x0000000f, 0x00000148, 0x00000011, 0x00000150, 0x00000012, 
    0x00000160, 0x00000014, 0x0000016c, 0x00000015, 0x0000017c, 0x00000017, 
    0x00000190, 0x00000018, 0x000001a0, 0x0000001a, 0x000001b0, 0x7263694d, 
    0x666f736f, 0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 
    0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 
    0x00000042, 0xb00f0000, 0x00000042, 0xb00f0001, 0x00000042, 0xb00f0002, 
    0x00000005, 0x80070000, 0xb0e40000, 0x90e40000, 0x40000001, 0x80080000, 
    0x90ff0000, 0x00000005, 0x800f0001, 0xb0ff0000, 0xb0e40000, 0x00000004, 
    0x800f0001, 0xa0e40001, 0x80e40001, 0xb0e40001, 0x00000005, 0x80070000, 
    0x80e40001, 0x80e40000, 0x00000005, 0x81070000, 0xa0e40000, 0x80e40000, 
    0x00000004, 0x80070000, 0xb0e40002, 0xa0e40002, 0x80e40000, 0x0000ffff
};
static PrecompiledShaderByteCode_t LightmappedGeneric_SelfIlluminatedEnvMapV2_pixel_shaders[1] = 
{
	{ pixelShader_LightmappedGeneric_SelfIlluminatedEnvMapV2_0, sizeof( pixelShader_LightmappedGeneric_SelfIlluminatedEnvMapV2_0 ) },
};
struct LightmappedGeneric_SelfIlluminatedEnvMapV2PixelShader_t : public PrecompiledShader_t
{
	LightmappedGeneric_SelfIlluminatedEnvMapV2PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = LightmappedGeneric_SelfIlluminatedEnvMapV2_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "LightmappedGeneric_SelfIlluminatedEnvMapV2";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static LightmappedGeneric_SelfIlluminatedEnvMapV2PixelShader_t LightmappedGeneric_SelfIlluminatedEnvMapV2_PixelShaderInstance;
