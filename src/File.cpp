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
	File::IOFlag mode,
	int& errorCode )
{
	std::string normalizedFilepath, scheme;

	if ( not _normalize_filepath( normalizedFilepath, filepath ) )
	{
		errorCode = EINVAL;
		return 0;
	}

	struct FileContext* context = _allocate_context();

	if ( not _open_uri( context, normalizedFilepath, mode, errorCode ) )
	{
		_free_context( context );
		return 0;
	}

	uint64_t identifier = _assign_identifier( context );

	if ( 0 != identifier )
	{
		return identifier;
	}

	_free_context( context );
}

static void __close_file(
	uint64_t fileIdentifier )
{
	if ( 0 != fileIdentifier )
	{
	}
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
	uint64_t fileIdentifier = __open_file( filepath, mode, mErrorCode );

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
		bool ignoreIOStats = false;
		struct timeval startTime, endTime;
		int64_t bytesWritten;

		if ( 0 != gettimeofday( &startTime, nullptr ) )
		{
			ignoreIOStats = true;
		}

		bytesWritten = context->_F_write( context, buffer, count, true );

		if ( ( not ignoreIOStats ) and ( 0 != gettimeofday( &endTime, nullptr ) ) )
		{
			ignoreIOStats = true;
		}

		if ( not ignoreIOStats )
		{
			_update_io_stats( context, FILE_IO_STATS_WRITE, startTime, endTime, bytesWritten );
		}

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

void File::close()
{
	
}

std::string File::errorMessage(
	bool clearAfterRead )
{
	
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
		bool ignoreIOStats = false;
		struct timeval startTime, endTime;
		int64_t bytesRead;

		if ( 0 != gettimeofday( &startTime, nullptr ) )
		{
			ignoreIOStats = true;
		}

		bytesRead = context->_F_read( context, buffer, count, false );

		if ( ( not ignoreIOStats ) and ( 0 != gettimeofday( &endTime, nullptr ) ) )
		{
			ignoreIOStats = true;
		}

		if ( not ignoreIOStats )
		{
			_update_io_stats( context, FILE_IO_STATS_READ, startTime, endTime, bytesRead );
		}

		return bytesRead;
	}

	mErrorCode = ENOTSUP;
	return -1;
}

int64_t File::position() const
{
	if ( 0 == mFileIdentifier )
	{
		mErrorCode = EBADF;
		return -1;
	}

	std::unique_lock< std::mutex > contextLock;
	auto context = _get_context( mFileIdentifier, contextLock );

	if ( nullptr == context )
	{
		mErrorCode = EBADF;
		return -1;
	}

	return context->_M_FilePosition;
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
		bool ignoreIOStats = false;
		struct timeval startTime, endTime;
		int64_t bytesRead;

		if ( 0 != gettimeofday( &startTime, nullptr ) )
		{
			ignoreIOStats = true;
		}

		bytesRead = context->_F_read( context, buffer, count, true );

		if ( ( not ignoreIOStats ) and ( 0 != gettimeofday( &endTime, nullptr ) ) )
		{
			ignoreIOStats = true;
		}

		if ( not ignoreIOStats )
		{
			_update_io_stats( context, FILE_IO_STATS_READ, startTime, endTime, bytesRead );
		}

		return bytesRead;
	}

	mErrorCode = ENOTSUP;
	return -1;
}

bool File::reserve(
	int64_t size,
	uint8_t fill )
{
	if ( 0 > size )
	{
		mErrorCode = EINVAL;
		return false;
	}

	std::unique_lock< std::mutex > contextLock;
	auto context = _get_context( mFileIdentifier, contextLock );

	if ( nullptr == context )
	{
		mErrorCode = EBADF;
		return false;
	}

	if ( size <= context->_M_FileSize )
	{
		return true;
	}

	if ( FILE_CAN_WRITE( context ) )
	{
		bool ignoreIOStats = false;
		struct timeval startTime, endTime;
		int64_t newFileSize;

		if ( 0 != gettimeofday( &startTime, nullptr ) )
		{
			ignoreIOStats = true;
		}

		newFileSize = context->_F_resize( context, size, fill, false, true );

		if ( ( not ignoreIOStats ) and ( 0 != gettimeofday( &endTime, nullptr ) ) )
		{
			ignoreIOStats = true;
		}

		if ( newFileSize > context->_M_FileSize )
		{
			if ( not ignoreIOStats )
			{
				_update_io_stats( context, FILE_IO_STATS_WRITE, startTime, endTime, newFileSize - context->_M_FileSize );
			}

			context->_M_FileSize = newFileSize;
			return true;
		}

		return false;
	}

	mErrorCode = ENOTSUP;
	return false;
}

bool File::resize(
	int64_t size,
	uint8_t fill )
{
	if ( 0 > size )
	{
		mErrorCode = EINVAL;
		return false;
	}

	std::unique_lock< std::mutex > contextLock;
	auto context = _get_context( mFileIdentifier, contextLock );

	if ( nullptr == context )
	{
		mErrorCode = EBADF;
		return false;
	}

	if ( size == context->_M_FileSize )
	{
		return true;
	}

	if ( FILE_CAN_WRITE( context ) )
	{
		bool ignoreIOStats = false;
		struct timeval startTime, endTime;
		int64_t newFileSize;

		if ( 0 != gettimeofday( &startTime, nullptr ) )
		{
			ignoreIOStats = true;
		}

		newFileSize = context->_F_resize( context, size, fill, true, true );

		if ( ( not ignoreIOStats ) and ( 0 != gettimeofday( &endTime, nullptr ) ) )
		{
			ignoreIOStats = true;
		}

		if ( newFileSize > context->_M_FileSize )
		{
			if ( not ignoreIOStats )
			{
				_update_io_stats( context, FILE_IO_STATS_WRITE, startTime, endTime, newFileSize - context->_M_FileSize );
			}
		}

		context->_M_FileSize = newFileSize;
		return size == newFileSize;
	}

	mErrorCode = ENOTSUP;
	return false;
}

int64_t File::size() const
{
	std::unique_lock< std::mutex > contextLock;
	auto context = _get_context( mFileIdentifier, contextLock );

	if ( nullptr == context )
	{
		mErrorCode = EBADF;
		return -1;
	}

	return context->_M_FileSize;
}

bool File::truncate(
	int64_t size )
{
	if ( 0 > size )
	{
		mErrorCode = EINVAL;
		return false;
	}

	std::unique_lock< std::mutex > contextLock;
	auto context = _get_context( mFileIdentifier, contextLock );

	if ( nullptr == context )
	{
		mErrorCode = EBADF;
		return false;
	}

	if ( size >= context->_M_FileSize )
	{
		return true;
	}

	if ( FILE_CAN_WRITE( context ) )
	{
		int64_t newFileSize;

		newFileSize = context->_F_resize( context, size, fill, true, false );

		context->_M_FileSize = newFileSize;
		return size == newFileSize;
	}

	mErrorCode = ENOTSUP;
	return false;
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
		bool ignoreIOStats = false;
		struct timeval startTime, endTime;
		int64_t bytesWritten;

		if ( 0 != gettimeofday( &startTime, nullptr ) )
		{
			ignoreIOStats = true;
		}

		bytesWritten = context->_F_write( context, buffer, count, false );

		if ( ( not ignoreIOStats ) and ( 0 != gettimeofday( &endTime, nullptr ) ) )
		{
			ignoreIOStats = true;
		}

		if ( not ignoreIOStats )
		{
			_update_io_stats( context, FILE_IO_STATS_WRITE, startTime, endTime, bytesWritten );
		}

		return bytesWritten;
	}

	mErrorCode = ENOTSUP;
	return -1;
}
