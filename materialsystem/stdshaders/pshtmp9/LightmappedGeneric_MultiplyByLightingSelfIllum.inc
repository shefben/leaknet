#if 0
    ps_1_1

#line 11 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\LightmappedGeneric_MultiplyByLightingSelfIllum.psh"
    def c2, 1, 1, 1, 1
    tex t0
    tex t1

#line 18
    mul_x2 r1.xyz, c0, t1
  + mov_sat r1.w, v0
    lrp r0.xyz, t0.w, c1, r1
  + mov r0.w, c2.w
    lrp r0.xyz, r1.w, r0, c2

// approximately 5 instruction slots used (2 texture, 3 arithmetic)
#endif

static unsigned int pixelShader_LightmappedGeneric_MultiplyByLightingSelfIllum_0[] =
{
    0xffff0101, 0x0049fffe, 0x47554244, 0x00000028, 0x000000ec, 0x00000000, 
    0x00000001, 0x000000a8, 0x00000008, 0x000000ac, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x6867694c, 0x70616d74, 0x47646570, 0x72656e65, 
    0x4d5f6369, 0x69746c75, 0x42796c70, 0x67694c79, 0x6e697468, 0x6c655367, 
    0x6c6c4966, 0x702e6d75, 0xab006873, 0x00000028, 0x0000000b, 0x0000012c, 
    0x0000000d, 0x00000144, 0x0000000e, 0x0000014c, 0x00000012, 0x00000154, 
    0x00000013, 0x00000164, 0x00000015, 0x00000170, 0x00000016, 0x00000184, 
    0x00000018, 0x00000190, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 
    0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 
    0x35392e34, 0x36322e30, 0xab003635, 0x00000051, 0xa00f0002, 0x3f800000, 
    0x3f800000, 0x3f800000, 0x3f800000, 0x00000042, 0xb00f0000, 0x00000042, 
    0xb00f0001, 0x00000005, 0x81070001, 0xa0e40000, 0xb0e40001, 0x40000001, 
    0x80180001, 0x90e40000, 0x00000012, 0x80070000, 0xb0ff0000, 0xa0e40001, 
    0x80e40001, 0x40000001, 0x80080000, 0xa0ff0002, 0x00000012, 0x80070000, 
    0x80ff0001, 0x80e40000, 0xa0e40002, 0x0000ffff
};
static PrecompiledShaderByteCode_t LightmappedGeneric_MultiplyByLightingSelfIllum_pixel_shaders[1] = 
{
	{ pixelShader_LightmappedGeneric_MultiplyByLightingSelfIllum_0, sizeof( pixelShader_LightmappedGeneric_MultiplyByLightingSelfIllum_0 ) },
};
struct LightmappedGeneric_MultiplyByLightingSelfIllumPixelShader_t : public PrecompiledShader_t
{
	LightmappedGeneric_MultiplyByLightingSelfIllumPixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = LightmappedGeneric_MultiplyByLightingSelfIllum_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "LightmappedGeneric_MultiplyByLightingSelfIllum";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static LightmappedGeneric_MultiplyByLightingSelfIllumPixelShader_t LightmappedGeneric_MultiplyByLightingSelfIllum_PixelShaderInstance;
