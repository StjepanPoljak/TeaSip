#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <regex>

#define REGEX(r) std::regex_match(input, match, std::regex(r))

/* inspired by answer on SO */
std::string trim(const std::string& string)
{
	size_t first, last;
	
	first = string.find_first_not_of(' ');

	if (std::string::npos == first)
		return string;

	last = string.find_last_not_of(' ');

	return string.substr(first, (last - first + 1));
}

#endif
