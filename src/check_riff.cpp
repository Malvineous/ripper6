/**
 * @file   check_riff.cpp
 * @brief  Check for standard RIFF files.
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

bool check_riff(const uint8_t *content, unsigned long len, Match *mc)
{
	REQUIRE(content, "RIFF");
	uint32_t lenChunk = as_u32le(content + 4);
	if (lenChunk + 8 > len) return false;
	std::string type((const char *)content + 8, 4);

	// Chunk sizes must be a multiple of two
	if (lenChunk % 2) lenChunk++;

	mc->len = lenChunk + 8;
	if (type.compare("AVI ") == 0) {
		mc->cat = check::Video;
		mc->ext = "avi";
		mc->desc = "Microsoft AVI";
	} else if (type.compare("DSMF") == 0) {
		// Ignore files >16MB as they are probably false positives
		if (lenChunk > 16777216) return false;

		mc->cat = check::Music;
		mc->ext = "dsm";
		mc->desc = "DSIK DSMF module";
	} else if (type.compare("RMID") == 0) {
		// Ignore files >16MB as they are probably false positives
		if (lenChunk > 16777216) return false;

		mc->cat = check::Music;
		mc->ext = "rmi";
		mc->desc = "RIFF MIDI";
	} else if (type.compare("WAVE") == 0) {
		mc->cat = check::Audio;
		mc->ext = "wav";
		mc->desc = "Microsoft Wave";
	} else {
		// Ignore files >16MB as they are probably false positives
		if (lenChunk > 16777216) return false;

		// Exclude anything with control or extended characters in the type
		// field.  The spec says this isn't allowed but then goes on to explain
		// how to include newlines and other control characters in this field(!)
		for (unsigned int i = 0; i < 4; i++) {
			if ((type[i] < 20) || (type[i] > 126)) return false;
		}
		mc->cat = check::Other;
		mc->ext = "riff";
		mc->desc = "Unknown RIFF";
	}
	return true;
}
