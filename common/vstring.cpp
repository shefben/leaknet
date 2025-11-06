
#include <string.h>
#include <malloc.h>
#include "vstring.h"
#include "basetypes.h"


static char *g_DefaultVString = "";



VString::VString()
{
	m_pStr = g_DefaultVString;
}


VString::~VString()
{
	FreeString();
}


int VString::Strcmp(const char *pStr)
{
	return Q_strcmp(m_pStr, pStr);
}


int VString::Stricmp(const char *pStr)
{
	return Q_stricmp(m_pStr, pStr);
}


bool VString::CopyString(const char *pStr)
{
	FreeString();

	m_pStr = (char*)malloc(Q_strlen(pStr)+1);
	if(!m_pStr)
		return false;

	Q_strcpy(m_pStr, pStr);
	return true;
}


void VString::FreeString()
{
	if(m_pStr != g_DefaultVString)
		free(m_pStr);
}


