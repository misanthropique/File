/**
 * Copyright ©2021. Brent Weichel. All Rights Reserved.
 * Permission to use, copy, modify, and/or distribute this software, in whole
 * or part by any means, without express prior written agreement is prohibited.
 */
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <mutex>
#include <string>
#include <sys/time.h>
#include <utility>

#include "File.hpp"
#include "FileContext.hpp"
#include "Util.hpp"

static uint64_t __open_file(
	const std::string& filepath,
	File::IOFlag mode )
{
	std::string normalizedFilepath = _normalize_filepath( filepath );
	std::string scheme = _get_scheme( normalizedFilepath );

	// Check if the scheme is supported
	if ( _scheme_supported( normalizedFilepath ) )
	{
		// Create the context
		struct FileContext* context = _allocate_context();

		// Get the function pointer for opening a file by scheme.
		auto schemeOpenFile = _get_scheme_open( scheme );

		// Open the file into the context
		if ( not schemeOpenFile( context, normalizedFilepath ) )
		{
			// Error
		}

		// Get an identifier for the context
		uint64_t identifier = _assign_file_identifier( context );

		// Return identifier
		return identifier;
	}
}

static void __close_file(
	uint64_t fileIdentifier )
{
}

File::File() noexcept :
	mFileIdentifier( 0 )
{
}

File::File(
	File&& other )
{
	mFileIdentifier.store( other.mFileIdentifier.exchange( 0 ) );
	mErrorCode = std::exchange( other.mErrorCode, 0 );
}

File::File(
	const File& other )
{
	mFileIdentifier.store( other.mFileIdentifier.load() );
	mErrorCode = other.mErrorCode;
}

File::File(
	const std::string& filepath,
	File::IOFlag mode )
{
	uint64_t fileIdentifier = __open_file( filepath, mode );

	if ( 0 == fileIdentifier )
	{
	}
}

int64_t File::append(
	const uint8_t* buffer,
	uint32_t count )
{
	if ( 0 == mFileIdentifier.load() )
	{
		mErrorCode = EBADF;
		return -1;
	}

	if ( nullptr == buffer )
	{
		mErrorCode = ( 0 != count ) ? EINVAL : 0;
		return -( 0 != count );
	}

	if ( 0 == count )
	{
		mErrorCode = 0;
		return 0;
	}

	std::unique_lock< std::mutex > contextLock;
	auto context = _get_context( mFileIdentifier, contextLock );

	if ( nullptr == context )
	{
		mErrorCode = EBADF;
		return -1;
	}

	if ( FILE_CAN_WRITE( context ) )
	{
		struct timeval startTime, endTime;
		int64_t bytesWritten;

		gettimeofday( &startTime, nullptr );
		bytesWritten = context->_F_write( context, buffer, count, true );
		gettimeofday( &endTime, nullptr );

		_update_io_stats( context->_M_SumInverseRates[ FILE_IO_STATS_WRITE ],
			context->_M_NumberObservations[ FILE_IO_STATS_WRITE ], startTime, endTime, bytesWritten );

		return bytesWritten;
	}

	mErrorCode = ENOTSUP;
	return -1;
}

double File::byteRate(
	File::IOFlag ioFlag ) const
{
	if ( 0 == mFileIdentifier.load() )
	{
		mErrorCode = EBADF;
		return std::nan( "0" );
	}

	std::unique_lock< std::mutex > contextLock;
	auto context = _get_context( mFileIdentifier, contextLock );

	if ( nullptr == context )
	{
		mErrorCode = EBADF;
		return std::nan( "0" );
	}

	if ( File::IOFlag::READ & ioFlag )
	{
		if ( 0 == context->_M_NumberObservations[ FILE_IO_STATS_READ ] )
		{
			return 0;
		}

		return static_cast< double >( context->_M_NumberObservations[ FILE_IO_STATS_READ ] )
			/ context->_M_SumInverseRates[ FILE_IO_STATS_READ ];
	}
	else if ( File::IOFlag::WRITE & ioFlag )
	{
		if ( 0 == context->_M_NumberObservations[ FILE_IO_STATS_WRITE ] )
		{
			return 0;
		}

		return static_cast< double >( context->_M_NumberObservations[ FILE_IO_STATS_WRITE ] )
			/ context->_M_SumInverseRates( FILE_IO_STATS_WRITE ]
	}

	return std::nan( "0" );
}

File& File::operator=(
	File&& other )
{
	if ( this != &other )
	{
		mFileIdentifier.store( other.mFileIdentifier.exchange( 0 ) );
		mErrorCode = std::exchange( other.mErrorCode, 0 );
	}

	return *this;
}

File& File::operator=(
	const File& other )
{
	if ( this != &other )
	{
		mFileIdentifier.store( other.mFileIdentifier.load() );
		mErrorCode = other.mErrorCode;
	}

	return *this;
}

int64_t File::peek(
	uint8_t* buffer,
	uint32_t count )
{
	if ( 0 == mFileIdentifier.load() )
	{
		mErrorCode = EBADF;
		return -1;
	}

	if ( nullptr == buffer )
	{
		mErrorCode = ( 0 != count ) ? EINVAL : 0;
		return -( 0 != count );
	}

	if ( 0 == count )
	{
		mErrorCode = 0;
		return 0;
	}

	std::unique_lock< std::mutex > contextLock;
	auto context = _get_context( mFileIdentifier, contextLock );

	if ( nullptr == context )
	{
		mErrorCode = EBADF;
		return -1;
	}

	if ( FILE_CAN_READ( context ) )
	{
		struct timeval startTime, endTime;
		int64_t bytesRead;

		gettimeofday( &startTime, nullptr );
		bytesRead = context->_F_read( context, buffer, count, false );
		gettimeofday( &endTime, nullptr );

		_update_io_stats( context->_M_SumInverseRates[ FILE_IO_STATS_READ ],
			context->_M_NumberObservations[ FILE_IO_STATS_READ ], startTime, endTime, bytesRead );

		return bytesRead;
	}

	mErrorCode = ENOTSUP;
	return -1;
}

int64_t File::read(
	uint8_t* buffer,
	uint32_t count )
{
	if ( 0 == mFileIdentifier )
	{
		mErrorCode = EBADF;
		return -1;
	}

	if ( nullptr == buffer )
	{
		mErrorCode = ( 0 != count ) ? EINVAL : 0;
		return -( 0 != count );
	}

	if ( 0 == count )
	{
		mErrorCode = 0;
		return 0;
	}

	std::unique_lock< std::mutex > contextLock;
	auto context = _get_context( mFileIdentifier, contextLock );

	if ( nullptr == context )
	{
		mErrorCode = EBADF;
		return -1;
	}

	if ( FILE_CAN_READ( context ) )
	{
		struct timeval startTime, endTime;
		int64_t bytesRead;

		gettimeofday( &startTime, nullptr );
		bytesRead = context->_F_read( context, buffer, count, true );
		gettimeofday( &endTime, nullptr );

		_update_io_stats( context->_M_SumInverseRates[ FILE_IO_STATS_READ ],
			context->_M_NumberObservations[ FILE_IO_STATS_READ ], startTime, endTime, bytesRead );

		return bytesRead;
	}

	mErrorCode = ENOTSUP;
	return -1;
}

int64_t File::write(
	const uint8_t* buffer,
	uint32_t count )
{
	if ( 0 == mFileIdentifier.load() )
	{
		mErrorCode = EBADF;
		return -1;
	}

	if ( nullptr == buffer )
	{
		mErrorCode = ( 0 != count ) ? EINVAL : 0;
		return -( 0 != count );
	}

	if ( 0 == count )
	{
		mErrorCode = 0;
		return 0;
	}

	std::unique_lock< std::mutex > contextLock;
	auto context = _get_context( mFileIdentifier, contextLock );

	if ( nullptr == context )
	{
		mErrorCode = EBADF;
		return -1;
	}

	if ( FILE_CAN_WRITE( context ) )
	{
		struct timeval startTime, endTime;
		int64_t bytesWritten;

		gettimeofday( &startTime, nullptr );
		bytesWritten = context->_F_write( context, buffer, count, false );
		gettimeofday( &endTime, nullptr );

		_update_io_stats( context->_M_SumInverseRates[ FILE_IO_STATS_WRITE ],
			context->_M_NumberObservations[ FILE_IO_STATS_WRITE ], startTime, endTime, bytesWritten );

		return bytesWritten;
	}

	mErrorCode = ENOTSUP;
	return -1;
}
