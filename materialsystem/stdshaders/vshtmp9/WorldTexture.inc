#if 0
    vs_1_1

#line 9 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\vshtmp9\vshtmp9\vertexShader_WorldTexture_none_none_none_none_0_rangefog.tmp"
    dcl_position v0
    dcl_texcoord v7

#line 15
    dp4 r0.x, v0, c8
    dp4 r0.y, v0, c9
    dp4 r0.z, v0, c10
    dp4 r0.w, v0, c11
    mov oPos, r0

#line 38
    mad oFog, -r0.z, c16.w, c16.x

#line 50
    mov oT0, v7

// approximately 7 instruction slots used
#endif

 static unsigned int vertexShader_WorldTexture_none_none_none_none_0_rangefog[] =
{
    0xfffe0101, 0x0052fffe, 0x47554244, 0x00000028, 0x00000110, 0x00000000, 
    0x00000001, 0x000000c4, 0x00000009, 0x000000c8, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74687376, 0x5c39706d, 0x74687376, 0x5c39706d, 
    0x74726576, 0x68537865, 0x72656461, 0x726f575f, 0x6554646c, 0x72757478, 
    0x6f6e5f65, 0x6e5f656e, 0x5f656e6f, 0x656e6f6e, 0x6e6f6e5f, 0x5f305f65, 
    0x676e6172, 0x676f6665, 0x706d742e, 0xababab00, 0x00000028, 0x00000009, 
    0x00000150, 0x0000000a, 0x0000015c, 0x0000000f, 0x00000168, 0x00000010, 
    0x00000178, 0x00000011, 0x00000188, 0x00000012, 0x00000198, 0x00000013, 
    0x000001a8, 0x00000026, 0x000001b4, 0x00000032, 0x000001c8, 0x7263694d, 
    0x666f736f, 0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 
    0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 
    0x0000001f, 0x80000000, 0x900f0000, 0x0000001f, 0x80000005, 0x900f0007, 
    0x00000009, 0x80010000, 0x90e40000, 0xa0e40008, 0x00000009, 0x80020000, 
    0x90e40000, 0xa0e40009, 0x00000009, 0x80040000, 0x90e40000, 0xa0e4000a, 
    0x00000009, 0x80080000, 0x90e40000, 0xa0e4000b, 0x00000001, 0xc00f0000, 
    0x80e40000, 0x00000004, 0xc00f0001, 0x81aa0000, 0xa0ff0010, 0xa0000010, 
    0x00000001, 0xe00f0000, 0x90e40007, 0x0000ffff
};
#if 0
    vs_1_1

#line 9 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\vshtmp9\vshtmp9\vertexShader_WorldTexture_none_none_none_none_0_heightfog.tmp"
    dcl_position v0
    dcl_texcoord v7

#line 15
    dp4 r0.x, v0, c8
    dp4 r0.y, v0, c9
    dp4 r0.z, v0, c10
    dp4 r0.w, v0, c11
    mov oPos, r0

#line 29
    dp4 r1.z, v0, c44

#line 48
    add r2.xy, c2.wzzz, -r1.z

#line 53
    max r2.x, r2.x, c0.x
    rcp r2.z, r2.y
    mul r2.w, r2.x, r2.z

#line 64
    mul r2.w, r2.w, r0.z
    mad oFog, -r2.w, c16.w, c16.y

#line 83
    mov oT0, v7

// approximately 13 instruction slots used
#endif

 static unsigned int vertexShader_WorldTexture_none_none_none_none_0_heightfog[] =
{
    0xfffe0101, 0x005efffe, 0x47554244, 0x00000028, 0x00000140, 0x00000000, 
    0x00000001, 0x000000c4, 0x0000000f, 0x000000c8, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74687376, 0x5c39706d, 0x74687376, 0x5c39706d, 
    0x74726576, 0x68537865, 0x72656461, 0x726f575f, 0x6554646c, 0x72757478, 
    0x6f6e5f65, 0x6e5f656e, 0x5f656e6f, 0x656e6f6e, 0x6e6f6e5f, 0x5f305f65, 
    0x67696568, 0x6f667468, 0x6d742e67, 0xabab0070, 0x00000028, 0x00000009, 
    0x00000180, 0x0000000a, 0x0000018c, 0x0000000f, 0x00000198, 0x00000010, 
    0x000001a8, 0x00000011, 0x000001b8, 0x00000012, 0x000001c8, 0x00000013, 
    0x000001d8, 0x0000001d, 0x000001e4, 0x00000030, 0x000001f4, 0x00000035, 
    0x00000204, 0x00000037, 0x00000214, 0x00000038, 0x00000220, 0x00000040, 
    0x00000230, 0x00000042, 0x00000240, 0x00000053, 0x00000254, 0x7263694d, 
    0x666f736f, 0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 
    0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 
    0x0000001f, 0x80000000, 0x900f0000, 0x0000001f, 0x80000005, 0x900f0007, 
    0x00000009, 0x80010000, 0x90e40000, 0xa0e40008, 0x00000009, 0x80020000, 
    0x90e40000, 0xa0e40009, 0x00000009, 0x80040000, 0x90e40000, 0xa0e4000a, 
    0x00000009, 0x80080000, 0x90e40000, 0xa0e4000b, 0x00000001, 0xc00f0000, 
    0x80e40000, 0x00000009, 0x80040001, 0x90e40000, 0xa0e4002c, 0x00000002, 
    0x80030002, 0xa0ab0002, 0x81aa0001, 0x0000000b, 0x80010002, 0x80000002, 
    0xa0000000, 0x00000006, 0x80040002, 0x80550002, 0x00000005, 0x80080002, 
    0x80000002, 0x80aa0002, 0x00000005, 0x80080002, 0x80ff0002, 0x80aa0000, 
    0x00000004, 0xc00f0001, 0x81ff0002, 0xa0ff0010, 0xa0550010, 0x00000001, 
    0xe00f0000, 0x90e40007, 0x0000ffff
};
static PrecompiledShaderByteCode_t WorldTexture_vertex_shaders[] = {
{ 	vertexShader_WorldTexture_none_none_none_none_0_rangefog, sizeof( 	vertexShader_WorldTexture_none_none_none_none_0_rangefog ) },
{ 	vertexShader_WorldTexture_none_none_none_none_0_heightfog, sizeof( 	vertexShader_WorldTexture_none_none_none_none_0_heightfog ) },
};
struct WorldTexture_VertexShader_t : public PrecompiledShader_t
{
	WorldTexture_VertexShader_t()
	{
		m_nFlags = 0;
		m_nFlags |= SHADER_USES_HEIGHT_FOG;
		m_nFlags |= SHADER_USES_MODEL_MATRIX;
		m_pByteCode = WorldTexture_vertex_shaders;
		m_pName = "WorldTexture";
		m_nShaderCount = 2;
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_VERTEX_SHADER, this );
	}
};
static WorldTexture_VertexShader_t WorldTexture_VertexShaderInstance;
