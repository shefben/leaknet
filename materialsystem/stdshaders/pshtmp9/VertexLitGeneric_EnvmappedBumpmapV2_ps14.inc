#if 0
    ps_1_4

#line 6 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\VertexLitGeneric_EnvmappedBumpmapV2_ps14.psh"
    texld r0, t0
    texcrd r1.xyz, t1
    texcrd r2.xyz, t2
    texcrd r3.xyz, t3
    texld r4, t4

#line 15
    dp3 r5.x, r1, r0_bx2
    dp3 r5.y, r2, r0_bx2
    dp3 r5.z, r3, r0_bx2
    dp3_x2 r3.xyz, r5, r4_bx2
    mul r3.xyz, r5, r3
    dp3 r2.xyz, r5, r5
    mad r2.xyz, -r4_bx2, r2, r3
    mov r5, r0.w

#line 28
    phase
    texld r3, r2
    mul r0.xyz, r3, c0
  + mov r0.w, c0.w
    mul r1.xyz, r0, r0
    lrp r0.xyz, c1, r1, r0
    dp3 r1.xyz, r0, c3
    lrp r0.xyz, c2, r0, r1

// approximately 19 instruction slots used (6 texture, 13 arithmetic)
#endif

static unsigned int pixelShader_VertexLitGeneric_EnvmappedBumpmapV2_ps14_0[] =
{
    0xffff0104, 0x0062fffe, 0x47554244, 0x00000028, 0x00000150, 0x00000000, 
    0x00000001, 0x000000a4, 0x00000015, 0x000000a8, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74726556, 0x694c7865, 0x6e654774, 0x63697265, 
    0x766e455f, 0x7070616d, 0x75426465, 0x616d706d, 0x5f325670, 0x34317370, 
    0x6873702e, 0xababab00, 0x00000028, 0x00000006, 0x00000190, 0x00000008, 
    0x0000019c, 0x00000009, 0x000001a8, 0x0000000a, 0x000001b4, 0x0000000c, 
    0x000001c0, 0x0000000f, 0x000001cc, 0x00000010, 0x000001dc, 0x00000011, 
    0x000001ec, 0x00000013, 0x000001fc, 0x00000014, 0x0000020c, 0x00000015, 
    0x0000021c, 0x00000016, 0x0000022c, 0x00000018, 0x00000240, 0x0000001c, 
    0x0000024c, 0x0000001e, 0x00000250, 0x00000020, 0x0000025c, 0x00000021, 
    0x0000026c, 0x00000023, 0x00000278, 0x00000024, 0x00000288, 0x00000025, 
    0x0000029c, 0x00000026, 0x000002ac, 0x7263694d, 0x666f736f, 0x52282074, 
    0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 
    0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x00000042, 0x800f0000, 
    0xb0e40000, 0x00000040, 0x80070001, 0xb0e40001, 0x00000040, 0x80070002, 
    0xb0e40002, 0x00000040, 0x80070003, 0xb0e40003, 0x00000042, 0x800f0004, 
    0xb0e40004, 0x00000008, 0x80010005, 0x80e40001, 0x84e40000, 0x00000008, 
    0x80020005, 0x80e40002, 0x84e40000, 0x00000008, 0x80040005, 0x80e40003, 
    0x84e40000, 0x00000008, 0x81070003, 0x80e40005, 0x84e40004, 0x00000005, 
    0x80070003, 0x80e40005, 0x80e40003, 0x00000008, 0x80070002, 0x80e40005, 
    0x80e40005, 0x00000004, 0x80070002, 0x85e40004, 0x80e40002, 0x80e40003, 
    0x00000001, 0x800f0005, 0x80ff0000, 0x0000fffd, 0x00000042, 0x800f0003, 
    0x80e40002, 0x00000005, 0x80070000, 0x80e40003, 0xa0e40000, 0x40000001, 
    0x80080000, 0xa0ff0000, 0x00000005, 0x80070001, 0x80e40000, 0x80e40000, 
    0x00000012, 0x80070000, 0xa0e40001, 0x80e40001, 0x80e40000, 0x00000008, 
    0x80070001, 0x80e40000, 0xa0e40003, 0x00000012, 0x80070000, 0xa0e40002, 
    0x80e40000, 0x80e40001, 0x0000ffff
};
static PrecompiledShaderByteCode_t VertexLitGeneric_EnvmappedBumpmapV2_ps14_pixel_shaders[1] = 
{
	{ pixelShader_VertexLitGeneric_EnvmappedBumpmapV2_ps14_0, sizeof( pixelShader_VertexLitGeneric_EnvmappedBumpmapV2_ps14_0 ) },
};
struct VertexLitGeneric_EnvmappedBumpmapV2_ps14PixelShader_t : public PrecompiledShader_t
{
	VertexLitGeneric_EnvmappedBumpmapV2_ps14PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = VertexLitGeneric_EnvmappedBumpmapV2_ps14_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "VertexLitGeneric_EnvmappedBumpmapV2_ps14";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static VertexLitGeneric_EnvmappedBumpmapV2_ps14PixelShader_t VertexLitGeneric_EnvmappedBumpmapV2_ps14_PixelShaderInstance;
