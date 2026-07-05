#if 0
    ps_1_1

#line 10 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\WaterRefract_ps11.psh"
    tex t0
    texbem t1, t0
    mov r0, t1

// approximately 3 instruction slots used (2 texture, 1 arithmetic)
#endif

static unsigned int pixelShader_WaterRefract_ps11_0[] =
{
    0xffff0101, 0x0038fffe, 0x47554244, 0x00000028, 0x000000a8, 0x00000000, 
    0x00000001, 0x0000008c, 0x00000003, 0x00000090, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x65746157, 0x66655272, 0x74636172, 0x3173705f, 
    0x73702e31, 0xabab0068, 0x00000028, 0x0000000a, 0x000000e8, 0x0000000b, 
    0x000000f0, 0x0000000d, 0x000000fc, 0x7263694d, 0x666f736f, 0x52282074, 
    0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 
    0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 0xb00f0000, 
    0x00000043, 0xb00f0001, 0xb0e40000, 0x00000001, 0x800f0000, 0xb0e40001, 
    0x0000ffff
};
static PrecompiledShaderByteCode_t WaterRefract_ps11_pixel_shaders[1] = 
{
	{ pixelShader_WaterRefract_ps11_0, sizeof( pixelShader_WaterRefract_ps11_0 ) },
};
struct WaterRefract_ps11PixelShader_t : public PrecompiledShader_t
{
	WaterRefract_ps11PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = WaterRefract_ps11_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "WaterRefract_ps11";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static WaterRefract_ps11PixelShader_t WaterRefract_ps11_PixelShaderInstance;
