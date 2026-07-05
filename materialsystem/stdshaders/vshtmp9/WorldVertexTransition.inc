#if 0
    vs_1_1

#line 9 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\vshtmp9\vshtmp9\vertexShader_WorldVertexTransition_none_none_none_none_0_rangefog.tmp"
    dcl_position v0
    dcl_texcoord v7
    dcl_texcoord1 v8

#line 16
    dp4 r0.x, v0, c4
    dp4 r0.y, v0, c5
    dp4 r0.z, v0, c6
    dp4 r0.w, v0, c7
    mov oPos, r0

#line 27
    dp4 r1.z, v0, c44

#line 38
    mad oFog, -r0.z, c16.w, c16.x

#line 45
    mad oT3, -c37.x, r1.z, c37.y

#line 57
    dp4 oT0.x, v7, c90
    dp4 oT0.y, v7, c91
    dp4 oT1.x, v7, c92
    dp4 oT1.y, v7, c93
    mov oT2, v8

// approximately 13 instruction slots used
#endif

 static unsigned int vertexShader_WorldVertexTransition_none_none_none_none_0_rangefog[] =
{
    0xfffe0101, 0x0062fffe, 0x47554244, 0x00000028, 0x00000150, 0x00000000, 
    0x00000001, 0x000000cc, 0x00000010, 0x000000d0, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74687376, 0x5c39706d, 0x74687376, 0x5c39706d, 
    0x74726576, 0x68537865, 0x72656461, 0x726f575f, 0x6556646c, 0x78657472, 
    0x6e617254, 0x69746973, 0x6e5f6e6f, 0x5f656e6f, 0x656e6f6e, 0x6e6f6e5f, 
    0x6f6e5f65, 0x305f656e, 0x6e61725f, 0x6f666567, 0x6d742e67, 0xabab0070, 
    0x00000028, 0x00000009, 0x00000190, 0x0000000a, 0x0000019c, 0x0000000b, 
    0x000001a8, 0x00000010, 0x000001b4, 0x00000011, 0x000001c4, 0x00000012, 
    0x000001d4, 0x00000013, 0x000001e4, 0x00000014, 0x000001f4, 0x0000001b, 
    0x00000200, 0x00000026, 0x00000210, 0x0000002d, 0x00000224, 0x00000039, 
    0x00000238, 0x0000003a, 0x00000248, 0x0000003b, 0x00000258, 0x0000003c, 
    0x00000268, 0x0000003e, 0x00000278, 0x7263694d, 0x666f736f, 0x52282074, 
    0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 
    0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x0000001f, 0x80000000, 
    0x900f0000, 0x0000001f, 0x80000005, 0x900f0007, 0x0000001f, 0x80010005, 
    0x900f0008, 0x00000009, 0x80010000, 0x90e40000, 0xa0e40004, 0x00000009, 
    0x80020000, 0x90e40000, 0xa0e40005, 0x00000009, 0x80040000, 0x90e40000, 
    0xa0e40006, 0x00000009, 0x80080000, 0x90e40000, 0xa0e40007, 0x00000001, 
    0xc00f0000, 0x80e40000, 0x00000009, 0x80040001, 0x90e40000, 0xa0e4002c, 
    0x00000004, 0xc00f0001, 0x81aa0000, 0xa0ff0010, 0xa0000010, 0x00000004, 
    0xe00f0003, 0xa1000025, 0x80aa0001, 0xa0550025, 0x00000009, 0xe0010000, 
    0x90e40007, 0xa0e4005a, 0x00000009, 0xe0020000, 0x90e40007, 0xa0e4005b, 
    0x00000009, 0xe0010001, 0x90e40007, 0xa0e4005c, 0x00000009, 0xe0020001, 
    0x90e40007, 0xa0e4005d, 0x00000001, 0xe00f0002, 0x90e40008, 0x0000ffff
};
#if 0
    vs_1_1

#line 9 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\vshtmp9\vshtmp9\vertexShader_WorldVertexTransition_none_none_none_none_0_heightfog.tmp"
    dcl_position v0
    dcl_texcoord v7
    dcl_texcoord1 v8

#line 16
    dp4 r0.x, v0, c4
    dp4 r0.y, v0, c5
    dp4 r0.z, v0, c6
    dp4 r0.w, v0, c7
    mov oPos, r0

#line 27
    dp4 r1.z, v0, c44

#line 46
    add r2.xy, c2.wzzz, -r1.z

#line 51
    max r2.x, r2.x, c0.x
    rcp r2.z, r2.y
    mul r2.w, r2.x, r2.z

#line 62
    mul r2.w, r2.w, r0.z
    mad oFog, -r2.w, c16.w, c16.y

#line 76
    mad oT3, -c37.x, r1.z, c37.y

#line 88
    dp4 oT0.x, v7, c90
    dp4 oT0.y, v7, c91
    dp4 oT1.x, v7, c92
    dp4 oT1.y, v7, c93
    mov oT2, v8

// approximately 18 instruction slots used
#endif

 static unsigned int vertexShader_WorldVertexTransition_none_none_none_none_0_heightfog[] =
{
    0xfffe0101, 0x006cfffe, 0x47554244, 0x00000028, 0x00000178, 0x00000000, 
    0x00000001, 0x000000cc, 0x00000015, 0x000000d0, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74687376, 0x5c39706d, 0x74687376, 0x5c39706d, 
    0x74726576, 0x68537865, 0x72656461, 0x726f575f, 0x6556646c, 0x78657472, 
    0x6e617254, 0x69746973, 0x6e5f6e6f, 0x5f656e6f, 0x656e6f6e, 0x6e6f6e5f, 
    0x6f6e5f65, 0x305f656e, 0x6965685f, 0x66746867, 0x742e676f, 0xab00706d, 
    0x00000028, 0x00000009, 0x000001b8, 0x0000000a, 0x000001c4, 0x0000000b, 
    0x000001d0, 0x00000010, 0x000001dc, 0x00000011, 0x000001ec, 0x00000012, 
    0x000001fc, 0x00000013, 0x0000020c, 0x00000014, 0x0000021c, 0x0000001b, 
    0x00000228, 0x0000002e, 0x00000238, 0x00000033, 0x00000248, 0x00000035, 
    0x00000258, 0x00000036, 0x00000264, 0x0000003e, 0x00000274, 0x00000040, 
    0x00000284, 0x0000004c, 0x00000298, 0x00000058, 0x000002ac, 0x00000059, 
    0x000002bc, 0x0000005a, 0x000002cc, 0x0000005b, 0x000002dc, 0x0000005d, 
    0x000002ec, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 0x20395844, 
    0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 
    0x36322e30, 0xab003635, 0x0000001f, 0x80000000, 0x900f0000, 0x0000001f, 
    0x80000005, 0x900f0007, 0x0000001f, 0x80010005, 0x900f0008, 0x00000009, 
    0x80010000, 0x90e40000, 0xa0e40004, 0x00000009, 0x80020000, 0x90e40000, 
    0xa0e40005, 0x00000009, 0x80040000, 0x90e40000, 0xa0e40006, 0x00000009, 
    0x80080000, 0x90e40000, 0xa0e40007, 0x00000001, 0xc00f0000, 0x80e40000, 
    0x00000009, 0x80040001, 0x90e40000, 0xa0e4002c, 0x00000002, 0x80030002, 
    0xa0ab0002, 0x81aa0001, 0x0000000b, 0x80010002, 0x80000002, 0xa0000000, 
    0x00000006, 0x80040002, 0x80550002, 0x00000005, 0x80080002, 0x80000002, 
    0x80aa0002, 0x00000005, 0x80080002, 0x80ff0002, 0x80aa0000, 0x00000004, 
    0xc00f0001, 0x81ff0002, 0xa0ff0010, 0xa0550010, 0x00000004, 0xe00f0003, 
    0xa1000025, 0x80aa0001, 0xa0550025, 0x00000009, 0xe0010000, 0x90e40007, 
    0xa0e4005a, 0x00000009, 0xe0020000, 0x90e40007, 0xa0e4005b, 0x00000009, 
    0xe0010001, 0x90e40007, 0xa0e4005c, 0x00000009, 0xe0020001, 0x90e40007, 
    0xa0e4005d, 0x00000001, 0xe00f0002, 0x90e40008, 0x0000ffff
};
static PrecompiledShaderByteCode_t WorldVertexTransition_vertex_shaders[] = {
{ 	vertexShader_WorldVertexTransition_none_none_none_none_0_rangefog, sizeof( 	vertexShader_WorldVertexTransition_none_none_none_none_0_rangefog ) },
{ 	vertexShader_WorldVertexTransition_none_none_none_none_0_heightfog, sizeof( 	vertexShader_WorldVertexTransition_none_none_none_none_0_heightfog ) },
};
struct WorldVertexTransition_VertexShader_t : public PrecompiledShader_t
{
	WorldVertexTransition_VertexShader_t()
	{
		m_nFlags = 0;
		m_nFlags |= SHADER_USES_HEIGHT_FOG;
		m_nFlags |= SHADER_USES_MODEL_VIEW_PROJ_MATRIX;
		m_nFlags |= SHADER_USES_MODEL_MATRIX;
		m_pByteCode = WorldVertexTransition_vertex_shaders;
		m_pName = "WorldVertexTransition";
		m_nShaderCount = 2;
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_VERTEX_SHADER, this );
	}
};
static WorldVertexTransition_VertexShader_t WorldVertexTransition_VertexShaderInstance;
