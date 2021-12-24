/**
 * Copyright ©2021. Brent Weichel. All Rights Reserved.
 * Permission to use, copy, modify, and/or distribute this software, in whole
 * or part by any means, without express prior written agreement is prohibited.
 */
#pragma once

#include <cstdint>
#include <string>

#include "File.hpp"
#include "FileContext.hpp"
#include "Scheme.hpp"

bool __scheme_file_open(
	struct FileContext* context,
	const std::string& uri,
	File::IOFlag mode,
	int& errorCode );

int64_t __scheme_file_read(
	struct FileContext* context,
	uint8_t* buffer,
	uint32_t bytes,
	bool updatePosition );

int64_t __scheme_file_write(
	struct FileContext* context,
	const uint8_t* buffer,
	uint32_t bytes,
	bool append );

int64_t __scheme_file_seek(
	struct FileContext* context,
	int64_t offset,
	bool relative );

bool __scheme_file_resize(
	struct FileContext* context,
	int64_t size,
	uint8_t fill,
	bool grown,
	bool shrink );

std::string __scheme_file_error_string(
	struct FileContext* context );

void __scheme_file_close(
	struct FileContext* context );

const struct SchemeAPI SCHEME_FILE_API =
{
	._F_open = __scheme_file_open,
	._F_read = __scheme_file_read,
	._F_write = __scheme_file_write,
	._F_seek = __scheme_file_seek,
	._F_resize = __scheme_file_resize,
	._F_error_string = __scheme_file_error_string,
	._F_close = __scheme_file_close
};
