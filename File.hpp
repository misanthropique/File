/**
 * Copyright ©2021. Brent Weichel. All Rights Reserved.
 * Permission to use, copy, modify, and/or distribute this software, in whole
 * or part by any means, without express prior written agreement is prohibited.
 */
#pragma once
 
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

	File() noexcept;

	File( const std::string& filePath, File::IOFlag mode );

	File( const File& other );

	File( File&& other ) noexcept;

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
	 * Length of the file in bytes.
	 */
	int64_t size() const;

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
