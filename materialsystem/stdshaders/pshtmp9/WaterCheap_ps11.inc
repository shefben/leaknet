#if 0
    ps_1_1

#line 4 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\WaterCheap_ps11.psh"
    tex t0

#line 9
    texm3x3pad t1, t0_bx2
    texm3x3pad t2, t0_bx2
    texm3x3vspec t3, t0_bx2

#line 15
    mov r0.xyz, t3
    dp3_sat t2, v0_bx2, t0_bx2

#line 20
    mul r0.xyz, r0, c1
  + mul t1.w, 1-t2.w, 1-t2.w
    mul t1.w, t1.w, t1.w
    mul_sat t1.w, t1.w, 1-t2.w

#line 27
    lrp r0.xyz, t1.w, r0, c0
  + mov r0.w, v0.w

// approximately 10 instruction slots used (4 texture, 6 arithmetic)
#endif

static unsigned int pixelShader_WaterCheap_ps11_0[] =
{
    0xffff0101, 0x0049fffe, 0x47554244, 0x00000028, 0x000000ec, 0x00000000, 
    0x00000001, 0x00000088, 0x0000000c, 0x0000008c, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x65746157, 0x65684372, 0x705f7061, 0x2e313173, 
    0x00687370, 0x00000028, 0x00000004, 0x0000012c, 0x00000009, 0x00000134, 
    0x0000000a, 0x00000140, 0x0000000b, 0x0000014c, 0x0000000f, 0x00000158, 
    0x00000011, 0x00000164, 0x00000014, 0x00000174, 0x00000015, 0x00000184, 
    0x00000017, 0x00000194, 0x00000018, 0x000001a4, 0x0000001b, 0x000001b4, 
    0x0000001c, 0x000001c8, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 
    0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 
    0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 0xb00f0000, 0x00000049, 
    0xb00f0001, 0xb4e40000, 0x00000049, 0xb00f0002, 0xb4e40000, 0x0000004d, 
    0xb00f0003, 0xb4e40000, 0x00000001, 0x80070000, 0xb0e40003, 0x00000008, 
    0xb01f0002, 0x94e40000, 0xb4e40000, 0x00000005, 0x80070000, 0x80e40000, 
    0xa0e40001, 0x40000005, 0xb0080001, 0xb6ff0002, 0xb6ff0002, 0x00000005, 
    0xb0080001, 0xb0ff0001, 0xb0ff0001, 0x00000005, 0xb0180001, 0xb0ff0001, 
    0xb6ff0002, 0x00000012, 0x80070000, 0xb0ff0001, 0x80e40000, 0xa0e40000, 
    0x40000001, 0x80080000, 0x90ff0000, 0x0000ffff
};
static PrecompiledShaderByteCode_t WaterCheap_ps11_pixel_shaders[1] = 
{
	{ pixelShader_WaterCheap_ps11_0, sizeof( pixelShader_WaterCheap_ps11_0 ) },
};
struct WaterCheap_ps11PixelShader_t : public PrecompiledShader_t
{
	WaterCheap_ps11PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = WaterCheap_ps11_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "WaterCheap_ps11";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static WaterCheap_ps11PixelShader_t WaterCheap_ps11_PixelShaderInstance;
