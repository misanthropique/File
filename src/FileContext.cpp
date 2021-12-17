/**
 * Copyright ©2021. Brent Weichel. All Rights Reserved.
 * Permission to use, copy, modify, and/or distribute this software, in whole
 * or part by any means, without express prior written agreement is prohibited.
 */
#include <cstdint>
#include <mutex>
#include <sys/time.h>

#include "File.hpp"
#include "FileContext.hpp"

static std::map< std::string, uint64_t > _G_UriToFileIdentifierMap;

/*
 * Normalize the filepath to a URI.
 * @param filepath The file path to be normalized to a URI.
 * @return A URI is returned.
 */
std::string __normalize_filepath(
	const std::string& filepath )
{
}

uint64_t _create_context(
	const std::string& filepath,
	File::IOFlag mode )
{
}

struct FileContext* _get_context(
	uint64_t fileIdentifier,
	std::unique_lock< std::mutex >& contextLock )
{
}

void _release_context(
	uint64_t fileIdentifier )
{
}

void _update_io_stats(
	double& sumInverseRates,
	uint64_t& numberObservations,
	struct timeval& startTime,
	struct timeval& endTime,
	int64_t bytes )
{
}
