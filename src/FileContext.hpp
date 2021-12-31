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

	int64_t _M_FileSize;
	int64_t _M_FilePosition; // Current file position
	File::IOFlag _M_Capabilities; // Read | Write | Seek flags
	int _M_ErrorCode; // Context level error codes, scheme specific codes are stored in _M_SchemeContext

	void* _M_SchemeContext;

	/*
	 * Get the scheme specific error message.
	 */
	std::string ( *_F_error_string )( struct FileContext* );

	/*
	 * Decrement the reference count and close the resource if applicable.
	 */
	void ( *_F_close )( struct FileContext* );

	/*
	 * Seek to the requested position if supported.
	 */
	int64_t ( *_F_seek )( struct FileContext*, int64_t, bool );

	/*
	 * The number of bytes read from the resource is returned.
	 */
	int64_t ( *_F_read )( struct FileContext*, uint8_t*, uint32_t, bool );

	/*
	 * The number of bytes written out to the resource is returned.
	 */
	int64_t ( *_F_write )( struct FileContext*, uint8_t*, uint32_t, bool );

	/*
	 * Resize the file to the desired size.
	 * signature: ( context: FileContext*, size: int64_t, fill: uint8_t, shrink: bool, grow: bool ) -> int64_t
	 * The new file size is returned.
	 * The following mappings exist for the File API
	 * reserve( size, fill ) ↦ _resize( context, size, fill, false, true );
	 * resize( size, fill )  ↦ _resize( context, size, fill, true,  true );
	 * truncate( size )      ↦ _resize( context, size, '\0', true,  false );
	 */
	int64_t ( *_F_resize )( struct FileContext*, int64_t, uint8_t, bool, bool );

	/*
	 * Synchronize the contents of the memory buffer with the resource if applicable.
	 * False is returned if an error occurred, true on success or no-op.
	 */
	bool ( *_F_sync )( struct FileContext* );
};

#define FILE_CAN_READ( context )  ( ( context )->_M_Capabilities & File::IOFlag::READ )
#define FILE_CAN_WRITE( context ) ( ( context )->_M_Capabilities & File::IOFlag::WRITE )
#define FILE_CAN_SEEK( context )  ( ( context )->_M_Capabilities & File::IOFlag::SEEK )

/*
 * Allocate a FileContext object initialized to a zero state.
 * @return A pointer to a FileContext object is returned.
 */
struct FileContext* _allocate_context();

/*
 * Open the URI into the provided context.
 * @param context A pointer to the context to store the handle to the file.
 * @param uri The URI to the resource to be opened.
 * @param mode The mode in which to open the resource.
 * @param errorCode A reference to an integer in which to store error codes related to opening the file.
 * @return True is returned upon successfully opening the resource, false is returned on error and {@param errorCode} is set.
 */
bool _open_uri(
	const FileContext* context,
	const std::string& uri,
	File::IOFlag mode,
	int& errorCode );

/*
 * @param context A pointer to the context to register.
 * @return A file identifier that the context is registered to.
 */
int64_t _register_context(
	FileContext* context );

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
	struct FileContext& context,
	uint32_t ioStat,
	struct timeval& startTime,
	struct timeval& endTime,
	int64_t bytes );
