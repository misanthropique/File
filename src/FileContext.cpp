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

// Here is the list of supported schemes
#include "scheme/scheme_file.hpp"

static std::atomic_uint64_t _G_FileIdentifierCounter( 1 );
static std::unordered_map< uint64_t, struct FileContext* > _G_FileIdentifierContextMap;
static std::mutex _G_FileIdentifierContextMapMutex;

static const std::map< std::string, struct SchemeFunctionPointers > SUPPORTED_SCHEME_API_MAP {
	{ SCHEME_FILE_CANONICAL_PREFIX, SCHEME_FILE_API }
};

struct FileContext* _allocate_context()
{
	struct FileContext* context = static_cast< struct FileContext* >( calloc( 1, sizeof( struct FileContext ) ) );

	if ( nullptr != context )
	{
		// Initialize non-POD variables
	}

	return context;
}

bool _open_uri(
	const FileContext* context,
	const std::string& uri,
	File::IOFlag mode,
	int& errorCode )
{
	std::string scheme = _get_scheme( uri );

	auto schemeAPIIterator = SUPPORTED_SCHEME_API_MAP.find( scheme );
	if ( SUPPORTED_SCHEME_API_MAP.end() == schemeAPIIterator )
	{
		return false;
	}

	return true;
}

uint64_t _register_context(
	FileContext* context )
{
	uint64_t identifier = _G_FileIdentifierCounter++;
	std::lock_guard mapLock( _G_FileIdentifierContextMapMutex );
	_G_FileIdentifierContextMap[ identifier ] = context;
	return identifier;
}

struct FileContext* _get_context(
	uint64_t fileIdentifier,
	std::unique_lock< std::mutex >& contextLock )
{
}

void _release_context(
	uint64_t fileIdentifier )
{
	std::lock_guard mapLock( _G_FileIdentifierContextMapMutex );
	struct FileContext* context = _G_FileIdentifierContextMap[ fileIdentifier ];
	if ( 1 == context->_M_ReferenceCount-- )
	{
		// Remove the context from the map.
		// Release the lock.
		// close the resource.
	}
}

void _update_io_stats(
	struct FileContext& context,
	uint32_t ioStat,
	struct timeval& startTime,
	struct timeval& endTime,
	int64_t bytes )
{
	if ( ( 0 < bytes )
		and ( ioStat < FILE_IO_STATS_SIZE ) )
	{
		long double duration = ( endTime.tv_usec - startTime.tv_usec )
			+ MICROSECONDS_IN_SECOND * ( endTime.tv_sec - startTime.tv_sec );
		context._M_SumInverseRates[ ioStat ] += duration / ( MICROSECONDS_IN_SECOND * bytes );
		context._M_NumberObservations[ ioStat ] += 1;
	}
}
