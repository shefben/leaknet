// VXP

#include "CSharedModelLoader.h"
#include "modelgen.h"
#include <vstdlib/strtools.h>

#define TYPICAL_SHAREDANIMGROUP_FILESIZE 14 * 1024 * 1024 // Just about 1384588 bytes

EXPOSE_SINGLE_INTERFACE( CSharedModelLoader, ISharedModelLoader, ISHAREDMODELLOADER_INTERFACE_VERSION );

#if STUDIO_VERSION == 37
CSharedModelLoader::CSharedModelLoader()
{
	cachedata.header = (studioanimgrouphdr_t *)malloc( TYPICAL_SHAREDANIMGROUP_FILESIZE );
	cachedata.path[0] = '\0';

	memset( cachedata.header, 0, TYPICAL_SHAREDANIMGROUP_FILESIZE );
}

void CSharedModelLoader::InitFilesystem( IBaseFileSystem *filesystem )
{
	m_pFilesystem = filesystem;
}

studioanimgrouphdr_t *CSharedModelLoader::LoadSharedModel( const char *path )
{
	if ( Q_strcmp( cachedata.path, path ) == 0 )
		return cachedata.header;

	if ( m_pFilesystem == NULL )
		return NULL;

	FileHandle_t pFileHandle = m_pFilesystem->Open( path, "rb" );
	if ( pFileHandle == NULL )
	{
		Warning( "CSharedModelLoader: Cannot open %s\n", path );
		return NULL;
	}

	int nSize = m_pFilesystem->Size( pFileHandle );
	if ( nSize <= 0 )
	{
		Warning( "CSharedModelLoader: Cannot read %s\n", path );
		m_pFilesystem->Close( pFileHandle );
		return NULL;
	}

	if ( nSize > TYPICAL_SHAREDANIMGROUP_FILESIZE )
	{
		Warning( "CSharedModelLoader: %s is bigger than original file. Tell a programmer\n", path );
		m_pFilesystem->Close( pFileHandle );
		return NULL;
	}

	studioanimgrouphdr_t *sharedModelMemory = (studioanimgrouphdr_t *)malloc( nSize );
	int readSize = m_pFilesystem->Read( sharedModelMemory, nSize, pFileHandle );
	m_pFilesystem->Close( pFileHandle );
	if ( readSize != nSize )
	{
		Warning( "CSharedModelLoader: %s is corrupted\n", path );
		return NULL;
	}

	if ( sharedModelMemory->id != IDSTUDIOANIMGROUPHEADER )
	{
		Warning( "CSharedModelLoader: %s is not a valid animgroup file\n", path );
		return NULL;
	}

	if ( sharedModelMemory->length != nSize )
	{
		Warning( "CSharedModelLoader: %s has wrong header file\n", path );
		return NULL;
	}

	sharedModelMemory->pAnimdesc(0); // For debugger

	cachedata.header = sharedModelMemory;
	Q_strcpy( cachedata.path, path );

	return cachedata.header;
}
#else
CSharedModelLoader::CSharedModelLoader()
{
}

void CSharedModelLoader::InitFilesystem( IBaseFileSystem *filesystem )
{
}

void *CSharedModelLoader::LoadSharedModel( const char *path )
{
	return NULL;
}
#endif
