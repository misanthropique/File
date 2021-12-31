/**
 * Copyright ©2021. Brent Weichel. All Rights Reserved.
 * Permission to use, copy, modify, and/or distribute this software, in whole
 * or part by any means, without express prior written agreement is prohibited.
 */
#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "File.hpp"
#include "FileContext.hpp"
#include "scheme_file.hpp"

struct SchemeFileContext
{
	int mFileHandle;
	int mErrorCode;
};

struct SchemeFileContext* __allocate_scheme_file_context()
{
	struct SchemeFileContext* schemeContext = nullptr;

	schemeContext = static_cast< struct SchemeFileContext* >( calloc( 1, sizeof( struct SchemeFileContext ) ) );

	if ( nullptr != schemeContext )
	{
		schemeContext->mFileHandle = -1;
	}

	return schemeContext;
}

void __free_scheme_file_context(
	struct SchemeFileContext* context )
{
	memset( context, 0, sizeof( struct SchemeFileContext ) );
	free( context );
}

// TODO:
// [ ] Handle Regular File
// [ ] Handle Character Device
// [ ] Handle FIFO/Pipe
// [ ] Perform Memory Mapped IO for Regular Files

// For the moment, assume that we're dealing with
// a regular file in a Unix environment.
bool __scheme_file_open(
	struct FileContext* context,
	const std::string& uri,
	File::IOFlag mode,
	int& errorCode )
{
	static const char SCHEME_FILE_PREFIX[] = "file://";

	if ( nullptr == context )
	{
		errorCode = EBADF;
		return false;
	}

	if ( nullptr != context->_M_SchemeContext )
	{
		errorCode = ESTALE;
		return false;
	}

	// Check that the mode is set to READ, WRITE, or both.
	if ( not ( File::IOFlag::READ & mode )
		and not ( File::IOFlag::WRITE & mode ) )
	{
		errorCode = EINVAL;
		return false;
	}

	// Check that the URI starts with "file://"
	if ( 0 != strncmp( SCHEME_FILE_PREFIX, uri.c_str(), sizeof( SCHEME_FILE_PREFIX ) - 1 ) )
	{
		errorCode = EINVAL;
		return false;
	}

	int flags = O_CREAT | O_SYNC |
		( ( File::IOFlag::READ & mode )
			? ( ( File::IOFlag::WRITE & mode ) ? O_RDWR : O_RDONLY )
			: O_WRONLY );
	int defaultMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

	struct SchemeFileContext* schemeContext = __allocate_scheme_file_context();

	if ( nullptr == schemeContext )
	{
		errorCode = ENOMEM;
		return false;
	}

	schemeContext->mFileHandle = open( uri.c_str() + sizeof( SCHEME_FILE_PREFIX ) - 1, flags, defaultMode );

	if ( -1 == schemeContext->mFileHandle )
	{
		errorCode = errno;
		__free_scheme_file_context( schemeContext );
		return false;
	}

	// stat the file
	struct stat fileStatus;
	if ( -1 == fstat( schemeContext->mFileHandle, &fileStatus ) )
	{
		errorCode = errno;
		__free_scheme_file_context( schemeContext );
		return false;
	}

	// TODO: Update the capabilities

	switch ( S_IFMT & fileStatus.st_mode )
	{
	case S_IFREG:
		context->_M_FileSize = fileStatus.st_size;
	case S_IFCHR:
	case S_IFIFO:
		context->_M_FileSize = -1;
	}

	context->_M_SchemeContext = static_cast< void* >( schemeContext );
	return true;
}

int64_t __scheme_file_read(
	struct FileContext* context,
	uint8_t* buffer,
	uint32_t bytes,
	bool updatePosition )
{
	if ( nullptr == context )
	{
		return -1;
	}

	if ( nullptr == context->_M_SchemeContext )
	{
		context->_M_ErrorCode = EIDRM;
		return -1;
	}

	struct SchemeFileContext* schemeContext = static_cast< struct SchemeFileContext* >( context->_M_SchemeContext );
	ssize_t bytesRead = -1;

	if ( updatePosition )
	{
		bytesRead = read( schemeContext->mFileHandle, buffer, bytes );
	}
	else
	{
		bytesRead = pread( schemeContext->mFileHandle, buffer, bytes, context->_M_FilePosition );
	}

	if ( -1 == bytesRead )
	{
		schemeContext->mErrorCode = errno;
	}

	return static_cast< int64_t >( bytesRead );
}

int64_t __scheme_file_write(
	struct FileContext* context,
	const uint8_t* buffer,
	uint32_t bytes,
	bool append )
{
	if ( nullptr == context )
	{
		return -1;
	}

	if ( nullptr == context->_M_SchemeContext )
	{
		context->_M_ErrorCode = EIDRM;
		return -1;
	}

	struct SchemeFileContext* schemeContext = static_cast< struct SchemeFileContext* >( context->_M_SchemeContext );
	ssize_t bytesWritten = -1;

	if ( append )
	{
		bytesWritten = pwrite( schemeContext->mFileHandle, buffer, bytes, context->_M_FileSize );

		if ( -1 != byteWritten )
		{
			context->_M_FileSize += bytesWritten;
		}
	}
	else
	{
		bytesWritten = write( schemeContext->mFileHandle, buffer, bytes );

		if ( -1 != bytesWritten )
		{
			context->_M_FilePosition += bytesWritten;
			context->_M_FileSize = std::max( context->_M_FileSize, context->_M_FilePosition );
		}
	}

	if ( -1 == bytesWritten )
	{
		schemeContext->mErrorCode = errno;
	}

	return static_cast< int64_t >( bytesWritten );
}

int64_t __scheme_file_seek(
	struct FileContext* context,
	int64_t offset,
	bool relative )
{
}

bool __scheme_file_resize(
	struct FileContext* context,
	int64_t size,
	uint8_t fill,
	bool grown,
	bool shrink )
{
}

std::string __scheme_file_error_string(
	struct FileContext* context )
{
	if ( nullptr == context )
	{
		return std::string( strerror( EBADF ) );
	}

	if ( nullptr == context->_M_SchemeContext )
	{
		return std::string( strerror( EIDRM ) );
	}

	struct SchemeFileContext* schemeContext = static_cast< struct SchemeFileContext* >( context->_M_SchemeContext );

	if ( 0 != schemeContext->mErrorCode )
	{
		return std::string( strerror( schemeContext->mErrorCode ) );
	}

	return std::string();
}

void __scheme_file_close(
	struct FileContext* context )
{
	if ( ( nullptr == context )
		or ( nullptr == context->_M_SchemeContext ) )
	{
		return;
	}

	struct SchemeFileContext* schemeContext = static_cast< struct SchemeFileContext* >( context->_M_SchemeContext );
	// TODO: Catch the error for close
	close( schemeContext->mFileHandle );
	__free_scheme_file_context( schemeContext );
	context->_M_SchemeContext = nullptr;
}
