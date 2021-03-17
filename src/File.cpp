/**
 * Copyright ©2021. Brent Weichel. All Rights Reserved.
 * Permission to use, copy, modify, and/or distribute this software, in whole
 * or part by any means, without express prior written agreement is prohibited.
 */
#include <cerrno>
#include <cstdint>
#include <mutex>

#include "File.hpp"

File::File() noexcept :
	mFileIdentifier( 0 )
{
}

int64_t File::append( const uint8_t* buffer, uint32_t count )
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

int64_t File::peek( uint8_t* buffer, uint32_t count )
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

int64_t File::read( uint8_t* buffer, uint32_t count )
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

int64_t File::write( const uint8_t* buffer, uint32_t count )
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
