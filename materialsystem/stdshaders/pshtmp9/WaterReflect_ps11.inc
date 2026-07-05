#if 0
    ps_1_1

#line 15 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\WaterReflect_ps11.psh"
    tex t0
    texbem t1, t0
    tex t2
    tex t3

#line 21
    dp3_sat r1, t3_bx2, t2_bx2

#line 24
    mul r0.w, 1-r1.w, 1-r1.w
    mul r0.w, r0.w, r0.w
    mov r0.xyz, t1
  + mul_sat r0.w, r0.w, 1-r1.w

// approximately 8 instruction slots used (4 texture, 4 arithmetic)
#endif

static unsigned int pixelShader_WaterReflect_ps11_0[] =
{
    0xffff0101, 0x0044fffe, 0x47554244, 0x00000028, 0x000000d8, 0x00000000, 
    0x00000001, 0x0000008c, 0x00000009, 0x00000090, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x65746157, 0x66655272, 0x7463656c, 0x3173705f, 
    0x73702e31, 0xabab0068, 0x00000028, 0x0000000f, 0x00000118, 0x00000010, 
    0x00000120, 0x00000011, 0x0000012c, 0x00000012, 0x00000134, 0x00000015, 
    0x0000013c, 0x00000018, 0x0000014c, 0x00000019, 0x0000015c, 0x0000001a, 
    0x0000016c, 0x0000001b, 0x00000178, 0x7263694d, 0x666f736f, 0x52282074, 
    0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 
    0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 0xb00f0000, 
    0x00000043, 0xb00f0001, 0xb0e40000, 0x00000042, 0xb00f0002, 0x00000042, 
    0xb00f0003, 0x00000008, 0x801f0001, 0xb4e40003, 0xb4e40002, 0x00000005, 
    0x80080000, 0x86ff0001, 0x86ff0001, 0x00000005, 0x80080000, 0x80ff0000, 
    0x80ff0000, 0x00000001, 0x80070000, 0xb0e40001, 0x40000005, 0x80180000, 
    0x80ff0000, 0x86ff0001, 0x0000ffff
};
static PrecompiledShaderByteCode_t WaterReflect_ps11_pixel_shaders[1] = 
{
	{ pixelShader_WaterReflect_ps11_0, sizeof( pixelShader_WaterReflect_ps11_0 ) },
};
struct WaterReflect_ps11PixelShader_t : public PrecompiledShader_t
{
	WaterReflect_ps11PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = WaterReflect_ps11_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "WaterReflect_ps11";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static WaterReflect_ps11PixelShader_t WaterReflect_ps11_PixelShaderInstance;
