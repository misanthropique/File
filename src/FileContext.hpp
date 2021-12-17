/**
 * Copyright ©2021. Brent Weichel. All Rights Reserved.
 * Permission to use, copy, modify, and/or distribute this software, in whole
 * or part by any means, without express prior written agreement is prohibited.
 */
#pragma once

#include <cstdint>
#include <mutex>
#include <sys/time.h>

#include "File.hpp"

#define FILE_IO_STATS_READ  ( 0 )
#define FILE_IO_STATS_WRITE ( 1 )
#define FILE_IO_STATS_SIZE  ( 2 )

struct FileContext
{
	std::mutex _M_Mutex;
	uint32_t _M_ReferenceCount;

	// Use the harmonic mean to compute the average of rates.
	double _M_SumInverseRates[ FILE_IO_STATS_SIZE ];
	uint64_t _M_NumberObservations[ FILE_IO_STATS_SIZE ];

	int64_t _M_FilePosition;
	File::IOFlag _M_Capabilities;
	int _M_ErrorCode;

	int64_t ( *_F_read )( struct FileContext*, uint8_t*, uint32_t, bool );
	int64_t ( *_F_write )( struct FileContext*, uint8_t*, uint32_t, bool );
};

#define FILE_CAN_READ( context )  ( ( context )->_M_Capabilities & File::IOFlag::READ )
#define FILE_CAN_WRITE( context ) ( ( context )->_M_Capabilities & File::IOFlag::WRITE )

/*
 * Create a context for the requested file. Should a context already
 * exist for the file, that identifier is returned.
 * @param filepath Const reference to the file.
 * @param mode Mode in which to open the file.
 * @return A file identifier for the requested file is returned.
 */
uint64_t _create_context(
	const std::string& filepath,
	File::IOFlag mode );

/*
 * Get the context for the file associated with the given identifier.
 * @param fileIdentifier Identifier to the file context.
 * @param contextLock The lock for acquiring the context.
 * @return A pointer to the context is returned. If nullptr is returned then
 *         there is no file context for the provided identifier.
 */
struct FileContext* _get_context(
	uint64_t fileIdentifier,
	std::unique_lock< std::mutex >& contextLock );

/*
 * Decrement the reference count, and release the resources if
 * no additional File instances point to the context.
 * @param fileIdentifier identifier for the file to be released.
 */
void _release_context(
	uint64_t fileIdentifier );

/*
 * Updated the IO stats with the latest information.
 * @param sumInverseRates A reference to the sum of the inverse of the rate.
 * @param numberObservations A reference to the count of the number of rate observations made.
 * @param startTime The logged time just before a read or write is called.
 * @param endTime The logged time just after a read or write is called.
 * @param bytes The number of bytes written or read from the file.
 */
void _update_io_stats(
	double& sumInverseRates,
	uint64_t& numberObservations,
	struct timeval& startTime,
	struct timeval& endTime,
	int64_t bytes );
