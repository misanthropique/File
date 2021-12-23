/**
 * Copyright ©2021. Brent Weichel. All Rights Reserved.
 * Permission to use, copy, modify, and/or distribute this software, in whole
 * or part by any means, without express prior written agreement is prohibited.
 */
#include <cstdint>
#include <cstdlib>
#include <mutex>
#include <string>
#include <sys/time.h>
#include <unordered_map>

#include "File.hpp"
#include "FileContext.hpp"
#include "Util.hpp"

static std::atomic_uint64_t _G_FileIdentifierCounter( 1 );
static std::unordered_map< uint64_t, struct FileContext* > _G_FileIdentifierContextMap;
static std::mutex _G_FileIdentifierContextMapMutex;

static const std::map< std::string, struct SchemeFunctionPointers > {
	{ "file", scheme::file::__function_pointers }
}

struct FileContext* _allocate_context()
{
	struct FileContext* context = static_cast< struct FileContext* >( calloc( 1, sizeof( struct FileContext ) ) );
}

uint64_t _assign_file_identifier(
	struct FileContext* context )
{
}

uint64_t _create_context(
	const std::string& filepath,
	File::IOFlag mode )
{
	struct FileContext* context =
		static_cast< struct FileContext* >(
			std::calloc( 1, sizeof( struct FileContext ) ) );

	if ( nullptr == context )
	{
		throw std::bad_alloc();
	}

	new( static_cast< void* >( &context->_M_Mutex ) ) std::mutex();
	context->_M_FileSize = int64_t( -1 );
	context->_M_FilePosition = int64_t( -1 );

	return 
}

struct FileContext* _get_context(
	uint64_t fileIdentifier,
	std::unique_lock< std::mutex >& contextLock )
{
}

void _release_context(
	uint64_t fileIdentifier )
{
}

#define MICROSECONDS_IN_SECOND ( 1000000.0L )

void _update_io_stats(
	double& sumInverseRates,
	uint64_t& numberObservations,
	struct timeval& startTime,
	struct timeval& endTime,
	int64_t bytes )
{
	if ( 0 < bytes )
	{
		long double duration = ( endTime.tv_usec - startTime.tv_usec )
			+ MICROSECONDS_IN_SECOND * ( endTime.tv_sec - startTime.tv_sec );
		sumInverseRates += duration / ( MICROSECONDS_IN_SECOND * bytes );
		++numberObservations;
	}
}
