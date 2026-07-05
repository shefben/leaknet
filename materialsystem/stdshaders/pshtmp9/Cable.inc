#if 0
    ps_1_1

#line 17 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\Cable.psh"
    tex t0
    tex t1
    dp3 r1, t0_bx2, v1_bx2
    add r0, r1, c0
    mul r1, v0, r0
    mul r0.xyz, r1, t1
    mul r0.w, t1.w, v0.w

// approximately 7 instruction slots used (2 texture, 5 arithmetic)
#endif

static unsigned int pixelShader_Cable_0[] =
{
    0xffff0101, 0x003dfffe, 0x47554244, 0x00000028, 0x000000bc, 0x00000000, 
    0x00000001, 0x00000080, 0x00000007, 0x00000084, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x6c626143, 0x73702e65, 0xabab0068, 0x00000028, 
    0x00000011, 0x000000fc, 0x00000012, 0x00000104, 0x00000014, 0x0000010c, 
    0x00000015, 0x0000011c, 0x00000017, 0x0000012c, 0x00000018, 0x0000013c, 
    0x0000001a, 0x0000014c, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 
    0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 
    0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 0xb00f0000, 0x00000042, 
    0xb00f0001, 0x00000008, 0x800f0001, 0xb4e40000, 0x94e40001, 0x00000002, 
    0x800f0000, 0x80e40001, 0xa0e40000, 0x00000005, 0x800f0001, 0x90e40000, 
    0x80e40000, 0x00000005, 0x80070000, 0x80e40001, 0xb0e40001, 0x00000005, 
    0x80080000, 0xb0ff0001, 0x90ff0000, 0x0000ffff
};
static PrecompiledShaderByteCode_t Cable_pixel_shaders[1] = 
{
	{ pixelShader_Cable_0, sizeof( pixelShader_Cable_0 ) },
};
struct CablePixelShader_t : public PrecompiledShader_t
{
	CablePixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = Cable_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "Cable";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static CablePixelShader_t Cable_PixelShaderInstance;
