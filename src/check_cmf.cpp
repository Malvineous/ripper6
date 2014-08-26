/**
 * @file   check_cmf.cpp
 * @brief  Check for Creative Music Files.
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

/// Maximum size of a CMF file (16-bit pointer)
#define CMF_MAX_SIZE (65536 + 256*1024)

bool check_cmf(const uint8_t *content, unsigned long len, Match *mc)
{
	// Too short
	if (len < 37) return false;

	REQUIRE(content, "CTMF");

	// Only known versions are 1.0 and 1.1
	unsigned long version = as_u16le(content + 4);
	if ((version != 0x0100) && (version != 0x0101)) return false;

	unsigned long offInst = as_u16le(content + 6);
	unsigned long offMusic = as_u16le(content + 8);
	unsigned long offTag1 = as_u16le(content + 14);
	unsigned long offTag2 = as_u16le(content + 16);
	unsigned long offTag3 = as_u16le(content + 18);
	unsigned long numInst;
	if (version == 0x100) numInst = content[36];
	else numInst = as_u16le(content + 36);

	unsigned long size = 37; // header
	size = std::max(size, offInst + numInst * 16);
	size = std::max(size, offMusic);
	size = std::max(size, offTag1);
	size = std::max(size, offTag2);
	size = std::max(size, offTag3);
	if (len < size) return false;

	// Parse the music to find the end of the file
	unsigned long endMusic = 0;
	const uint8_t *music = content + offMusic;
	for (unsigned long i = 0; i < CMF_MAX_SIZE; i++) {
		if (*music == 0xFF) {
			// Found a meta event
			if (music[1] == 0x2F) {
				// End of track
				endMusic = (music - content) + 3; // include 0x00 after the 0x2F
				break;
			}
		}
		music++;
	}
	if (endMusic == 0) return false; // couldn't find end-of-track marker
	size = std::max(size, endMusic);

	mc->len = size;
	mc->cat = check::Music;
	mc->ext = "cmf";
	mc->desc = "Creative Music File";
	return true;
}
