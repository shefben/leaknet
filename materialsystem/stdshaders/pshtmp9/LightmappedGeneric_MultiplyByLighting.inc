#if 0
    ps_1_1

#line 11 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\LightmappedGeneric_MultiplyByLighting.psh"
    def c2, 1, 1, 1, 1
    tex t0
    tex t1

#line 18
    mul_x2 r1.xyz, c0, t1
  + mul r1.w, t0, v0
    lrp r0, r1.w, r1, c2

// approximately 4 instruction slots used (2 texture, 2 arithmetic)
#endif

static unsigned int pixelShader_LightmappedGeneric_MultiplyByLighting_0[] =
{
    0xffff0101, 0x0043fffe, 0x47554244, 0x00000028, 0x000000d4, 0x00000000, 
    0x00000001, 0x000000a0, 0x00000006, 0x000000a4, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x6867694c, 0x70616d74, 0x47646570, 0x72656e65, 
    0x4d5f6369, 0x69746c75, 0x42796c70, 0x67694c79, 0x6e697468, 0x73702e67, 
    0xabab0068, 0x00000028, 0x0000000b, 0x00000114, 0x0000000d, 0x0000012c, 
    0x0000000e, 0x00000134, 0x00000012, 0x0000013c, 0x00000013, 0x0000014c, 
    0x00000014, 0x0000015c, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 
    0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 
    0x35392e34, 0x36322e30, 0xab003635, 0x00000051, 0xa00f0002, 0x3f800000, 
    0x3f800000, 0x3f800000, 0x3f800000, 0x00000042, 0xb00f0000, 0x00000042, 
    0xb00f0001, 0x00000005, 0x81070001, 0xa0e40000, 0xb0e40001, 0x40000005, 
    0x80080001, 0xb0e40000, 0x90e40000, 0x00000012, 0x800f0000, 0x80ff0001, 
    0x80e40001, 0xa0e40002, 0x0000ffff
};
static PrecompiledShaderByteCode_t LightmappedGeneric_MultiplyByLighting_pixel_shaders[1] = 
{
	{ pixelShader_LightmappedGeneric_MultiplyByLighting_0, sizeof( pixelShader_LightmappedGeneric_MultiplyByLighting_0 ) },
};
struct LightmappedGeneric_MultiplyByLightingPixelShader_t : public PrecompiledShader_t
{
	LightmappedGeneric_MultiplyByLightingPixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = LightmappedGeneric_MultiplyByLighting_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "LightmappedGeneric_MultiplyByLighting";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static LightmappedGeneric_MultiplyByLightingPixelShader_t LightmappedGeneric_MultiplyByLighting_PixelShaderInstance;
