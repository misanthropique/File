/**
 * Copyright ©2021. Brent Weichel. All Rights Reserved.
 * Permission to use, copy, modify, and/or distribute this software, in whole
 * or part by any means, without express prior written agreement is prohibited.
 */
#pragma once

/*
 * Normalize the given filepath to a URI.
 * Relative local paths will be converted to absolute
 * file URIs, and absolute paths will be prefixed with "file://"
 * If the filepath is neither a local file, nor a valid URI, then
 * false shall be returned and normalizedFilepath shall be left untouched.
 */
bool _normalize_filepath(
	std::string& normalizedFilepath,
	const std::string& filepath );

/**
 * We shall assume that {@param uri} is a valid URI.
 * Return the scheme of the URI.
 */
std::string _get_scheme(
	const std::string uri );
