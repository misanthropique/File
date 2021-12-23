/**
 * Copyright ©2021. Brent Weichel. All Rights Reserved.
 * Permission to use, copy, modify, and/or distribute this software, in whole
 * or part by any means, without express prior written agreement is prohibited.
 */
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <regex>
#include <string>

#include "Util.hpp"

static const std::regex __REGEX_URI_SCHEME_PREFIX( "^[a-zA-Z][a-zA-Z0-9+.-]*:.*$" );

bool _normalize_filepath(
	std::string& normalizedFilepath,
	const std::string& filepath )
{
	if ( 0 < filepath.size() )
	{
		if ( '/' == filepath[ 0 ] )
		{
			// absolute path
			normalizedFilepath = std::string( "file://" ) + filepath;
			return true;
		}
		else if ( std::regex_match( filepath, __REGEX_URI_SCHEME_PREFIX ) )
		{
			// looks like a URI
			normalizedFilepath = filepath;
			return true;
		}
		else
		{
			char* absolutePath = realpath( filepath.c_str(), nullptr );

			if ( nullptr == absolutePath )
			{
				// Set the error code and return false.
				errorCode = errno;
				return false;
			}

			normalizedPath = std::string( "file://" ) + std::string( absolutePath );
			free( absolutePath );
			return true;
		}
	}

	return false;
}

std::string _get_scheme(
	const std::string uri )
{
	std::string canonicalizedScheme;
	std::string scheme = uri.substr( 0, uri.find( ':' ) );
	// The canonical form of any URI scheme is lowercase,
	// so transform to lowercase before returning the scheme.
	std::transform( scheme.begin(), scheme.end(), canonicalizedScheme.begin(),
		[]( unsigned char character ) -> unsigned char { return std::tolower( character ); } );
	return canonicalizedScheme;
}
