// VXP

#ifndef CSHAREDMODELLOADER_H
#define CSHAREDMODELLOADER_H
#ifdef _WIN32
#pragma once
#endif

#include "engine/ISharedModelLoader.h"
//#include "utlrbtree.h"

// Always define - studioanimgrouphdr_t is available for all STUDIO_VERSION values
struct sharedmodelloader_t
{
	char					path[MAX_PATH];
	studioanimgrouphdr_t	*header;
};

class CSharedModelLoader : public ISharedModelLoader
{
public:
	CSharedModelLoader();

	void					InitFilesystem( IBaseFileSystem *filesystem );
	studioanimgrouphdr_t	*LoadSharedModel( const char *path );

private:
	IBaseFileSystem			*m_pFilesystem;
	sharedmodelloader_t		cachedata;
};


#endif // CSHAREDMODELLOADER_H
