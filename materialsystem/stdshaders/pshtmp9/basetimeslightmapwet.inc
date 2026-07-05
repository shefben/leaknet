#if 0
    ps_1_1

#line 16 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\basetimeslightmapwet.psh"
    tex t0

#line 21
    texm3x3pad t1, t0_bx2
    texm3x3pad t2, t0_bx2
    texm3x3vspec t3, t0_bx2

#line 26
    mov r0, t3

// approximately 5 instruction slots used (4 texture, 1 arithmetic)
#endif

static unsigned int pixelShader_basetimeslightmapwet_0[] =
{
    0xffff0101, 0x003dfffe, 0x47554244, 0x00000028, 0x000000bc, 0x00000000, 
    0x00000001, 0x00000090, 0x00000005, 0x00000094, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x65736162, 0x656d6974, 0x67696c73, 0x616d7468, 
    0x74657770, 0x6873702e, 0xababab00, 0x00000028, 0x00000010, 0x000000fc, 
    0x00000015, 0x00000104, 0x00000016, 0x00000110, 0x00000017, 0x0000011c, 
    0x0000001a, 0x00000128, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 
    0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 
    0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 0xb00f0000, 0x00000049, 
    0xb00f0001, 0xb4e40000, 0x00000049, 0xb00f0002, 0xb4e40000, 0x0000004d, 
    0xb00f0003, 0xb4e40000, 0x00000001, 0x800f0000, 0xb0e40003, 0x0000ffff
};
static PrecompiledShaderByteCode_t basetimeslightmapwet_pixel_shaders[1] = 
{
	{ pixelShader_basetimeslightmapwet_0, sizeof( pixelShader_basetimeslightmapwet_0 ) },
};
struct basetimeslightmapwetPixelShader_t : public PrecompiledShader_t
{
	basetimeslightmapwetPixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = basetimeslightmapwet_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "basetimeslightmapwet";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static basetimeslightmapwetPixelShader_t basetimeslightmapwet_PixelShaderInstance;
