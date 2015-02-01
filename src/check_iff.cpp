/**
 * @file   check_iff.cpp
 * @brief  Check for standard IFF files.
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

bool check_iff(const uint8_t *content, unsigned long len, Match *mc)
{
	REQUIRE(content, "FORM");
	uint32_t lenChunk = as_u32be(content + 4);

	// Ignore files >16MB as they are probably false positives
	if (lenChunk > 16777216) return false;

	// Chunk sizes must be a multiple of two
	if (lenChunk % 2) lenChunk++;

	lenChunk += 8; // include header
	if (lenChunk > len) return false;

	std::string type((const char *)content + 8, 4);

	mc->len = lenChunk;
	if (type.compare("XDIR") == 0) {
		// This format has a second IFF appended
		uint32_t lenChunk2 = as_u32be(content + lenChunk + 4);

		// Chunk sizes must be a multiple of two
		if (lenChunk2 % 2) lenChunk2++;

		lenChunk2 += 8; // include header
		if (lenChunk2 > len) return false;

		mc->len += lenChunk2;

		mc->cat = check::Music;
		mc->ext = "xmi";
		mc->desc = "Miles eXtended MIDI";
	} else if (type.compare("ILBM") == 0) {
		mc->cat = check::Image;
		mc->ext = "lbm";
		mc->desc = "InterLeaved BitMap";
	} else if (type.compare("AIFF") == 0) {
		mc->cat = check::Audio;
		mc->ext = "aiff";
		mc->desc = "Audio Interchange File Format";
	} else {
		// Exclude anything with control or extended characters in the type
		// field.
		for (unsigned int i = 0; i < 4; i++) {
			if ((type[i] < 20) || (type[i] > 126)) return false;
		}
		mc->cat = check::Other;
		mc->ext = "iff";
		mc->desc = "Unknown IFF";
	}
	return true;
}
