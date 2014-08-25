/**
 * @file   main.cpp
 * @brief  Ripper 6 entry point.
 *
 * Copyright (C) 2014 Adam Nielsen <malvineous@shikadi.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <camoto/iostream_helpers.hpp>

using namespace camoto;

struct check {
	enum MatchCategory {
		Unknown = 0,
		Audio,
		Image,
		Music,
		Video,
		Other
	};
};
struct Match {
	unsigned long len;
	check::MatchCategory cat;
	std::string ext;
	std::string desc;
};

/// Check for this format
/**
 * @param content
 *   The block to look at.  Do not scan the block, simply look at a
 *   fixed offset.  This function will be called repeatedly, once at each
 *   byte offset.  If a match is found, then you can scan around to find
 *   the start of the file, looking back at most getMinHead() bytes before
 *   this pointer, and forward at most len bytes.
 *
 * @param len
 *   Maximum distance to search past *content.  Will always be >=
 *   getMinTail().
 *
 * @param mc
 *   Details about any match, if the function returns true.  Ignored if the
 *   return value is false.
 *
 * @return false if content does not point to an instance of this file
 *   format, true if it does and mc has been filled in with details about
 *   the match.
 */
typedef bool (*CheckFunction)(const uint8_t *content, unsigned long len, Match *mc);

inline uint16_t as_u16le(const uint8_t *content)
{
	return le16toh(*((uint16_t *)content));
}

inline uint32_t as_u32le(const uint8_t *content)
{
	return le32toh(*((uint32_t *)content));
}

inline uint16_t as_u16be(const uint8_t *content)
{
	return be16toh(*((uint16_t *)content));
}

inline uint32_t as_u32be(const uint8_t *content)
{
	return be32toh(*((uint32_t *)content));
}

/// Require the string, which can contain embedded nulls, be at the given offset
/**
 * @param c
 *   Pointer to the content to compare, e.g. content + 5.
 *
 * @param v
 *   String that must exist, e.g. "\x00\x11\x22"
 *
 * @post Returns from the check function if there was no match.  Execution only
 *   continues beyond the macro if the string matched.
 */
#define REQUIRE(c, v) { \
	const uint8_t *t = c; \
	const uint8_t *vp = (const uint8_t *)v; \
	unsigned long l = sizeof(v) - 1; \
	while (l--) { \
		if (*t++ != *vp++) return false; \
	} \
}

/// Require the byte at the given offset be within the given range.
/**
 * @param i
 *   Offset into content, e.g. 0.
 *
 * @param min
 *   Minimum allowed value, e.g. 0.
 *
 * @param max
 *   Maximum allowed value, e.g. 255.
 *
 * @post Returns from the check function if the value is outside the given
 *   range.  Execution only continues beyond the macro if the value was in range.
 */
#define REQUIRE_RANGE(i, min, max) \
	if ((i < min) || (i > max)) return false;

#include "check_cdfm.cpp"
#include "check_midi.cpp"
#include "check_tbsa.cpp"

int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cerr << "Must specify file to search." << std::endl;
		return 1;
	}
	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		std::cerr << "Unable to open " << argv[1] << ": " << strerror(errno) << std::endl;
		return 2;
	}

	struct stat s;
	fstat(fd, &s);
	uint8_t *content = (uint8_t *)mmap(0, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (content == MAP_FAILED) {
		std::cerr << "Unable to mmap() file: " << strerror(errno) << std::endl;
		return 2;
	}

	unsigned long matchCount = 0;

	std::vector<CheckFunction> checkFunctions;
	checkFunctions.push_back(check_cdfm);
	checkFunctions.push_back(check_midi);
	checkFunctions.push_back(check_tbsa);

	uint8_t *cp = content;
	uint8_t *end = content + s.st_size;
	unsigned long lenRemaining = s.st_size;
	Match match;
	while (cp < end) {
		if ((unsigned long)cp % 4096 == 0) {
			unsigned long offset = cp - content;
			std::cout << "\rSearching... " << offset << " bytes ("
				<< offset * 100 / s.st_size << "%)" << std::flush;
		}
		for (std::vector<CheckFunction>::iterator
			c = checkFunctions.begin(); c != checkFunctions.end(); c++
		) {
			if ((*c)(cp, lenRemaining, &match)) {
				std::stringstream ss;
				ss << std::setw(4) << std::setfill('0') << matchCount << '.' << match.ext;
				std::cout << "\e[2K\rFound match " << std::hex << match.len
					<< "@" << (cp - content) << std::dec << ": writing " << ss.str()
					<< " [";
				switch (match.cat) {
					case check::Unknown: std::cout << "?"; break;
					case check::Audio: std::cout << "audio"; break;
					case check::Image: std::cout << "image"; break;
					case check::Music: std::cout << "music"; break;
					case check::Video: std::cout << "video"; break;
					case check::Other: std::cout << "other"; break;
				}
				std::cout << "; " << match.desc << "]" << std::endl;

				int fdmatch = open(ss.str().c_str(), O_RDWR | O_CREAT, 0644);
				if (fdmatch < 0) {
					std::cerr << "Unable to open output file: " << strerror(errno) << std::endl;
					return 2;
				}
				ftruncate(fdmatch, match.len);

				uint8_t *matchContent = (uint8_t *)mmap(0, match.len, PROT_WRITE, MAP_SHARED, fdmatch, 0);
				if (matchContent == MAP_FAILED) {
					std::cerr << "Unable to mmap() output file: " << strerror(errno) << std::endl;
					return 2;
				}
				memcpy(matchContent, cp, match.len);
				munmap(matchContent, match.len);
				close(fdmatch);
				matchCount++;
				cp += match.len - 1;
				lenRemaining -= match.len - 1;
				break;
			}
		}
		cp++;
		lenRemaining--;
	}
	std::cout << "\e[2K\rComplete.  " << s.st_size << " bytes (100%)" << std::endl;

	munmap(content, s.st_size);
	close(fd);
	return 0;
}
