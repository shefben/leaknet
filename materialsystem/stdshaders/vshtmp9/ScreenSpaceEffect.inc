#if 0
    vs_1_1

#line 22 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\vshtmp9\vshtmp9\vertexShader_ScreenSpaceEffect_none_none_none_none_0_rangefog.tmp"
    dcl_position v0
    dcl_texcoord v7

#line 27
    mov oPos.xyz, v0.xyzz
    mov oPos.w, c0.y

#line 32
    mov oT0, v7

// approximately 3 instruction slots used
#endif

 static unsigned int vertexShader_ScreenSpaceEffect_none_none_none_none_0_rangefog[] =
{
    0xfffe0101, 0x004bfffe, 0x47554244, 0x00000028, 0x000000f4, 0x00000000, 
    0x00000001, 0x000000c8, 0x00000005, 0x000000cc, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74687376, 0x5c39706d, 0x74687376, 0x5c39706d, 
    0x74726576, 0x68537865, 0x72656461, 0x7263535f, 0x536e6565, 0x65636170, 
    0x65666645, 0x6e5f7463, 0x5f656e6f, 0x656e6f6e, 0x6e6f6e5f, 0x6f6e5f65, 
    0x305f656e, 0x6e61725f, 0x6f666567, 0x6d742e67, 0xabab0070, 0x00000028, 
    0x00000016, 0x00000134, 0x00000017, 0x00000140, 0x0000001b, 0x0000014c, 
    0x0000001c, 0x00000158, 0x00000020, 0x00000164, 0x7263694d, 0x666f736f, 
    0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 
    0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x0000001f, 
    0x80000000, 0x900f0000, 0x0000001f, 0x80000005, 0x900f0007, 0x00000001, 
    0xc0070000, 0x90a40000, 0x00000001, 0xc0080000, 0xa0550000, 0x00000001, 
    0xe00f0000, 0x90e40007, 0x0000ffff
};
#if 0
    vs_1_1

#line 22 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\vshtmp9\vshtmp9\vertexShader_ScreenSpaceEffect_none_none_none_none_0_heightfog.tmp"
    dcl_position v0
    dcl_texcoord v7

#line 27
    mov oPos.xyz, v0.xyzz
    mov oPos.w, c0.y

#line 32
    mov oT0, v7

// approximately 3 instruction slots used
#endif

 static unsigned int vertexShader_ScreenSpaceEffect_none_none_none_none_0_heightfog[] =
{
    0xfffe0101, 0x004bfffe, 0x47554244, 0x00000028, 0x000000f4, 0x00000000, 
    0x00000001, 0x000000c8, 0x00000005, 0x000000cc, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74687376, 0x5c39706d, 0x74687376, 0x5c39706d, 
    0x74726576, 0x68537865, 0x72656461, 0x7263535f, 0x536e6565, 0x65636170, 
    0x65666645, 0x6e5f7463, 0x5f656e6f, 0x656e6f6e, 0x6e6f6e5f, 0x6f6e5f65, 
    0x305f656e, 0x6965685f, 0x66746867, 0x742e676f, 0xab00706d, 0x00000028, 
    0x00000016, 0x00000134, 0x00000017, 0x00000140, 0x0000001b, 0x0000014c, 
    0x0000001c, 0x00000158, 0x00000020, 0x00000164, 0x7263694d, 0x666f736f, 
    0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 0x6d657373, 
    0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 0x0000001f, 
    0x80000000, 0x900f0000, 0x0000001f, 0x80000005, 0x900f0007, 0x00000001, 
    0xc0070000, 0x90a40000, 0x00000001, 0xc0080000, 0xa0550000, 0x00000001, 
    0xe00f0000, 0x90e40007, 0x0000ffff
};
static PrecompiledShaderByteCode_t ScreenSpaceEffect_vertex_shaders[] = {
{ 	vertexShader_ScreenSpaceEffect_none_none_none_none_0_rangefog, sizeof( 	vertexShader_ScreenSpaceEffect_none_none_none_none_0_rangefog ) },
{ 	vertexShader_ScreenSpaceEffect_none_none_none_none_0_heightfog, sizeof( 	vertexShader_ScreenSpaceEffect_none_none_none_none_0_heightfog ) },
};
struct ScreenSpaceEffect_VertexShader_t : public PrecompiledShader_t
{
	ScreenSpaceEffect_VertexShader_t()
	{
		m_nFlags = 0;
		m_nFlags |= SHADER_USES_HEIGHT_FOG;
		m_pByteCode = ScreenSpaceEffect_vertex_shaders;
		m_pName = "ScreenSpaceEffect";
		m_nShaderCount = 2;
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_VERTEX_SHADER, this );
	}
};
static ScreenSpaceEffect_VertexShader_t ScreenSpaceEffect_VertexShaderInstance;
