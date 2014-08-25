/**
 * @file   check_s3m.cpp
 * @brief  Check for ScreamTracker 3 modules.
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

bool check_s3m(const uint8_t *content, unsigned long len, Match *mc)
{
	// Too short
	if (len < 0x60) return false;

	REQUIRE(content + 0x2c, "SCRM");
	if (content[28] != 0x1A) return false;

	unsigned int orderCount = as_u16le(content + 32);
	unsigned int instCount = as_u16le(content + 34);
	unsigned int patternCount = as_u16le(content + 36);

	unsigned long size = 0x60 + orderCount;

	const uint8_t *ptr = content + size;
	for (unsigned int i = 0; i < instCount; i++) {
		unsigned long offInst = as_u16le(ptr + i * 2) << 4;
		unsigned long endInst = offInst + 0x50;
		if (endInst > len) return false;
		if (size < endInst) size = endInst;
		const uint8_t *inst = content + offInst;
		switch (*inst) {
			case 0: // empty
				break;
			case 1: { // pcm
				inst += 13;
				unsigned long offSample = ((*inst << 16) | as_u16le(inst + 1)) << 4;
				unsigned long lenSample = as_u32le(inst + 3);
				unsigned long endSample = offSample + lenSample;
				if (endSample > len) return false;
				if (size < endSample) size = endSample;
				break;
			}
			case 2: // adlib
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				break;
		}
	}

	ptr = content + 0x60 + orderCount + instCount * 2;
	for (unsigned int i = 0; i < patternCount; i++) {
		unsigned long offPattern = as_u16le(ptr + i * 2) << 4;
		if (offPattern >= len) return false;
		unsigned long lenPattern = as_u16le(content + offPattern);
		unsigned long endPattern = offPattern + lenPattern + 2;
		if (size < endPattern) size = endPattern;
	}

	mc->len = size;
	mc->cat = check::Music;
	mc->ext = "s3m";
	mc->desc = "ScreamTracker 3";
	return true;
}
