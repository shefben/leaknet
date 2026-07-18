#if 0
    ps_1_1

#line 2 "F:\development\steam\emulator_bot\LeakNet-rewrite\materialsystem\stdshaders\BufferClearObeyStencil_ps11.psh"
    mov r0, v0

// approximately 1 instruction slot used
#endif

static unsigned int pixelShader_BufferClearObeyStencil_ps11_0[] =
{
    0xffff0101, 0x0036fffe, 0x47554244, 0x00000028, 0x000000a0, 0x00000000, 
    0x00000001, 0x00000094, 0x00000001, 0x00000098, 0x00000000, 0x00000000, 
    0x00000000, 0x645c3a46, 0x6c657665, 0x656d706f, 0x735c746e, 0x6d616574, 
    0x756d655c, 0x6f74616c, 0x6f625f72, 0x654c5c74, 0x654e6b61, 0x65722d74, 
    0x74697277, 0x616d5c65, 0x69726574, 0x79736c61, 0x6d657473, 0x6474735c, 
    0x64616873, 0x5c737265, 0x66667542, 0x6c437265, 0x4f726165, 0x53796562, 
    0x636e6574, 0x705f6c69, 0x2e313173, 0x00687370, 0x00000028, 0x00000002, 
    0x000000e0, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 0x20395844, 
    0x64616853, 0x41207265, 0x6d657373, 0x72656c62, 0x322e3920, 0x35392e34, 
    0x36322e30, 0xab003635, 0x00000001, 0x800f0000, 0x90e40000, 0x0000ffff
};
static PrecompiledShaderByteCode_t BufferClearObeyStencil_ps11_pixel_shaders[1] = 
{
	{ pixelShader_BufferClearObeyStencil_ps11_0, sizeof( pixelShader_BufferClearObeyStencil_ps11_0 ) },
};
struct BufferClearObeyStencil_ps11PixelShader_t : public PrecompiledShader_t
{
	BufferClearObeyStencil_ps11PixelShader_t()
	{
		m_nFlags = SHADER_CUSTOM_ENUMERATION;
		m_pByteCode = BufferClearObeyStencil_ps11_pixel_shaders;
		m_nShaderCount = 1;
		m_pName = "BufferClearObeyStencil_ps11";
		GetShaderDLL()->InsertPrecompiledShader( PRECOMPILED_PIXEL_SHADER, this );
	}
};
static BufferClearObeyStencil_ps11PixelShader_t BufferClearObeyStencil_ps11_PixelShaderInstance;
