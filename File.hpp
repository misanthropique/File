/**
 * Copyright ©2021. Brent Weichel. All Rights Reserved.
 * Permission to use, copy, modify, and/or distribute this software, in whole
 * or part by any means, without express prior written agreement is prohibited.
 */
#pragma once

#include <atomic>
#include <cstdint>
#include <string>

/*
 * TODO:
 * [ ] lock
 * [ ] pread
 * [ ] pwrite
 * [ ] UserCredentials
 */

/**
 * A class for abstracting the details of files regardless of type, location, or scheme.
 * I'm tired of having to use different interfaces for local versus remote files. Sure I'll have
 * to implement the details here, but at least I'll have this available to me in the future
 * when I need it.
 */
class File
{
private:
	std::atomic_uint64_t mFileIdentifier;

	// This member variable is for errors that occur
	// at the interface layer, before going down into the
	// protocol specific codes; which are stored in the
	// file context object.
	int mErrorCode;

public:
	enum IOFlag : uint32_t
	{
		READ = 0x1,
		WRITE = 0x2,
		SEEK = 0x4
	};

	/**
	 * Default constructor to a null file handle.
	 */
	File() noexcept;

	/**
	 * Constructor to open a file.
	 * @param filepath Path to the file to be opened.
	 * @param mode IOFlag of the file to be opened.
	 */
	File( const std::string& filepath, File::IOFlag mode );

	/**
	 * File copy constructor.
	 * @param other Const reference to the File instance to copy.
	 */
	File( const File& other );

	/**
	 * File move constructor.
	 * @param other R-Value to the File instance to move to this instance.
	 */
	File( File&& other ) noexcept;

	/**
	 * File destructor shall release all resources.
	 */
	~File();

	/**
	 * Write to the end of the file the requested number of bytes
	 * without updating the file position.
	 * @param buffer Pointer to an array of const bytes.
	 * @param count The number of byte to write from {@param buffer}.
	 * @return The number of bytes appended to the file is returned. This may be less
	 *         than the requested number of bytes, and the cuase can be retrieved via
	 *         errorMessage(). On error, -1 is returned and the error message can
	 *         be retrieved via errorMessage().
	 */
	int64_t append( const uint8_t* buffer, uint32_t count );

	/**
	 * Average of the observed read byte-rate (bytes per second).
	 * @param ioFlag Flag indicating which byte-rate to return. If both Read and Write
	 *               are set, then the Read byte-rate is returned. If neither Read nor
	 *               Write are set, then NaN is returned. [default: File::IOFlag::Read]
	 * @return An average of bytes read/written per second. NaN is returned on error,
	 *         and zero indicates either nothing has been read/written, or you're using AvianIP (RFC-1149).
	 */
	double byteRate( File::IOFlag ioFlag = File::IOFlag::Read ) const;

	/**
	 * If applicable, close the file and release the resources.
	 * Should no file be open, this method does nothing.
	 */
	void close();

	/**
	 * Get the current error message.
	 * @param clearAfterRead Clear the error code after reading if set to true. [default: true]
	 * @return A string containing the error message is returned.
	 */
	std::string errorMessage( bool clearAfterRead = true );

	/**
	 * Open a file to this File instance.
	 * @param filepath Path to a file or a URI.
	 * @param mode Mode in which to open the file.
	 * @return True is returned upon opening the file successfully. The error shall be set if false is returned.
	 */
	bool open( const std::string& filepath, File::IOFlag mode );

	/**
	 * File move assignment operator.
	 * @param other R-Value to a File instance to move to this instance.
	 * @return Reference to this File instance is returned.
	 */
	File& operator=( File&& other );

	/**
	 * File copy assignment operator.
	 * @param other Const reference to a File instance to copy to this instance.
	 * @return Reference to this File instance is returned.
	 */
	File& operator=( const File& other );

	/**
	 * Read from the file the requested number of bytes without updating the file position.
	 * @param buffer Pointer to a byte array large enough to hold the requested data.
	 * @param count The number of bytes to read into {@param buffer}.
	 * @return The number of bytes read from the file is returned. This value may be
	 *         less than the requested number of bytes, and the cause can be retrieved
	 *         via errorMessage(). On error, -1 is returned and the error message can
	 *         be retrieved via errorMessage().
	 */
	int64_t peek( uint8_t* buffer, uint32_t count );

	/**
	 * Get the current file position from the beginning of the file, in bytes.
	 * @return The byte offset from the beginning of the file is returned.
	 */
	int64_t position() const;

	/**
	 * Read from the file the requested number of bytes and
	 * update the file position by the corresponding count.
	 * @param buffer Pointer to a byte array large enough to hold the requested data.
	 * @param count The number of bytes to read into {@param buffer}.
	 * @return The number of bytes read from the file is returned. This value may be
	 *         less than the requested number of bytes, and the cause can be retrieved
	 *         via errorMessage(). On error, -1 is returned and the error message can
	 *         be retrieved via errorMessage().
	 */
	int64_t read( uint8_t* buffer, uint32_t count );

	/**
	 * Reserve the requested number of bytes for the file size.
	 * If the requested size is less than the current file size,
	 * then this method does nothing. If the requested size is
	 * greater than the current file size, then the file size will be
	 * increased on disk to occupy the requested space. If there was
	 * an issue allocating the requested space, then false is returned
	 * and an error code will be assigned to the error code.
	 * @param size The number of bytes requested that the file ought to occupy.
	 * @param fill The byte value to fill in the new bytes.
	 * @return True is returned if the requested number of bytes occupies
	 *         the desired space on the drive. False is returned on error.
	 */
	bool reserve( int64_t size, uint8_t fill = '\0' );

	/**
	 * Resize the file to the desired number of bytes.
	 * @param size The length of the file in bytes.
	 * @param fill The byte value to fill in the new bytes.
	 * @return True is returned if the requested number of bytes occupies
	 *         the desired space on the drive. False is returned on error.
	 */
	bool resize( int64_t size, uint8_t fill = '\0' );

	/**
	 * Seek to the requested file offset.
	 * @param offset Offset from either the beginning of the file or the current position.
	 * @param relative When set to false, {@param offset} is from the beginning of the file
	 *                 if positive, else from the end of the file if negative. When set to true,
	 *                 {@param offset} is relative to the current file position. [default: false]
	 * @return The difference between the requested file offset and the set file offset is returned.
	 *         Negative one is returned on error and the error code is set.
	 */
	int64_t seek( int64_t offset, bool relative = false );

	/**
	 * Length of the file in bytes.
	 * @return Length of the file in bytes. If the length is indeterminate, then -1 is returned.
	 */
	int64_t size() const;

	/**
	 * Synchronize the File instance with its source.
	 * @return True is returned on success, else false is returned and the error code is set.
	 */
	bool sync();

	/**
	 * Truncate the file to the desired size. If {@param size} is less than
	 * the current file size, then the file is truncated. If {@param size} is
	 * greater than the current file size, then nothing is done.
	 * @param size Size of the file in bytes to truncate the file to.
	 * @return True is returned if the file has been truncated, false is returned
	 *         if nothing was done, or an error has occurred.
	 */
	bool truncate( int64_t size );

	/**
	 * Write to the file the requested number of bytes and
	 * update the file position by the corresponding count.
	 * @param buffer Pointer to an array of const bytes.
	 * @param count The number of byte to write from {@param buffer}.
	 * @return The number of bytes written to the file is returned. This may be less
	 *         than the requested number of bytes, and the cuase can be retrieved via
	 *         errorMessage(). On error, -1 is returned and the error message can
	 *         be retrieved via errorMessage().
	 */
	int64_t write( const uint8_t* buffer, uint32_t count );
};
