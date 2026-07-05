#if 0
    ps_1_1

#line 7 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\vertexlitgeneric_lightingonly_overbright2.psh"
    mov r0, v0

// approximately 1 instruction slot used
#endif

static unsigned int pixelShader_vertexlitgeneric_lightingonly_overbright2_0[] =
{
    0xffff0101, 0x003afffe, 0x47554244, 0x00000028, 0x000000b0, 0x00000000, 
    0x00000001, 0x000000a4, 0x00000001, 0x000000a8, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74726576, 0x696c7865, 0x6e656774, 0x63697265, 
    0x67696c5f, 0x6e697468, 0x6c6e6f67, 0x766f5f79, 0x72627265, 0x74686769, 
    0x73702e32, 0xabab0068, 0x00000028, 0x00000007, 0x000000f0, 0x7263694d, 
    0x666f736f, 0x52282074, 0x33442029, 0x20395844, 0x64616853, 0x41207265, 
    0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 0x36322e30, 0xab003635, 
    0x00000001, 0x800f0000, 0x90e40000, 0x0000ffff
};
#if 0
    ps_1_1

#line 5 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\vertexlitgeneric_lightingonly_overbright2.psh"
    texkill t3
    mov r0, v0

// approximately 2 instruction slots used
#endif

static unsigned int pixelShader_vertexlitgeneric_lightingonly_overbright2_1[] =
{
    0xffff0101, 0x003cfffe, 0x47554244, 0x00000028, 0x000000b8, 0x00000000, 
    0x00000001, 0x000000a4, 0x00000002, 0x000000a8, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x74726576, 0x696c7865, 0x6e656774, 0x63697265, 
    0x67696c5f, 0x6e697468, 0x6c6e6f67, 0x766f5f79, 0x72627265, 0x74686769, 
    0x73702e32, 0xabab0068, 0x00000028, 0x00000005, 0x000000f8, 0x00000007, 
    0x00000100, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 0x20395844, 
    0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 
    0x36322e30, 0xab003635, 0x00000041, 0xb00f0003, 0x00000001, 0x800f0000, 
    0x90e40000, 0x0000ffff
};
static PrecompiledShaderByteCode_t vertexlitgeneric_lightingonly_overbright2_pixel_shaders[2] = 
{
	{ pixelShader_vertexlitgeneric_lightingonly_overbright2_0, sizeof( pixelShader_vertexlitgeneric_lightingonly_overbright2_0 ) },
	{ pixelShader_vertexlitgeneric_lightingonly_overbright2_1, sizeof( pixelShader_vertexlitgeneric_lightingonly_overbright2_1 ) },
};
struct vertexlitgeneric_lightingonly_overbright2PixelShader_t : public PrecompiledShader_t
{
	vertexlitgeneric_lightingonly_overbright2PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = vertexlitgeneric_lightingonly_overbright2_pixel_shaders;
		m_nShaderCount = 2;
		m_pName = "vertexlitgeneric_lightingonly_overbright2";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static vertexlitgeneric_lightingonly_overbright2PixelShader_t vertexlitgeneric_lightingonly_overbright2_PixelShaderInstance;
