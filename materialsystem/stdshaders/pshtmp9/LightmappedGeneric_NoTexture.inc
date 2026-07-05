#if 0
    ps_1_1

#line 11 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\LightmappedGeneric_NoTexture.psh"
    tex t1
    mul r0.xyz, t1, v0
  + mov r0.w, v0.w
    mul_x2 r0.xyz, c0, r0

// approximately 3 instruction slots used (1 texture, 2 arithmetic)
#endif

static unsigned int pixelShader_LightmappedGeneric_NoTexture_0[] =
{
    0xffff0101, 0x003dfffe, 0x47554244, 0x00000028, 0x000000bc, 0x00000000, 
    0x00000001, 0x00000098, 0x00000004, 0x0000009c, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x6867694c, 0x70616d74, 0x47646570, 0x72656e65, 
    0x4e5f6369, 0x7865546f, 0x65727574, 0x6873702e, 0xababab00, 0x00000028, 
    0x0000000b, 0x000000fc, 0x0000000c, 0x00000104, 0x0000000d, 0x00000114, 
    0x0000000e, 0x00000120, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 
    0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 
    0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 0xb00f0001, 0x00000005, 
    0x80070000, 0xb0e40001, 0x90e40000, 0x40000001, 0x80080000, 0x90ff0000, 
    0x00000005, 0x81070000, 0xa0e40000, 0x80e40000, 0x0000ffff
};
static PrecompiledShaderByteCode_t LightmappedGeneric_NoTexture_pixel_shaders[1] = 
{
	{ pixelShader_LightmappedGeneric_NoTexture_0, sizeof( pixelShader_LightmappedGeneric_NoTexture_0 ) },
};
struct LightmappedGeneric_NoTexturePixelShader_t : public PrecompiledShader_t
{
	LightmappedGeneric_NoTexturePixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = LightmappedGeneric_NoTexture_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "LightmappedGeneric_NoTexture";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static LightmappedGeneric_NoTexturePixelShader_t LightmappedGeneric_NoTexture_PixelShaderInstance;
