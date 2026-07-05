#if 0
    ps_1_1

#line 12 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\Refract_ps11.psh"
    tex t0
    texbem t1, t0

#line 22
    mov r0, t1

#line 26
    mul r0, r0, c0

// approximately 4 instruction slots used (2 texture, 2 arithmetic)
#endif

static unsigned int pixelShader_Refract_ps11_0[] =
{
    0xffff0101, 0x0039fffe, 0x47554244, 0x00000028, 0x000000ac, 0x00000000, 
    0x00000001, 0x00000088, 0x00000004, 0x0000008c, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x72666552, 0x5f746361, 0x31317370, 0x6873702e, 
    0xababab00, 0x00000028, 0x0000000c, 0x000000ec, 0x0000000d, 0x000000f4, 
    0x00000016, 0x00000100, 0x0000001a, 0x0000010c, 0x7263694d, 0x666f736f, 
    0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 
    0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 
    0xb00f0000, 0x00000043, 0xb00f0001, 0xb0e40000, 0x00000001, 0x800f0000, 
    0xb0e40001, 0x00000005, 0x800f0000, 0x80e40000, 0xa0e40000, 0x0000ffff
};
#if 0
    ps_1_1

#line 12 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\Refract_ps11.psh"
    tex t0
    texbem t1, t0

#line 16
    tex t2

#line 20
    mul_x2 r0, t1, t2

#line 26
    mul r0, r0, c0

// approximately 5 instruction slots used (3 texture, 2 arithmetic)
#endif

static unsigned int pixelShader_Refract_ps11_1[] =
{
    0xffff0101, 0x003bfffe, 0x47554244, 0x00000028, 0x000000b4, 0x00000000, 
    0x00000001, 0x00000088, 0x00000005, 0x0000008c, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x72666552, 0x5f746361, 0x31317370, 0x6873702e, 
    0xababab00, 0x00000028, 0x0000000c, 0x000000f4, 0x0000000d, 0x000000fc, 
    0x00000010, 0x00000108, 0x00000014, 0x00000110, 0x0000001a, 0x00000120, 
    0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 0x20395844, 0x64616853, 
    0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 
    0xab003635, 0x00000042, 0xb00f0000, 0x00000043, 0xb00f0001, 0xb0e40000, 
    0x00000042, 0xb00f0002, 0x00000005, 0x810f0000, 0xb0e40001, 0xb0e40002, 
    0x00000005, 0x800f0000, 0x80e40000, 0xa0e40000, 0x0000ffff
};
static PrecompiledShaderByteCode_t Refract_ps11_pixel_shaders[2] = 
{
	{ pixelShader_Refract_ps11_0, sizeof( pixelShader_Refract_ps11_0 ) },
	{ pixelShader_Refract_ps11_1, sizeof( pixelShader_Refract_ps11_1 ) },
};
struct Refract_ps11PixelShader_t : public PrecompiledShader_t
{
	Refract_ps11PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = Refract_ps11_pixel_shaders;
		m_nShaderCount = 2;
		m_pName = "Refract_ps11";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static Refract_ps11PixelShader_t Refract_ps11_PixelShaderInstance;
