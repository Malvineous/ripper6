/**
 * @file   check_voc.cpp
 * @brief  Check for Creative Voice (.voc) files.
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

#define VOC_MAX_BLOCKS 512

bool check_voc(const uint8_t *content, unsigned long len, Match *mc)
{
	REQUIRE(content, "Creative Voice File\x1A");
	unsigned int lenHeader = as_u16le(content + 20);
	unsigned int version = as_u16le(content + 22);
	unsigned int checksum = as_u16le(content + 24);
	if (((0x1233 - version) & 0xFFFF) != checksum) return false;

	unsigned long size = lenHeader;
	bool finished = false;
	for (unsigned int i = 0; i < VOC_MAX_BLOCKS; i++) {
		if (size >= len) return false;
		unsigned int hdr = as_u32le(content + size);
		unsigned int type = hdr & 0xFF;
		unsigned int len = hdr >> 8;
		size += 4;
		if (type == 0) {
			finished = true;
			break;
		}
		if (type > 9) return false; // unknown block type
		size += len;
	}
	if (!finished) return false;

	mc->len = size;
	mc->cat = check::Audio;
	mc->ext = "voc";
	mc->desc = "Creative Voice File";
	return true;
}
