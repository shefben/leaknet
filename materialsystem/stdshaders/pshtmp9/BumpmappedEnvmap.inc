#if 0
    ps_1_1

#line 17 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\BumpmappedEnvmap.psh"
    tex t0

#line 22
    texm3x3pad t1, t0_bx2
    texm3x3pad t2, t0_bx2
    texm3x3vspec t3, t0_bx2

#line 27
    mul r0, t3, c0

#line 30
    mov r0.w, t0.w

// approximately 6 instruction slots used (4 texture, 2 arithmetic)
#endif

static unsigned int pixelShader_BumpmappedEnvmap_0[] =
{
    0xffff0101, 0x003efffe, 0x47554244, 0x00000028, 0x000000c0, 0x00000000, 
    0x00000001, 0x0000008c, 0x00000006, 0x00000090, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x706d7542, 0x7070616d, 0x6e456465, 0x70616d76, 
    0x6873702e, 0xababab00, 0x00000028, 0x00000011, 0x00000100, 0x00000016, 
    0x00000108, 0x00000017, 0x00000114, 0x00000018, 0x00000120, 0x0000001b, 
    0x0000012c, 0x0000001e, 0x0000013c, 0x7263694d, 0x666f736f, 0x52282074, 
    0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 
    0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 0xb00f0000, 
    0x00000049, 0xb00f0001, 0xb4e40000, 0x00000049, 0xb00f0002, 0xb4e40000, 
    0x0000004d, 0xb00f0003, 0xb4e40000, 0x00000005, 0x800f0000, 0xb0e40003, 
    0xa0e40000, 0x00000001, 0x80080000, 0xb0ff0000, 0x0000ffff
};
static PrecompiledShaderByteCode_t BumpmappedEnvmap_pixel_shaders[1] = 
{
	{ pixelShader_BumpmappedEnvmap_0, sizeof( pixelShader_BumpmappedEnvmap_0 ) },
};
struct BumpmappedEnvmapPixelShader_t : public PrecompiledShader_t
{
	BumpmappedEnvmapPixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = BumpmappedEnvmap_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "BumpmappedEnvmap";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static BumpmappedEnvmapPixelShader_t BumpmappedEnvmap_PixelShaderInstance;
