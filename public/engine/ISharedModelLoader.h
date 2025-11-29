// VXP

#ifndef ISHAREDMODELLOADER_H
#define ISHAREDMODELLOADER_H
#ifdef _WIN32
#pragma once
#endif

#include "interface.h"
#include "filesystem.h"
#include "studio.h"

#define ISHAREDMODELLOADER_INTERFACE_VERSION	"ISharedModelLoader001"

class ISharedModelLoader : public IBaseInterface
{
public:
	// Always use studioanimgrouphdr_t - it's available for all STUDIO_VERSION values
	// This allows loading v37 animation groups regardless of compile-time STUDIO_VERSION
	virtual studioanimgrouphdr_t	*LoadSharedModel( const char *path ) = 0;
	virtual void					InitFilesystem( IBaseFileSystem *filesystem ) = 0;
};

extern ISharedModelLoader *sharedmodelloader;


#endif // ISHAREDMODELLOADER_H
