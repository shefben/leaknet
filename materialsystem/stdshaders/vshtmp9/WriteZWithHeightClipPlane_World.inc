#if 0
    vs_1_1

#line 9 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\vshtmp9\vshtmp9\vertexShader_WriteZWithHeightClipPlane_World_none_none_none_none_0_rangefog.tmp"
    dcl_position v0

#line 20
    dp4 oPos.x, v0, c4
    dp4 oPos.y, v0, c5
    dp4 oPos.z, v0, c6
    dp4 oPos.w, v0, c7
    dp4 r0.x, v0, c42
    dp4 r0.y, v0, c43
    dp4 r0.z, v0, c44

#line 34
    mad oT0, -c37.x, r0.z, c37.y
    mov oD0, c0.y

// approximately 9 instruction slots used
#endif

 static unsigned int vertexShader_WriteZWithHeightClipPlane_World_none_none_none_none_0_rangefog[] =
{
    0xfffe0101, 0x0058fffe, 0x47554244, 0x00000028, 0x00000128, 0x00000000, 
    0x00000001, 0x000000d4, 0x0000000a, 0x000000d8, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74687376, 0x5c39706d, 0x74687376, 0x5c39706d, 
    0x74726576, 0x68537865, 0x72656461, 0x6972575f, 0x575a6574, 0x48687469, 
    0x68676965, 0x696c4374, 0x616c5070, 0x575f656e, 0x646c726f, 0x6e6f6e5f, 
    0x6f6e5f65, 0x6e5f656e, 0x5f656e6f, 0x656e6f6e, 0x725f305f, 0x65676e61, 
    0x2e676f66, 0x00706d74, 0x00000028, 0x00000009, 0x00000168, 0x00000014, 
    0x00000174, 0x00000015, 0x00000184, 0x00000016, 0x00000194, 0x00000017, 
    0x000001a4, 0x00000019, 0x000001b4, 0x0000001a, 0x000001c4, 0x0000001b, 
    0x000001d4, 0x00000022, 0x000001e4, 0x00000023, 0x000001f8, 0x7263694d, 
    0x666f736f, 0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 
    0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 
    0x0000001f, 0x80000000, 0x900f0000, 0x00000009, 0xc0010000, 0x90e40000, 
    0xa0e40004, 0x00000009, 0xc0020000, 0x90e40000, 0xa0e40005, 0x00000009, 
    0xc0040000, 0x90e40000, 0xa0e40006, 0x00000009, 0xc0080000, 0x90e40000, 
    0xa0e40007, 0x00000009, 0x80010000, 0x90e40000, 0xa0e4002a, 0x00000009, 
    0x80020000, 0x90e40000, 0xa0e4002b, 0x00000009, 0x80040000, 0x90e40000, 
    0xa0e4002c, 0x00000004, 0xe00f0000, 0xa1000025, 0x80aa0000, 0xa0550025, 
    0x00000001, 0xd00f0000, 0xa0550000, 0x0000ffff
};
#if 0
    vs_1_1

#line 9 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\vshtmp9\vshtmp9\vertexShader_WriteZWithHeightClipPlane_World_none_none_none_none_0_heightfog.tmp"
    dcl_position v0

#line 20
    dp4 oPos.x, v0, c4
    dp4 oPos.y, v0, c5
    dp4 oPos.z, v0, c6
    dp4 oPos.w, v0, c7
    dp4 r0.x, v0, c42
    dp4 r0.y, v0, c43
    dp4 r0.z, v0, c44

#line 34
    mad oT0, -c37.x, r0.z, c37.y
    mov oD0, c0.y

// approximately 9 instruction slots used
#endif

 static unsigned int vertexShader_WriteZWithHeightClipPlane_World_none_none_none_none_0_heightfog[] =
{
    0xfffe0101, 0x0059fffe, 0x47554244, 0x00000028, 0x0000012c, 0x00000000, 
    0x00000001, 0x000000d8, 0x0000000a, 0x000000dc, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74687376, 0x5c39706d, 0x74687376, 0x5c39706d, 
    0x74726576, 0x68537865, 0x72656461, 0x6972575f, 0x575a6574, 0x48687469, 
    0x68676965, 0x696c4374, 0x616c5070, 0x575f656e, 0x646c726f, 0x6e6f6e5f, 
    0x6f6e5f65, 0x6e5f656e, 0x5f656e6f, 0x656e6f6e, 0x685f305f, 0x68676965, 
    0x676f6674, 0x706d742e, 0xababab00, 0x00000028, 0x00000009, 0x0000016c, 
    0x00000014, 0x00000178, 0x00000015, 0x00000188, 0x00000016, 0x00000198, 
    0x00000017, 0x000001a8, 0x00000019, 0x000001b8, 0x0000001a, 0x000001c8, 
    0x0000001b, 0x000001d8, 0x00000022, 0x000001e8, 0x00000023, 0x000001fc, 
    0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 0x20395844, 0x64616853, 
    0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 
    0xab003635, 0x0000001f, 0x80000000, 0x900f0000, 0x00000009, 0xc0010000, 
    0x90e40000, 0xa0e40004, 0x00000009, 0xc0020000, 0x90e40000, 0xa0e40005, 
    0x00000009, 0xc0040000, 0x90e40000, 0xa0e40006, 0x00000009, 0xc0080000, 
    0x90e40000, 0xa0e40007, 0x00000009, 0x80010000, 0x90e40000, 0xa0e4002a, 
    0x00000009, 0x80020000, 0x90e40000, 0xa0e4002b, 0x00000009, 0x80040000, 
    0x90e40000, 0xa0e4002c, 0x00000004, 0xe00f0000, 0xa1000025, 0x80aa0000, 
    0xa0550025, 0x00000001, 0xd00f0000, 0xa0550000, 0x0000ffff
};
static PrecompiledShaderByteCode_t WriteZWithHeightClipPlane_World_vertex_shaders[] = {
{ 	vertexShader_WriteZWithHeightClipPlane_World_none_none_none_none_0_rangefog, sizeof( 	vertexShader_WriteZWithHeightClipPlane_World_none_none_none_none_0_rangefog ) },
{ 	vertexShader_WriteZWithHeightClipPlane_World_none_none_none_none_0_heightfog, sizeof( 	vertexShader_WriteZWithHeightClipPlane_World_none_none_none_none_0_heightfog ) },
};
struct WriteZWithHeightClipPlane_World_VertexShader_t : public PrecompiledShader_t
{
	WriteZWithHeightClipPlane_World_VertexShader_t()
	{
		m_nFlags = 0;
		m_nFlags |= SHADER_USES_HEIGHT_FOG;
		m_nFlags |= SHADER_USES_MODEL_VIEW_PROJ_MATRIX;
		m_nFlags |= SHADER_USES_MODEL_MATRIX;
		m_pByteCode = WriteZWithHeightClipPlane_World_vertex_shaders;
		m_pName = "WriteZWithHeightClipPlane_World";
		m_nShaderCount = 2;
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_VERTEX_SHADER, this );
	}
};
static WriteZWithHeightClipPlane_World_VertexShader_t WriteZWithHeightClipPlane_World_VertexShaderInstance;
