#if 0
    vs_1_1

#line 9 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\vshtmp9\vshtmp9\vertexShader_LightmappedGeneric_LightingOnly_none_none_none_none_0_rangefog.tmp"
    dcl_position v0
    dcl_texcoord1 v8

#line 21
    dp4 r0.x, v0, c4
    dp4 r0.y, v0, c5
    dp4 r0.z, v0, c6
    dp4 r0.w, v0, c7
    mov oPos, r0

#line 44
    mad oFog, -r0.z, c16.w, c16.x

#line 53
    mov oT0, c0.x
    mov oT1, v8
    mov oD0, c0.y

// approximately 9 instruction slots used
#endif

 static unsigned int vertexShader_LightmappedGeneric_LightingOnly_none_none_none_none_0_rangefog[] =
{
    0xfffe0101, 0x005afffe, 0x47554244, 0x00000028, 0x00000130, 0x00000000, 
    0x00000001, 0x000000d4, 0x0000000b, 0x000000d8, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74687376, 0x5c39706d, 0x74687376, 0x5c39706d, 
    0x74726576, 0x68537865, 0x72656461, 0x67694c5f, 0x616d7468, 0x64657070, 
    0x656e6547, 0x5f636972, 0x6867694c, 0x676e6974, 0x796c6e4f, 0x6e6f6e5f, 
    0x6f6e5f65, 0x6e5f656e, 0x5f656e6f, 0x656e6f6e, 0x725f305f, 0x65676e61, 
    0x2e676f66, 0x00706d74, 0x00000028, 0x00000009, 0x00000170, 0x0000000a, 
    0x0000017c, 0x00000015, 0x00000188, 0x00000016, 0x00000198, 0x00000017, 
    0x000001a8, 0x00000018, 0x000001b8, 0x00000019, 0x000001c8, 0x0000002c, 
    0x000001d4, 0x00000035, 0x000001e8, 0x00000037, 0x000001f4, 0x00000038, 
    0x00000200, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 0x20395844, 
    0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 
    0x36322e30, 0xab003635, 0x0000001f, 0x80000000, 0x900f0000, 0x0000001f, 
    0x80010005, 0x900f0008, 0x00000009, 0x80010000, 0x90e40000, 0xa0e40004, 
    0x00000009, 0x80020000, 0x90e40000, 0xa0e40005, 0x00000009, 0x80040000, 
    0x90e40000, 0xa0e40006, 0x00000009, 0x80080000, 0x90e40000, 0xa0e40007, 
    0x00000001, 0xc00f0000, 0x80e40000, 0x00000004, 0xc00f0001, 0x81aa0000, 
    0xa0ff0010, 0xa0000010, 0x00000001, 0xe00f0000, 0xa0000000, 0x00000001, 
    0xe00f0001, 0x90e40008, 0x00000001, 0xd00f0000, 0xa0550000, 0x0000ffff
};
#if 0
    vs_1_1

#line 9 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\vshtmp9\vshtmp9\vertexShader_LightmappedGeneric_LightingOnly_none_none_none_none_0_heightfog.tmp"
    dcl_position v0
    dcl_texcoord1 v8

#line 21
    dp4 r0.x, v0, c4
    dp4 r0.y, v0, c5
    dp4 r0.z, v0, c6
    dp4 r0.w, v0, c7
    mov oPos, r0

#line 35
    dp4 r1.z, v0, c44

#line 54
    add r2.xy, c2.wzzz, -r1.z

#line 59
    max r2.x, r2.x, c0.x
    rcp r2.z, r2.y
    mul r2.w, r2.x, r2.z

#line 70
    mul r2.w, r2.w, r0.z
    mad oFog, -r2.w, c16.w, c16.y

#line 86
    mov oT0, c0.x
    mov oT1, v8
    mov oD0, c0.y

// approximately 15 instruction slots used
#endif

 static unsigned int vertexShader_LightmappedGeneric_LightingOnly_none_none_none_none_0_heightfog[] =
{
    0xfffe0101, 0x0067fffe, 0x47554244, 0x00000028, 0x00000164, 0x00000000, 
    0x00000001, 0x000000d8, 0x00000011, 0x000000dc, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74687376, 0x5c39706d, 0x74687376, 0x5c39706d, 
    0x74726576, 0x68537865, 0x72656461, 0x67694c5f, 0x616d7468, 0x64657070, 
    0x656e6547, 0x5f636972, 0x6867694c, 0x676e6974, 0x796c6e4f, 0x6e6f6e5f, 
    0x6f6e5f65, 0x6e5f656e, 0x5f656e6f, 0x656e6f6e, 0x685f305f, 0x68676965, 
    0x676f6674, 0x706d742e, 0xababab00, 0x00000028, 0x00000009, 0x000001a4, 
    0x0000000a, 0x000001b0, 0x00000015, 0x000001bc, 0x00000016, 0x000001cc, 
    0x00000017, 0x000001dc, 0x00000018, 0x000001ec, 0x00000019, 0x000001fc, 
    0x00000023, 0x00000208, 0x00000036, 0x00000218, 0x0000003b, 0x00000228, 
    0x0000003d, 0x00000238, 0x0000003e, 0x00000244, 0x00000046, 0x00000254, 
    0x00000048, 0x00000264, 0x00000056, 0x00000278, 0x00000058, 0x00000284, 
    0x00000059, 0x00000290, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 
    0x20395844, 0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 
    0x35392e34, 0x36322e30, 0xab003635, 0x0000001f, 0x80000000, 0x900f0000, 
    0x0000001f, 0x80010005, 0x900f0008, 0x00000009, 0x80010000, 0x90e40000, 
    0xa0e40004, 0x00000009, 0x80020000, 0x90e40000, 0xa0e40005, 0x00000009, 
    0x80040000, 0x90e40000, 0xa0e40006, 0x00000009, 0x80080000, 0x90e40000, 
    0xa0e40007, 0x00000001, 0xc00f0000, 0x80e40000, 0x00000009, 0x80040001, 
    0x90e40000, 0xa0e4002c, 0x00000002, 0x80030002, 0xa0ab0002, 0x81aa0001, 
    0x0000000b, 0x80010002, 0x80000002, 0xa0000000, 0x00000006, 0x80040002, 
    0x80550002, 0x00000005, 0x80080002, 0x80000002, 0x80aa0002, 0x00000005, 
    0x80080002, 0x80ff0002, 0x80aa0000, 0x00000004, 0xc00f0001, 0x81ff0002, 
    0xa0ff0010, 0xa0550010, 0x00000001, 0xe00f0000, 0xa0000000, 0x00000001, 
    0xe00f0001, 0x90e40008, 0x00000001, 0xd00f0000, 0xa0550000, 0x0000ffff
};
static PrecompiledShaderByteCode_t LightmappedGeneric_LightingOnly_vertex_shaders[] = {
{ 	vertexShader_LightmappedGeneric_LightingOnly_none_none_none_none_0_rangefog, sizeof( 	vertexShader_LightmappedGeneric_LightingOnly_none_none_none_none_0_rangefog ) },
{ 	vertexShader_LightmappedGeneric_LightingOnly_none_none_none_none_0_heightfog, sizeof( 	vertexShader_LightmappedGeneric_LightingOnly_none_none_none_none_0_heightfog ) },
};
struct LightmappedGeneric_LightingOnly_VertexShader_t : public PrecompiledShader_t
{
	LightmappedGeneric_LightingOnly_VertexShader_t()
	{
		m_nFlags = 0;
		m_nFlags |= SHADER_USES_HEIGHT_FOG;
		m_nFlags |= SHADER_USES_MODEL_VIEW_PROJ_MATRIX;
		m_nFlags |= SHADER_USES_MODEL_MATRIX;
		m_pByteCode = LightmappedGeneric_LightingOnly_vertex_shaders;
		m_pName = "LightmappedGeneric_LightingOnly";
		m_nShaderCount = 2;
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_VERTEX_SHADER, this );
	}
};
static LightmappedGeneric_LightingOnly_VertexShader_t LightmappedGeneric_LightingOnly_VertexShaderInstance;
