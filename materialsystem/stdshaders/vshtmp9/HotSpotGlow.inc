#if 0
    vs_1_1

#line 9 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\vshtmp9\vshtmp9\vertexShader_HotSpotGlow_none_none_none_none_0_rangefog.tmp"
    dcl_position v0
    dcl_texcoord v7

#line 19
    dp4 r0.x, v0, c4
    dp4 r0.y, v0, c5
    dp4 r0.z, v0, c6
    dp4 r0.w, v0, c7
    mov oPos, r0

#line 42
    mad oFog, -r0.z, c16.w, c16.x

#line 55
    add oT0, v7, c95.yyzz
    add oT1, v7, c95.xyzz
    add oT2, v7, c95.xxzz
    add oT3, v7, c95.yxzz

// approximately 10 instruction slots used
#endif

 static unsigned int vertexShader_HotSpotGlow_none_none_none_none_0_rangefog[] =
{
    0xfffe0101, 0x0057fffe, 0x47554244, 0x00000028, 0x00000124, 0x00000000, 
    0x00000001, 0x000000c0, 0x0000000c, 0x000000c4, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74687376, 0x5c39706d, 0x74687376, 0x5c39706d, 
    0x74726576, 0x68537865, 0x72656461, 0x746f485f, 0x746f7053, 0x776f6c47, 
    0x6e6f6e5f, 0x6f6e5f65, 0x6e5f656e, 0x5f656e6f, 0x656e6f6e, 0x725f305f, 
    0x65676e61, 0x2e676f66, 0x00706d74, 0x00000028, 0x00000009, 0x00000164, 
    0x0000000a, 0x00000170, 0x00000013, 0x0000017c, 0x00000014, 0x0000018c, 
    0x00000015, 0x0000019c, 0x00000016, 0x000001ac, 0x00000017, 0x000001bc, 
    0x0000002a, 0x000001c8, 0x00000037, 0x000001dc, 0x00000038, 0x000001ec, 
    0x00000039, 0x000001fc, 0x0000003a, 0x0000020c, 0x7263694d, 0x666f736f, 
    0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 
    0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x0000001f, 
    0x80000000, 0x900f0000, 0x0000001f, 0x80000005, 0x900f0007, 0x00000009, 
    0x80010000, 0x90e40000, 0xa0e40004, 0x00000009, 0x80020000, 0x90e40000, 
    0xa0e40005, 0x00000009, 0x80040000, 0x90e40000, 0xa0e40006, 0x00000009, 
    0x80080000, 0x90e40000, 0xa0e40007, 0x00000001, 0xc00f0000, 0x80e40000, 
    0x00000004, 0xc00f0001, 0x81aa0000, 0xa0ff0010, 0xa0000010, 0x00000002, 
    0xe00f0000, 0x90e40007, 0xa0a5005f, 0x00000002, 0xe00f0001, 0x90e40007, 
    0xa0a4005f, 0x00000002, 0xe00f0002, 0x90e40007, 0xa0a0005f, 0x00000002, 
    0xe00f0003, 0x90e40007, 0xa0a1005f, 0x0000ffff
};
#if 0
    vs_1_1

#line 9 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\vshtmp9\vshtmp9\vertexShader_HotSpotGlow_none_none_none_none_0_heightfog.tmp"
    dcl_position v0
    dcl_texcoord v7

#line 19
    dp4 r0.x, v0, c4
    dp4 r0.y, v0, c5
    dp4 r0.z, v0, c6
    dp4 r0.w, v0, c7
    mov oPos, r0

#line 33
    dp4 r1.z, v0, c44

#line 52
    add r2.xy, c2.wzzz, -r1.z

#line 57
    max r2.x, r2.x, c0.x
    rcp r2.z, r2.y
    mul r2.w, r2.x, r2.z

#line 68
    mul r2.w, r2.w, r0.z
    mad oFog, -r2.w, c16.w, c16.y

#line 88
    add oT0, v7, c95.yyzz
    add oT1, v7, c95.xyzz
    add oT2, v7, c95.xxzz
    add oT3, v7, c95.yxzz

// approximately 16 instruction slots used
#endif

 static unsigned int vertexShader_HotSpotGlow_none_none_none_none_0_heightfog[] =
{
    0xfffe0101, 0x0064fffe, 0x47554244, 0x00000028, 0x00000158, 0x00000000, 
    0x00000001, 0x000000c4, 0x00000012, 0x000000c8, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74687376, 0x5c39706d, 0x74687376, 0x5c39706d, 
    0x74726576, 0x68537865, 0x72656461, 0x746f485f, 0x746f7053, 0x776f6c47, 
    0x6e6f6e5f, 0x6f6e5f65, 0x6e5f656e, 0x5f656e6f, 0x656e6f6e, 0x685f305f, 
    0x68676965, 0x676f6674, 0x706d742e, 0xababab00, 0x00000028, 0x00000009, 
    0x00000198, 0x0000000a, 0x000001a4, 0x00000013, 0x000001b0, 0x00000014, 
    0x000001c0, 0x00000015, 0x000001d0, 0x00000016, 0x000001e0, 0x00000017, 
    0x000001f0, 0x00000021, 0x000001fc, 0x00000034, 0x0000020c, 0x00000039, 
    0x0000021c, 0x0000003b, 0x0000022c, 0x0000003c, 0x00000238, 0x00000044, 
    0x00000248, 0x00000046, 0x00000258, 0x00000058, 0x0000026c, 0x00000059, 
    0x0000027c, 0x0000005a, 0x0000028c, 0x0000005b, 0x0000029c, 0x7263694d, 
    0x666f736f, 0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 
    0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 
    0x0000001f, 0x80000000, 0x900f0000, 0x0000001f, 0x80000005, 0x900f0007, 
    0x00000009, 0x80010000, 0x90e40000, 0xa0e40004, 0x00000009, 0x80020000, 
    0x90e40000, 0xa0e40005, 0x00000009, 0x80040000, 0x90e40000, 0xa0e40006, 
    0x00000009, 0x80080000, 0x90e40000, 0xa0e40007, 0x00000001, 0xc00f0000, 
    0x80e40000, 0x00000009, 0x80040001, 0x90e40000, 0xa0e4002c, 0x00000002, 
    0x80030002, 0xa0ab0002, 0x81aa0001, 0x0000000b, 0x80010002, 0x80000002, 
    0xa0000000, 0x00000006, 0x80040002, 0x80550002, 0x00000005, 0x80080002, 
    0x80000002, 0x80aa0002, 0x00000005, 0x80080002, 0x80ff0002, 0x80aa0000, 
    0x00000004, 0xc00f0001, 0x81ff0002, 0xa0ff0010, 0xa0550010, 0x00000002, 
    0xe00f0000, 0x90e40007, 0xa0a5005f, 0x00000002, 0xe00f0001, 0x90e40007, 
    0xa0a4005f, 0x00000002, 0xe00f0002, 0x90e40007, 0xa0a0005f, 0x00000002, 
    0xe00f0003, 0x90e40007, 0xa0a1005f, 0x0000ffff
};
static PrecompiledShaderByteCode_t HotSpotGlow_vertex_shaders[] = {
{ 	vertexShader_HotSpotGlow_none_none_none_none_0_rangefog, sizeof( 	vertexShader_HotSpotGlow_none_none_none_none_0_rangefog ) },
{ 	vertexShader_HotSpotGlow_none_none_none_none_0_heightfog, sizeof( 	vertexShader_HotSpotGlow_none_none_none_none_0_heightfog ) },
};
struct HotSpotGlow_VertexShader_t : public PrecompiledShader_t
{
	HotSpotGlow_VertexShader_t()
	{
		m_nFlags = 0;
		m_nFlags |= SHADER_USES_HEIGHT_FOG;
		m_nFlags |= SHADER_USES_MODEL_VIEW_PROJ_MATRIX;
		m_nFlags |= SHADER_USES_MODEL_MATRIX;
		m_pByteCode = HotSpotGlow_vertex_shaders;
		m_pName = "HotSpotGlow";
		m_nShaderCount = 2;
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_VERTEX_SHADER, this );
	}
};
static HotSpotGlow_VertexShader_t HotSpotGlow_VertexShaderInstance;
