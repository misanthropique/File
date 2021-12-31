/**
 * Copyright ©2021. Brent Weichel. All Rights Reserved.
 * Permission to use, copy, modify, and/or distribute this software, in whole
 * or part by any means, without express prior written agreement is prohibited.
 */
#pragma once

#include <cstdint>
#include <string>

#include "FileContext.hpp"

struct SchemeAPI
{
	/**
	 * Open the URI into the provided context with the given mode.
	 * @param context Pointer to a FileContext struct.
	 * @param uri Const string reference to the URI of the resource.
	 * @param mode Mode in which to open the URI.
	 * @param errorCode Reference to the int to store non-scheme related error codes.
	 * @return True is returned upon successfully opening the resource, false on error.
	 */
	bool ( *_F_open )( struct FileContext*, const std::string&, File::IOFlag, int& );

	/**
	 * Get an error message specific to the scheme.
	 * @param context Pointer to a FileContext struct.
	 * @return A string containing the error message.
	 */
	std::string ( *_F_error_string )( struct FileContext* );

	/**
	 * Synchronize the resource and close the handle. If there are multiple
	 * references to the resource, then the count is decremented and nothing else happens.
	 * @param context Pointer to a FileContext struct.
	 */
	void ( *_F_close )( struct FileContext* );

	/**
	 * Seek to the requested offset if seeking is supported.
	 * @param context Pointer to a FileContext struct.
	 * @param offset This is either an absolute or relative offset depending upon the value
	 *               of {@param relative}. Negative absolute values are relative to the end of the file.
	 * @param relative If this is set to false, then we're requesting an absolute offset,
	 *                 otherwise an offset relative to the current resource position.
	 * @return The difference between the requested offset and the set file offset is returned.
	 */
	int64_t ( *_F_seek )( struct FileContext*, int64_t, bool );

	/**
	 * Read the requested number of bytes from the file at the current file position.
	 * @param context Pointer to a FileContext struct.
	 * @param buffer Pointer to a buffer to store bytes read.
	 * @param bytes The number of bytes to read into the given buffer.
	 * @param updatePosition Should the file position be updated by the number of bytes read.
	 * @return The number of bytes read from the resource.
	 */
	int64_t ( *_F_read )( struct FileContext*, uint8_t*, uint32_t, bool );

	/**
	 * Write the requested number of bytes to the file either from the current position or the end of the file.
	 * @param context Pointer to a FileContext struct.
	 * @param buffer Pointer to a buffer to read from.
	 * @param bytes The number of bytes to write from the buffer out to the resource.
	 * @param append Should the requested bytes be appended or written from the current file position.
	 * @return The number of bytes written out to the resource is returned.
	 */
	int64_t ( *_F_write )( struct FileContext*, const uint8_t*, uint32_t, bool );

	/**
	 * Resize the file to the requested number of bytes. There are 2 control flags
	 * that enable shrinking and growing the file should the requested size be less than
	 * or greater than the current file size, respectively.
	 * @param context Pointer to a FileContext struct.
	 * @param size The requested size of the file in bytes.
	 * @param fill The byte value to fill the new bytes with.
	 * @param shrink Is shrinking the file permitted.
	 * @param grow Is growing the file permitted.
	 * @return The new size of the file, in bytes, is returned.
	 */
	int64_t ( *_F_resize )( struct FileContext*, int64_t, uint8_t, bool, bool );

	/**
	 * Synchronize the contents of the memory buffer with the resource.
	 * @param context Pointer to a FileContext struct.
	 * @return False is returned on error, True is returned on success or no-op.
	 */
	bool ( *_F_sync )( struct FileContext* );
};
