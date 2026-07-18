#if 0
    ps_1_1

#line 4 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\MonitorScreen.psh"
    def c4, 0.333330005, 0.333330005, 0.333330005, 0.333330005
    tex t0
    tex t1
    mul r0, t0, v0

#line 13
    mul r1.xyz, r0, r0
    lrp r0.xyz, c1, r1, r0
    dp3 r1.xyz, r0, c4
    lrp r0.xyz, c2, r0, r1
    mul r0.xyz, r0, c3

// approximately 8 instruction slots used (2 texture, 6 arithmetic)
#endif

static unsigned int pixelShader_MonitorScreen_0[] =
{
    0xffff0101, 0x0043fffe, 0x47554244, 0x00000028, 0x000000d4, 0x00000000, 
    0x00000001, 0x00000088, 0x00000009, 0x0000008c, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x696e6f4d, 0x53726f74, 0x65657263, 0x73702e6e, 
    0xabab0068, 0x00000028, 0x00000004, 0x00000114, 0x00000006, 0x0000012c, 
    0x00000007, 0x00000134, 0x00000009, 0x0000013c, 0x0000000d, 0x0000014c, 
    0x0000000e, 0x0000015c, 0x0000000f, 0x00000170, 0x00000010, 0x00000180, 
    0x00000011, 0x00000194, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 
    0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 
    0x35392e34, 0x36322e30, 0xab003635, 0x00000051, 0xa00f0004, 0x3eaaaa3b, 
    0x3eaaaa3b, 0x3eaaaa3b, 0x3eaaaa3b, 0x00000042, 0xb00f0000, 0x00000042, 
    0xb00f0001, 0x00000005, 0x800f0000, 0xb0e40000, 0x90e40000, 0x00000005, 
    0x80070001, 0x80e40000, 0x80e40000, 0x00000012, 0x80070000, 0xa0e40001, 
    0x80e40001, 0x80e40000, 0x00000008, 0x80070001, 0x80e40000, 0xa0e40004, 
    0x00000012, 0x80070000, 0xa0e40002, 0x80e40000, 0x80e40001, 0x00000005, 
    0x80070000, 0x80e40000, 0xa0e40003, 0x0000ffff
};
#if 0
    ps_1_1

#line 4 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\MonitorScreen.psh"
    def c4, 0.333330005, 0.333330005, 0.333330005, 0.333330005
    tex t0
    tex t1
    mul r0, t0, v0
    mul r0, t1, r0
    mul r1.xyz, r0, r0
    lrp r0.xyz, c1, r1, r0

#line 15
    dp3 r1.xyz, r0, c4
    lrp r0.xyz, c2, r0, r1
    mul r0.xyz, r0, c3

// approximately 9 instruction slots used (2 texture, 7 arithmetic)
#endif

static unsigned int pixelShader_MonitorScreen_1[] =
{
    0xffff0101, 0x0045fffe, 0x47554244, 0x00000028, 0x000000dc, 0x00000000, 
    0x00000001, 0x00000088, 0x0000000a, 0x0000008c, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x696e6f4d, 0x53726f74, 0x65657263, 0x73702e6e, 
    0xabab0068, 0x00000028, 0x00000004, 0x0000011c, 0x00000006, 0x00000134, 
    0x00000007, 0x0000013c, 0x00000009, 0x00000144, 0x0000000b, 0x00000154, 
    0x0000000d, 0x00000164, 0x0000000e, 0x00000174, 0x0000000f, 0x00000188, 
    0x00000010, 0x00000198, 0x00000011, 0x000001ac, 0x7263694d, 0x666f736f, 
    0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 
    0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000051, 
    0xa00f0004, 0x3eaaaa3b, 0x3eaaaa3b, 0x3eaaaa3b, 0x3eaaaa3b, 0x00000042, 
    0xb00f0000, 0x00000042, 0xb00f0001, 0x00000005, 0x800f0000, 0xb0e40000, 
    0x90e40000, 0x00000005, 0x800f0000, 0xb0e40001, 0x80e40000, 0x00000005, 
    0x80070001, 0x80e40000, 0x80e40000, 0x00000012, 0x80070000, 0xa0e40001, 
    0x80e40001, 0x80e40000, 0x00000008, 0x80070001, 0x80e40000, 0xa0e40004, 
    0x00000012, 0x80070000, 0xa0e40002, 0x80e40000, 0x80e40001, 0x00000005, 
    0x80070000, 0x80e40000, 0xa0e40003, 0x0000ffff
};
static PrecompiledShaderByteCode_t MonitorScreen_pixel_shaders[2] = 
{
	{ pixelShader_MonitorScreen_0, sizeof( pixelShader_MonitorScreen_0 ) },
	{ pixelShader_MonitorScreen_1, sizeof( pixelShader_MonitorScreen_1 ) },
};
struct MonitorScreenPixelShader_t : public PrecompiledShader_t
{
	MonitorScreenPixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = MonitorScreen_pixel_shaders;
		m_nShaderCount = 2;
		m_pName = "MonitorScreen";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static MonitorScreenPixelShader_t MonitorScreen_PixelShaderInstance;
