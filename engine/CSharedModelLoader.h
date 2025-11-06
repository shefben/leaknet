// VXP

#ifndef CSHAREDMODELLOADER_H
#define CSHAREDMODELLOADER_H
#ifdef _WIN32
#pragma once
#endif

#include "engine/ISharedModelLoader.h"
//#include "utlrbtree.h"

#if STUDIO_VERSION == 37
struct sharedmodelloader_t
{
	char					path[MAX_PATH];
	studioanimgrouphdr_t	*header;
};
#endif

class CSharedModelLoader : public ISharedModelLoader
{
public:
	CSharedModelLoader();

	void					InitFilesystem( IBaseFileSystem *filesystem );
#if STUDIO_VERSION == 37
	studioanimgrouphdr_t	*LoadSharedModel( const char *path );
#else
	void					*LoadSharedModel( const char *path );
#endif

private:
#if STUDIO_VERSION == 37
	IBaseFileSystem			*m_pFilesystem;
	sharedmodelloader_t		cachedata;
#endif
};


#endif // CSHAREDMODELLOADER_H
