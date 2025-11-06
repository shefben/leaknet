// VXP

#include <stdio.h>
#include <string.h>

#include "studio.h"
#include "modelgen.h"

int main( int argc, char **argv )
{
	FILE *fp = fopen( argv[1], "rb" );
	fseek( fp, 0, SEEK_END );
	int len = ftell( fp );
	if ( len <= 0 )
	{
		fprintf( stderr, "Model %s is empty\r\n", argv[1] ); // Hello houndeye!
		fclose( fp );
		return -1;
	}

	rewind( fp );

	studiohdr_t *pStudioHdr = ( studiohdr_t * )malloc( len );
	if ( pStudioHdr == NULL ) // Cannot allocate a memory
	{
		fprintf( stderr, "Unexpected memory error\r\n" );
		fclose( fp );
		return -1;
	}

	size_t readlen = fread( pStudioHdr, 1, len, fp );
	if ( readlen != len || pStudioHdr == NULL )
	{
		fprintf( stderr, "Error reading a model\r\n" );
		fclose( fp );
		return -1;
	}

	Studio_ConvertStudioHdrToNewVersion( pStudioHdr );

	if( pStudioHdr->version != STUDIO_VERSION )
	{
	//	fprintf( stderr, "Wrong version (%d != %d)\r\n", ( int )pStudioHdr->version, ( int )STUDIO_VERSION );
		fclose( fp );
		return -1;
	}

	//mstudioanimgroup_t *dummy1 = pStudioHdr->pAnimgroup( 0 );
	//if ( dummy1->group != 0 )
	//	printf( "%s has %d\r\n", pStudioHdr->name, dummy1->group );

	//mstudiobonedesc_t *dummy2 = pStudioHdr->pBonedesc( 0 );
	//if ( dummy2->dummy1 != -1 )
	//	printf( "%s has %d\r\n", pStudioHdr->name, dummy2->dummy1 );

	switch( pStudioHdr->id )
	{
	case IDSTUDIOHEADER:
	//	printf( "%s is IDSTUDIOHEADER\r\n", pStudioHdr->name );
		break;
	case IDSTUDIOSEQHEADER:
		printf( "%s is IDSTUDIOSEQHEADER\r\n", pStudioHdr->name );
		break;
	case IDSTUDIOANIMGROUPHEADER:
		printf( "%s is IDSTUDIOANIMGROUPHEADER\r\n", pStudioHdr->name );
		break;
	default:
		printf( "%s has unknown model ID (%i)\r\n", pStudioHdr->name, pStudioHdr->id );
		break;
	}

	fclose( fp );
	return 0;
}
