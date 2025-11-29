//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#include "quakedef.h"
#include "networkstringtable.h"
#include "networkstringtableitem.h"
#include "inetworkstringtableclient.h"
#include "networkstringtablecontainerclient.h"
#include "utlvector.h"
#include "cdll_engine_int.h"
#include "precache.h"
#include "utlsymbol.h"
#include "utlrbtree.h"
#include "utlbuffer.h"

// 2007 Source Engine compatibility
#ifndef COPY_ALL_CHARACTERS
#define COPY_ALL_CHARACTERS -1
#endif

// Substring bits for history entries
#define SUBSTRING_BITS	5

// String history entry for compression
struct StringHistoryEntry
{
	char string[ (1<<SUBSTRING_BITS) ];
};

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *object - 
//			stringTable - 
//			stringNumber - 
//			*newString - 
//			*newData - 
//-----------------------------------------------------------------------------
void Callback_ModelChanged( void *object, TABLEID stringTable, int stringNumber, char const *newString, void const *newData )
{
	if ( stringTable == cl.GetModelPrecacheTable() )
	{
		// Index 0 is always NULL, just ignore it
		// Index 1 == the world, don't 
		if ( stringNumber >= 1 )
		{
			cl.SetModel( stringNumber );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *object - 
//			stringTable - 
//			stringNumber - 
//			*newString - 
//			*newData - 
//-----------------------------------------------------------------------------
void Callback_GenericChanged( void *object, TABLEID stringTable, int stringNumber, char const *newString, void const *newData )
{
	if ( stringTable == cl.GetGenericPrecacheTable() )
	{
		// Index 0 is always NULL, just ignore it
		if ( stringNumber >= 1 )
		{
			cl.SetGeneric( stringNumber );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *object - 
//			stringTable - 
//			stringNumber - 
//			*newString - 
//			*newData - 
//-----------------------------------------------------------------------------
void Callback_SoundChanged( void *object, TABLEID stringTable, int stringNumber, char const *newString, void const *newData )
{
	if ( stringTable == cl.GetSoundPrecacheTable() )
	{
		// Index 0 is always NULL, just ignore it
		if ( stringNumber >= 1 )
		{
			cl.SetSound( stringNumber );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *object - 
//			stringTable - 
//			stringNumber - 
//			*newString - 
//			*newData - 
//-----------------------------------------------------------------------------
void Callback_DecalChanged( void *object, TABLEID stringTable, int stringNumber, char const *newString, void const *newData )
{
	if ( stringTable == cl.GetDecalPrecacheTable() )
	{
		cl.SetDecal( stringNumber );
	}
}


void Callback_InstanceBaselineChanged( void *object, TABLEID stringTable, int stringNumber, char const *newString, void const *newData )
{
	Assert( stringTable == cl.GetInstanceBaselineTable() );
	cl.UpdateInstanceBaseline( stringNumber );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CL_InstallEngineStringTableCallback( char const *tableName )
{
	// Hook Model Precache table
	if ( !Q_strcasecmp( tableName, MODEL_PRECACHE_TABLENAME ) )
	{
		// Look up the id 
		TABLEID id = networkStringTableContainerClient->FindStringTable( tableName );
		// Cache the id
		cl.SetModelPrecacheTable( id );
		// Install the callback
		networkStringTableContainerClient->SetStringChangedCallback( NULL, id, Callback_ModelChanged );
		return true;
	}

	if ( !Q_strcasecmp( tableName, GENERIC_PRECACHE_TABLENAME ) )
	{
		// Look up the id 
		TABLEID id = networkStringTableContainerClient->FindStringTable( tableName );
		// Cache the id
		cl.SetGenericPrecacheTable( id );
		// Install the callback
		networkStringTableContainerClient->SetStringChangedCallback( NULL, id, Callback_GenericChanged );
		return true;
	}

	if ( !Q_strcasecmp( tableName, SOUND_PRECACHE_TABLENAME ) )
	{
		// Look up the id 
		TABLEID id = networkStringTableContainerClient->FindStringTable( tableName );
		// Cache the id
		cl.SetSoundPrecacheTable( id );
		// Install the callback
		networkStringTableContainerClient->SetStringChangedCallback( NULL, id, Callback_SoundChanged );
		return true;
	}

	if ( !Q_strcasecmp( tableName, DECAL_PRECACHE_TABLENAME ) )
	{
		// Look up the id 
		TABLEID id = networkStringTableContainerClient->FindStringTable( tableName );
		// Cache the id
		cl.SetDecalPrecacheTable( id );
		// Install the callback
		networkStringTableContainerClient->SetStringChangedCallback( NULL, id, Callback_DecalChanged );
		return true;
	}

	if ( !Q_strcasecmp( tableName, INSTANCE_BASELINE_TABLENAME ) )
	{
		// Look up the id 
		TABLEID id = networkStringTableContainerClient->FindStringTable( tableName );
		// Cache the id
		cl.SetInstanceBaselineTable( id );
		// Install the callback
		networkStringTableContainerClient->SetStringChangedCallback( NULL, id, Callback_InstanceBaselineChanged );
		return true;
	}

	// The the client.dll have a shot at it
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Client side implementation of a string table
//-----------------------------------------------------------------------------
class CNetworkStringTableClient : public CNetworkStringTable
{
	typedef CNetworkStringTable BaseClass;

public:
	// Construction
							CNetworkStringTableClient( TABLEID id, const char *tableName, int maxentries );

	virtual void			DataChanged( int stringNumber, CNetworkStringTableItem *item );
	// Print contents to console
	virtual void			Dump( void );

	// Parse changed field info (2007 protocol)
	void					ParseUpdate( int numEntries );

	void					SetStringChangedCallback( void *object, pfnStringChanged changeFunc );

	void					DirectUpdate( int entryIndex, char const *string,
								int userdatalength, const void *userdata );

	void					WriteStringTable( CUtlBuffer& buf );
	bool					ReadStringTable( CUtlBuffer& buf );

	// 2007 protocol: User data fixed size support
	void					SetUserDataInfo( bool bFixedSize, int nSize, int nSizeBits );
	bool					IsUserDataFixedSize() const { return m_bUserDataFixedSize; }
	int						GetUserDataSize() const { return m_nUserDataSize; }
	int						GetUserDataSizeBits() const { return m_nUserDataSizeBits; }

private:
	CNetworkStringTableClient( const CNetworkStringTableClient & ); // not implemented, not accessible

	// Change function callback
	pfnStringChanged		m_changeFunc;
	// Optional context/object
	void					*m_pObject;

	// 2007 protocol: User data fixed size info
	bool					m_bUserDataFixedSize;
	int						m_nUserDataSize;
	int						m_nUserDataSizeBits;
};

//-----------------------------------------------------------------------------
// Purpose: Creates a string table on the client
// Input  : id -
//			*tableName -
//			maxentries -
//-----------------------------------------------------------------------------
CNetworkStringTableClient::CNetworkStringTableClient( TABLEID id, const char *tableName, int maxentries )
: CNetworkStringTable( id, tableName, maxentries )
{
	m_changeFunc = NULL;
	m_pObject = NULL;
	m_bUserDataFixedSize = false;
	m_nUserDataSize = 0;
	m_nUserDataSizeBits = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Set user data fixed size info (2007 protocol)
//-----------------------------------------------------------------------------
void CNetworkStringTableClient::SetUserDataInfo( bool bFixedSize, int nSize, int nSizeBits )
{
	m_bUserDataFixedSize = bFixedSize;
	m_nUserDataSize = nSize;
	m_nUserDataSizeBits = nSizeBits;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : changeFunc - 
//-----------------------------------------------------------------------------
void CNetworkStringTableClient::SetStringChangedCallback( void *object, pfnStringChanged changeFunc )
{
	m_changeFunc = changeFunc;
	m_pObject = object;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNetworkStringTableClient::Dump( void )
{
	Con_Printf( "Client\n" );
	BaseClass::Dump();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNetworkStringTableClient::DataChanged( int stringNumber, CNetworkStringTableItem *item )
{
	// Invoke callback if one was installed
	if ( m_changeFunc != NULL )
	{
		int userDataSize;
		const void *pUserData = item->GetUserData( &userDataSize );
		( *m_changeFunc )( m_pObject, GetTableId(), stringNumber, GetString( stringNumber ), pUserData );
	}
}

void CNetworkStringTableClient::DirectUpdate( int entryIndex, char const *string,
	int userdatalength, const void *userdata )
{
	char const *pName = string;

	// Read in the user data.
	const void *pUserData = NULL;
	int nBytes = 0;

	if ( userdatalength > 0 && userdata != NULL )
	{
		nBytes = userdatalength;
		pUserData = userdata;
	}

	/*
	char *netname = cl.m_pServerClasses[ atoi( pName ) ].m_ClassName;

	Con_Printf( "%s:  received %s, %i bytes = %s\n",
	   GetTableName(), netname, nBytes, pUserData ? COM_BinPrintf( (byte *)pUserData, nBytes ) : "NULL" );
	*/

	// Check if we are updating an old entry or adding a new one
	if ( entryIndex < GetNumStrings() )
	{
		SetString( entryIndex, pName );
		SetStringUserData( entryIndex, nBytes, pUserData );
	}
	else
	{
		// Grow the table (entryindex must be the next empty slot)
		Assert( entryIndex == GetNumStrings() );
		AddString( pName, nBytes, pUserData );
	}
}

void CNetworkStringTableClient::WriteStringTable( CUtlBuffer& buf )
{
	int numstrings = GetNumStrings();
	buf.PutInt( numstrings );
	for ( int i = 0 ; i < numstrings; i++ )
	{
		buf.PutString( GetString( i ) );
		int userDataSize;
		const void *pUserData = GetStringUserData( i, &userDataSize );
		if ( userDataSize > 0 )
		{
			buf.PutChar( 1 );
			buf.PutShort( (short)userDataSize );
			buf.Put( pUserData, userDataSize );
		}
		else
		{
			buf.PutChar( 0 );
		}
		
	}
}

bool CNetworkStringTableClient::ReadStringTable( CUtlBuffer& buf )
{
	DeleteAllStrings();

	// Check buffer validity
	if ( buf.IsValid() == false )
	{
		Warning( "CNetworkStringTableClient::ReadStringTable: Invalid buffer for table '%s'\n", GetTableName() );
		return false;
	}

	int numstrings = buf.GetInt();

	// Validate numstrings
	if ( numstrings < 0 )
	{
		Warning( "CNetworkStringTableClient::ReadStringTable: Invalid numstrings %d for table '%s'\n",
			numstrings, GetTableName() );
		return false;
	}

	if ( numstrings > GetMaxEntries() )
	{
		Warning( "CNetworkStringTableClient::ReadStringTable: numstrings %d exceeds max %d for table '%s'\n",
			numstrings, GetMaxEntries(), GetTableName() );
		return false;
	}

	for ( int i = 0 ; i < numstrings; i++ )
	{
		// Check for buffer overflow
		if ( buf.IsValid() == false )
		{
			Warning( "CNetworkStringTableClient::ReadStringTable: Buffer invalid at entry %d for table '%s'\n",
				i, GetTableName() );
			return false;
		}

		char stringname[4096];
		buf.GetString( stringname, sizeof( stringname ) );

		// Ensure null termination
		stringname[sizeof(stringname) - 1] = '\0';

		if ( buf.GetChar() == 1 )
		{
			int userDataSize = (int)buf.GetShort();

			// Validate userdata size
			if ( userDataSize <= 0 || userDataSize > CNetworkStringTableItem::MAX_USERDATA_SIZE )
			{
				Warning( "CNetworkStringTableClient::ReadStringTable: Invalid userdata size %d at entry %d for table '%s'\n",
					userDataSize, i, GetTableName() );
				return false;
			}

			byte *data = new byte[ userDataSize + 4 ];
			if ( !data )
			{
				Warning( "CNetworkStringTableClient::ReadStringTable: Failed to allocate %d bytes for entry %d in table '%s'\n",
					userDataSize + 4, i, GetTableName() );
				return false;
			}

			buf.Get( data, userDataSize );

			// Check if buffer read succeeded
			if ( buf.IsValid() == false )
			{
				delete[] data;
				Warning( "CNetworkStringTableClient::ReadStringTable: Buffer underflow reading userdata at entry %d for table '%s'\n",
					i, GetTableName() );
				return false;
			}

			AddString( stringname, userDataSize, data );

			delete[] data;
		}
		else
		{
			AddString( stringname );
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Parse string update with robust error handling (2007 protocol)
// Input  : numEntries - number of entries to read
//-----------------------------------------------------------------------------
void CNetworkStringTableClient::ParseUpdate( int numEntries )
{
	// Check for buffer overflow before starting
	if ( MSG_IsOverflowed() )
	{
		Warning( "CNetworkStringTableClient::ParseUpdate: Buffer already overflowed for table '%s'\n", GetTableName() );
		return;
	}

	CUtlVector< StringHistoryEntry > history;
	int lastEntry = -1;

	for ( int i = 0; i < numEntries; i++ )
	{
		// Check overflow
		if ( MSG_IsOverflowed() )
		{
			Warning( "CNetworkStringTableClient::ParseUpdate: Buffer overflow during update %d for table '%s'\n",
				i, GetTableName() );
			return;
		}

		// 2007 protocol: Delta encoding for entry index
		int entryIndex;
		if ( MSG_ReadOneBit() )
		{
			// Sequential - use lastEntry + 1
			entryIndex = lastEntry + 1;
		}
		else
		{
			// Read full index
			entryIndex = MSG_ReadBitLong( GetEntryBits() );
		}
		lastEntry = entryIndex;

		if ( entryIndex < 0 || entryIndex >= GetMaxEntries() )
		{
			Warning( "CNetworkStringTableClient::ParseUpdate: Bogus string index %i for table '%s'\n",
				entryIndex, GetTableName() );
			return;
		}

		const char *pEntry = NULL;
		char entry[ 1024 ];
		char substr[ 1024 ];

		// 2007 protocol: Check if string is included
		if ( MSG_ReadOneBit() )
		{
			// Has string data
			bool substringcheck = MSG_ReadOneBit() ? true : false;

			if ( substringcheck )
			{
				// 2007 protocol: Fixed 5-bit history index
				int index = MSG_ReadBitLong( 5 );

				if ( index >= 0 && index < history.Count() )
				{
					int bytestocopy = MSG_ReadBitLong( SUBSTRING_BITS );
					Q_strncpy( entry, history[ index ].string, bytestocopy + 1 );
					MSG_GetReadBuf()->ReadString( substr, sizeof( substr ) );
					Q_strncat( entry, substr, sizeof( entry ), COPY_ALL_CHARACTERS );
				}
				else
				{
					Warning( "CNetworkStringTableClient::ParseUpdate: Invalid history index %d for table '%s'\n",
						index, GetTableName() );
					return;
				}
			}
			else
			{
				MSG_GetReadBuf()->ReadString( entry, sizeof( entry ) );
			}

			pEntry = entry;
		}

		// Read in the user data
		unsigned char tempbuf[ CNetworkStringTableItem::MAX_USERDATA_SIZE ];
		memset( tempbuf, 0, sizeof( tempbuf ) );
		const void *pUserData = NULL;
		int nBytes = 0;

		if ( MSG_ReadOneBit() )
		{
			if ( IsUserDataFixedSize() )
			{
				// 2007 protocol: Fixed size user data
				nBytes = GetUserDataSize();
				if ( nBytes > 0 && nBytes <= (int)sizeof( tempbuf ) )
				{
					tempbuf[nBytes - 1] = 0; // Safety clear
					MSG_GetReadBuf()->ReadBits( tempbuf, GetUserDataSizeBits() );
				}
			}
			else
			{
				// Variable size user data
				nBytes = MSG_ReadBitLong( CNetworkStringTableItem::MAX_USERDATA_BITS );

				// Validate nBytes
				if ( nBytes < 0 || nBytes > (int)sizeof( tempbuf ) )
				{
					Warning( "CNetworkStringTableClient::ParseUpdate: Invalid userdata size %d for entry %d in table '%s'\n",
						nBytes, entryIndex, GetTableName() );
					return;
				}

				if ( nBytes > 0 )
				{
					MSG_GetReadBuf()->ReadBytes( tempbuf, nBytes );
				}
			}

			pUserData = tempbuf;
		}

		// Check overflow after reading
		if ( MSG_IsOverflowed() )
		{
			Warning( "CNetworkStringTableClient::ParseUpdate: Buffer overflow reading entry %d in table '%s'\n",
				entryIndex, GetTableName() );
			return;
		}

		// Check if we are updating an old entry or adding a new one
		if ( entryIndex < GetNumStrings() )
		{
			// Update existing entry
			SetStringUserData( entryIndex, nBytes, pUserData );
#ifdef _DEBUG
			if ( pEntry )
			{
				Assert( !Q_strcmp( pEntry, GetString( entryIndex ) ) ); // make sure string didn't change
			}
#endif
			pEntry = GetString( entryIndex ); // string didn't change
		}
		else
		{
			// Grow the table (entryindex must be the next empty slot)
			if ( entryIndex != GetNumStrings() )
			{
				Warning( "CNetworkStringTableClient::ParseUpdate: Non-sequential entry index %d (expected %d) in table '%s'\n",
					entryIndex, GetNumStrings(), GetTableName() );
			}

			if ( pEntry == NULL )
			{
				Warning( "CNetworkStringTableClient::ParseUpdate: NULL pEntry for table '%s', index %i\n",
					GetTableName(), entryIndex );
				pEntry = ""; // Prevent crash
			}

			AddString( pEntry, nBytes, pUserData );
		}

		// Update history
		if ( history.Count() > 31 )
		{
			history.Remove( 0 );
		}

		StringHistoryEntry she;
		Q_strncpy( she.string, pEntry, sizeof( she.string ) );
		history.AddToTail( she );
	}
}

static CNetworkStringTableContainerClient g_NetworkStringTableClient;
CNetworkStringTableContainerClient *networkStringTableContainerClient = &g_NetworkStringTableClient;
// Expose to client .dll
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CNetworkStringTableContainerClient, INetworkStringTableClient, INTERFACENAME_NETWORKSTRINGTABLECLIENT, g_NetworkStringTableClient );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CNetworkStringTableContainerClient::CNetworkStringTableContainerClient( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CNetworkStringTableContainerClient::~CNetworkStringTableContainerClient( void )
{
	RemoveAllTables();
}

void CNetworkStringTableContainerClient::SetStringChangedCallback( void *object, TABLEID stringTable, pfnStringChanged changeFunc )
{
	CNetworkStringTableClient *table = GetTable( stringTable );
	if ( table )
	{
		table->SetStringChangedCallback( object, changeFunc );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *tableName - 
// Output : TABLEID
//-----------------------------------------------------------------------------
TABLEID CNetworkStringTableContainerClient::FindStringTable( const char *tableName )
{
	for ( int i = 0; i < m_Tables.Size(); i++ )
	{
		if ( !_stricmp( tableName, m_Tables[ i ]->GetTableName() ) )
			return (TABLEID)i;
	}

	return INVALID_STRING_TABLE;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : stringTable - 
// Output : char const
//-----------------------------------------------------------------------------
char const *CNetworkStringTableContainerClient::GetTableName( TABLEID stringTable )
{
	CNetworkStringTableClient *table = GetTable( stringTable );
	assert( table );
	if ( !table )
	{
		assert( 0 );
		return "Unknown Table";
	}
	return table->GetTableName();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : stringTable - 
// Output : int
//-----------------------------------------------------------------------------
int CNetworkStringTableContainerClient::GetNumStrings( TABLEID stringTable )
{
	CNetworkStringTableClient *table = GetTable( stringTable );
	assert( table );
	if ( !table )
	{
		assert( 0 );
		return 0;
	}
	return table->GetNumStrings();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : stringTable - 
//-----------------------------------------------------------------------------
int	CNetworkStringTableContainerClient::GetMaxStrings( TABLEID stringTable )
{
	CNetworkStringTableClient *table = GetTable( stringTable );
	assert( table );
	if ( !table )
	{
		assert( 0 );
		return 0;
	}
	return table->GetMaxEntries();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : stringTable - 
//			stringNumber - 
// Output : const char
//-----------------------------------------------------------------------------
const char *CNetworkStringTableContainerClient::GetString( TABLEID stringTable, int stringNumber )
{
	CNetworkStringTableClient *table = GetTable( stringTable );
	assert( table );
	if ( !table )
	{
		assert( 0 );
		return NULL;
	}

	return table->GetString( stringNumber );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : stringTable - 
//			stringNumber - 
// Output : const char
//-----------------------------------------------------------------------------
const void *CNetworkStringTableContainerClient::GetStringUserData( TABLEID stringTable, int stringNumber, int *length /*=0*/ )
{
	CNetworkStringTableClient *table = GetTable( stringTable );
	assert( table );
	if ( !table )
	{
		assert( 0 );
		return NULL;
	}

	return table->GetStringUserData( stringNumber, length );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : stringTable - 
//			*string - 
// Output : int
//-----------------------------------------------------------------------------
int CNetworkStringTableContainerClient::FindStringIndex( TABLEID stringTable, char const *string )
{
	CNetworkStringTableClient *table = GetTable( stringTable );
	assert( table );
	if ( !table )
	{
		assert( 0 );
		return -1;
	}

	return table->FindStringIndex( string );	
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *tableName - 
//			maxentries - 
// Output : TABLEID
//-----------------------------------------------------------------------------
TABLEID CNetworkStringTableContainerClient::AddTable( const char *tableName, int maxentries )
{
	TABLEID found = FindStringTable( tableName );
	if ( found != INVALID_STRING_TABLE )
	{
		assert( 0 );
		return INVALID_STRING_TABLE;
	}

	TABLEID id = m_Tables.Size();

	CNetworkStringTableClient *pTable = new CNetworkStringTableClient( id, tableName, maxentries );
	assert( pTable );
	m_Tables.AddToTail( pTable );

	return id;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : stringTable - 
// Output : CNetworkStringTableClient
//-----------------------------------------------------------------------------
CNetworkStringTableClient *CNetworkStringTableContainerClient::GetTable( TABLEID stringTable )
{
	if ( stringTable < 0 || stringTable >= m_Tables.Size() )
		return NULL;

	return m_Tables[ stringTable ];
}

//-----------------------------------------------------------------------------
// Purpose: Parse string table definitions from server (2007 Source Engine protocol)
// Supports both 2003 LeakNet protocol and newer 2007 protocol
// Includes robust error handling for buffer overflow and corrupted data
//-----------------------------------------------------------------------------
void CNetworkStringTableContainerClient::ParseTableDefinitions( void )
{
	// Kill any existing ones
	RemoveAllTables();

	// Check for buffer overflow before starting
	if ( MSG_IsOverflowed() )
	{
		Warning( "ParseTableDefinitions: Message buffer already overflowed before parsing\n" );
		return;
	}

	int numTables = MSG_ReadByte();

	// Validate numTables
	if ( numTables < 0 || numTables > MAX_TABLES )
	{
		Warning( "ParseTableDefinitions: Invalid numTables %d (max %d) - buffer may be corrupted\n", numTables, MAX_TABLES );
		return;
	}

	// Check for overflow after reading
	if ( MSG_IsOverflowed() )
	{
		Warning( "ParseTableDefinitions: Buffer overflow reading numTables\n" );
		return;
	}

	CUtlVector< StringHistoryEntry > history;

	for ( int i = 0; i < numTables; i++ )
	{
		history.RemoveAll();

		// Check overflow before reading table name
		if ( MSG_IsOverflowed() )
		{
			Warning( "ParseTableDefinitions: Buffer overflow before table %d\n", i );
			return;
		}

		// 2007 protocol: Check for ':' prefix indicating filenames table
		bool bIsFilenames = false;
		char prefix = MSG_GetReadBuf()->PeekUBitLong( 8 );
		if ( prefix == ':' )
		{
			bIsFilenames = true;
			MSG_ReadByte(); // consume the prefix
		}

		char tableName[ 256 ];
		const char *pReadName = MSG_ReadString();
		if ( !pReadName || MSG_IsOverflowed() )
		{
			Warning( "ParseTableDefinitions: Failed to read table name for table %d\n", i );
			return;
		}
		Q_strncpy( tableName, pReadName, sizeof( tableName ) );

		// Validate table name - must be non-empty
		if ( tableName[0] == '\0' )
		{
			Warning( "ParseTableDefinitions: Empty table name at index %d - buffer may be corrupted\n", i );
			return;
		}

		int maxentries = MSG_ReadShort();

		// Check overflow after reading maxentries
		if ( MSG_IsOverflowed() )
		{
			Warning( "ParseTableDefinitions: Buffer overflow reading maxentries for table '%s'\n", tableName );
			return;
		}

		// Validate maxentries - must be > 0 and a power of 2
		// If invalid, message buffer is likely corrupted
		if ( maxentries <= 0 )
		{
			Warning( "ParseTableDefinitions: Invalid maxentries %d for table '%s' - buffer may be corrupted\n", maxentries, tableName );
			return;
		}
		// Check if power of 2
		if ( (maxentries & (maxentries - 1)) != 0 )
		{
			Warning( "ParseTableDefinitions: maxentries %d not power of 2 for '%s' - buffer may be corrupted\n", maxentries, tableName );
			return;
		}
		// Sanity check - maxentries shouldn't be unreasonably large
		if ( maxentries > 65536 )
		{
			Warning( "ParseTableDefinitions: maxentries %d too large for '%s' - buffer may be corrupted\n", maxentries, tableName );
			return;
		}

		// Calculate entry bits
		int entryBits = Q_log2( maxentries );

		// 2007 protocol: Read number of entries
		int usedentries = MSG_ReadBitLong( entryBits + 1 );

		// Debug: Log table info being parsed
		DevMsg( "ParseTableDefinitions: Reading table '%s' (maxentries=%d, entryBits=%d, usedentries=%d)\n",
			tableName, maxentries, entryBits, usedentries );

		// Validate usedentries
		if ( usedentries < 0 || usedentries > maxentries )
		{
			Warning( "ParseTableDefinitions: Invalid usedentries %d (max %d) for table '%s'\n",
				usedentries, maxentries, tableName );
			return;
		}

		// Read data length in bits (24 bits to support larger tables like soundprecache)
		// Original 2007 protocol used 20 bits, but we need more for GMod-compatible servers
		int dataLengthBits = MSG_ReadBitLong( 24 );

		DevMsg( "ParseTableDefinitions: Table '%s' dataLengthBits=%d\n", tableName, dataLengthBits );

		if ( dataLengthBits < 0 )
		{
			Warning( "ParseTableDefinitions: Invalid data length %d for table '%s'\n", dataLengthBits, tableName );
			return;
		}

		// 2007 protocol: Read user data fixed size info
		bool bUserDataFixedSize = MSG_ReadOneBit() ? true : false;
		int nUserDataSize = 0;
		int nUserDataSizeBits = 0;

		if ( bUserDataFixedSize )
		{
			nUserDataSize = MSG_ReadBitLong( 12 );
			nUserDataSizeBits = MSG_ReadBitLong( 4 );
		}

		// Check overflow after reading header
		if ( MSG_IsOverflowed() )
		{
			Warning( "ParseTableDefinitions: Buffer overflow reading header for table '%s'\n", tableName );
			return;
		}

		TABLEID id = AddTable( tableName, maxentries );
		if ( id == INVALID_STRING_TABLE )
		{
			Warning( "ParseTableDefinitions: Failed to add table '%s'\n", tableName );
			return;
		}

		CNetworkStringTableClient *table = GetTable( id );
		if ( !table )
		{
			Warning( "ParseTableDefinitions: Failed to get table '%s'\n", tableName );
			return;
		}

		// Store user data fixed size info in table (for later use in ParseUpdate)
		table->SetUserDataInfo( bUserDataFixedSize, nUserDataSize, nUserDataSizeBits );

		// Let engine hook callbacks before we read in any data values at all
		if ( !CL_InstallEngineStringTableCallback( tableName ) )
		{
			// If engine takes a pass, allow client dll to hook in its callbacks
			g_ClientDLL->InstallStringTableCallback( tableName );
		}

		// Track bit position before reading entries
		int startBit = MSG_GetReadBuf()->GetNumBitsRead();

		// 2007 protocol: Use delta encoding for entry indices
		int lastEntry = -1;

		for ( int j = 0; j < usedentries; j++ )
		{
			// Check overflow at start of each entry
			if ( MSG_IsOverflowed() )
			{
				Warning( "ParseTableDefinitions: Buffer overflow at entry %d of table '%s'\n", j, tableName );
				return;
			}

			// 2007 protocol: Delta encoding for entry index
			// If bit is 1, use lastEntry + 1, else read full index
			int entryIndex;
			if ( MSG_ReadOneBit() )
			{
				entryIndex = lastEntry + 1;
			}
			else
			{
				entryIndex = MSG_ReadBitLong( entryBits );
			}
			lastEntry = entryIndex;

			// Validate entry index
			if ( entryIndex < 0 || entryIndex >= maxentries )
			{
				Warning( "ParseTableDefinitions: Invalid entry index %d in table '%s'\n", entryIndex, tableName );
				return;
			}

			char entry[ 1024 ];
			entry[ 0 ] = 0;
			const char *pEntry = NULL;

			// 2007 protocol: Check if string is included
			if ( MSG_ReadOneBit() )
			{
				// Has string data
				bool substringcheck = MSG_ReadOneBit() ? true : false;
				if ( substringcheck )
				{
					// 2007 protocol: Fixed 5-bit history index
					int index = MSG_ReadBitLong( 5 );

					// Validate history index
					if ( index < 0 || index >= history.Count() )
					{
						Warning( "ParseTableDefinitions: Invalid history index %d (count %d) in table '%s'\n",
							index, history.Count(), tableName );
						return;
					}

					int bytestocopy = MSG_ReadBitLong( SUBSTRING_BITS );

					// Validate bytes to copy
					if ( bytestocopy < 0 || bytestocopy > (int)sizeof( history[index].string ) )
					{
						Warning( "ParseTableDefinitions: Invalid bytestocopy %d in table '%s'\n", bytestocopy, tableName );
						return;
					}

					Q_strncpy( entry, history[ index ].string, bytestocopy + 1 );

					char substr[ 1024 ];
					MSG_GetReadBuf()->ReadString( substr, sizeof( substr ) );
					Q_strncat( entry, substr, sizeof( entry ), COPY_ALL_CHARACTERS );
				}
				else
				{
					MSG_GetReadBuf()->ReadString( entry, sizeof( entry ) );
				}

				pEntry = entry;
			}

			// Check overflow after reading entry string
			if ( MSG_IsOverflowed() )
			{
				Warning( "ParseTableDefinitions: Buffer overflow reading entry string %d in table '%s'\n", j, tableName );
				return;
			}

			// Read userdata
			unsigned char tempbuf[ CNetworkStringTableItem::MAX_USERDATA_SIZE ];
			memset( tempbuf, 0, sizeof( tempbuf ) );
			const void *pUserData = NULL;
			int nBytes = 0;

			if ( MSG_ReadOneBit() )
			{
				if ( bUserDataFixedSize )
				{
					// 2007 protocol: Fixed size user data
					nBytes = nUserDataSize;
					if ( nBytes > 0 && nBytes <= (int)sizeof( tempbuf ) )
					{
						tempbuf[nBytes - 1] = 0; // Safety clear
						MSG_GetReadBuf()->ReadBits( tempbuf, nUserDataSizeBits );
					}
				}
				else
				{
					// Variable size user data
					nBytes = MSG_ReadBitLong( CNetworkStringTableItem::MAX_USERDATA_BITS );

					// Validate nBytes
					if ( nBytes < 0 || nBytes > CNetworkStringTableItem::MAX_USERDATA_SIZE )
					{
						Warning( "ParseTableDefinitions: Invalid userdata size %d in table '%s'\n", nBytes, tableName );
						return;
					}

					if ( nBytes > 0 )
					{
						MSG_GetReadBuf()->ReadBytes( tempbuf, nBytes );
					}
				}

				pUserData = tempbuf;
			}

			// Check overflow after reading user data
			if ( MSG_IsOverflowed() )
			{
				Warning( "ParseTableDefinitions: Buffer overflow reading userdata in table '%s'\n", tableName );
				return;
			}

			// Check if we are updating an old entry or adding a new one
			if ( entryIndex < table->GetNumStrings() )
			{
				// Update existing entry
				if ( pEntry )
				{
					table->SetString( entryIndex, pEntry );
				}
				table->SetStringUserData( entryIndex, nBytes, pUserData );
			}
			else
			{
				// Add new entry (should be sequential)
				if ( pEntry == NULL )
				{
					Warning( "ParseTableDefinitions: NULL string for new entry %d in table '%s'\n", entryIndex, tableName );
					pEntry = ""; // Prevent crash
				}
				table->AddString( pEntry, nBytes, pUserData );
			}

			// Update history
			if ( history.Count() > 31 )
			{
				history.Remove( 0 );
			}

			StringHistoryEntry she;
			if ( pEntry )
			{
				Q_strncpy( she.string, pEntry, sizeof( she.string ) );
			}
			else if ( entryIndex < table->GetNumStrings() )
			{
				Q_strncpy( she.string, table->GetString( entryIndex ), sizeof( she.string ) );
			}
			else
			{
				she.string[0] = '\0';
			}
			history.AddToTail( she );
		}

		// Verify we read the expected number of bits
		int endBit = MSG_GetReadBuf()->GetNumBitsRead();
		int bitsRead = endBit - startBit;
		if ( bitsRead != dataLengthBits )
		{
			Warning( "ParseTableDefinitions: Bit count mismatch for table '%s': read %d bits, expected %d bits\n",
				tableName, bitsRead, dataLengthBits );
		}

		DevMsg( "ParseTableDefinitions: Loaded table '%s' with %d entries (fixedsize=%d, userdata=%d bits, dataBits=%d)\n",
			tableName, usedentries, bUserDataFixedSize, nUserDataSizeBits, dataLengthBits );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Parse string table update with error handling (2007 protocol)
//-----------------------------------------------------------------------------
void CNetworkStringTableContainerClient::ParseUpdate( void )
{
	// Check for buffer overflow before reading
	if ( MSG_IsOverflowed() )
	{
		Warning( "CNetworkStringTableContainerClient::ParseUpdate: Buffer already overflowed\n" );
		return;
	}

	// 2007 protocol: Read table ID (5 bits for up to 32 tables)
	TABLEID tableId;
	tableId = MSG_ReadBitLong( 5 );

	// Check for overflow after reading table ID
	if ( MSG_IsOverflowed() )
	{
		Warning( "CNetworkStringTableContainerClient::ParseUpdate: Buffer overflow reading table ID\n" );
		return;
	}

	if ( tableId < 0 || tableId >= m_Tables.Size() )
	{
		Warning( "CNetworkStringTableContainerClient::ParseUpdate: Invalid table id %i (max %d)\n",
			tableId, m_Tables.Size() );
		return;
	}

	CNetworkStringTableClient *table = m_Tables[ tableId ];
	if ( !table )
	{
		Warning( "CNetworkStringTableContainerClient::ParseUpdate: Null table at id %i\n", tableId );
		return;
	}

	// 2007 protocol: Read number of changed entries
	int numChangedEntries = 1; // Default to 1 for backwards compatibility
	if ( MSG_ReadOneBit() )
	{
		numChangedEntries = MSG_ReadBitLong( 16 );
	}

	// 2007 protocol: Read data length in bits
	int dataLengthBits = MSG_ReadBitLong( 20 );
	if ( dataLengthBits < 0 )
	{
		Warning( "CNetworkStringTableContainerClient::ParseUpdate: Invalid data length for table '%s'\n",
			table->GetTableName() );
		return;
	}

	// Check for overflow
	if ( MSG_IsOverflowed() )
	{
		Warning( "CNetworkStringTableContainerClient::ParseUpdate: Buffer overflow reading header for table '%s'\n",
			table->GetTableName() );
		return;
	}

	table->ParseUpdate( numChangedEntries );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNetworkStringTableContainerClient::RemoveAllTables( void )
{
	while ( m_Tables.Size() > 0 )
	{
		CNetworkStringTableClient *table = m_Tables[ 0 ];
		m_Tables.Remove( 0 );
		delete table;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNetworkStringTableContainerClient::Dump( void )
{
	for ( int i = 0; i < m_Tables.Size(); i++ )
	{
		m_Tables[ i ]->Dump();
	}
}

void CNetworkStringTableContainerClient::WriteStringTables( CUtlBuffer& buf )
{
	int numTables = m_Tables.Size();

	buf.PutInt( numTables );
	for ( int i = 0; i < numTables; i++ )
	{
		CNetworkStringTableClient *table = m_Tables[ i ];
		buf.PutString( table->GetTableName() );
		table->WriteStringTable( buf );
	}
}

bool CNetworkStringTableContainerClient::ReadStringTables( CUtlBuffer& buf )
{
	int numTables = buf.GetInt();
	for ( int i = 0 ; i < numTables; i++ )
	{
		char tablename[ 256 ];
		buf.GetString( tablename, sizeof( tablename ) );

		TABLEID id = FindStringTable( tablename );
		if ( id == INVALID_STRING_TABLE )
		{
			Host_Error( "Error reading string tables, no such table %s\n", tablename );
		}

		// Find this table by name
		CNetworkStringTableClient *table = GetTable( id );
		Assert( table );

		// Now read the data for the table
		if ( !table->ReadStringTable( buf ) )
		{
			Host_Error( "Error reading string table %s\n", tablename );
		}

	}
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CL_ParseStringTables( void )
{
	g_NetworkStringTableClient.ParseTableDefinitions();

	CL_RegisterResources();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CL_ParseUpdateStringTable( void )
{
	g_NetworkStringTableClient.ParseUpdate();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CL_PrintStringTables( void )
{
	g_NetworkStringTableClient.Dump();
}

void LocalNetworkBackDoor_DirectStringTableUpdate( int tableId, int entryIndex, char const *string,
	int userdatalength, const void *userdata )
{
	CNetworkStringTableClient *table = networkStringTableContainerClient->GetTable( tableId );
	if ( !table )
	{
		Sys_Error( "Bogus table id in network string table backdoor!(%i)", tableId );
	}

	table->DirectUpdate( entryIndex, string, userdatalength, userdata );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : buf - 
//-----------------------------------------------------------------------------
void CL_WriteStringTables( CUtlBuffer& buf )
{
	g_NetworkStringTableClient.WriteStringTables( buf );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : buf - 
//-----------------------------------------------------------------------------
bool CL_ReadStringTables( CUtlBuffer& buf )
{
	return g_NetworkStringTableClient.ReadStringTables( buf );
}