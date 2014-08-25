/**
 * @file   check_tbsa.cpp
 * @brief  Check for The Bone Shaker Architect module format.
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

bool check_tbsa(const uint8_t *content, unsigned long len, Match *mc)
{
	REQUIRE(content, "TBSA0.01");

	unsigned long maxPointer = 0;

	unsigned long offOrderPtrListPtr = as_u16le(content + 8);
	if (offOrderPtrListPtr > 0x100000) return false;
	for (int i = 0; i < 256; i++) {
		unsigned int pos = offOrderPtrListPtr + i * 2;
		if (pos >= len) return false;
		unsigned long offOrderPtrList = as_u16le(content + pos);
		if (offOrderPtrList == 0xFFFF) break;
		if (offOrderPtrList >= len) return false;
		unsigned long countOrderPtrList = *(content + offOrderPtrList);
		for (unsigned long order = 0; order < countOrderPtrList; order++) {
			unsigned long pos = offOrderPtrList + 2 + order * 2;
			if (pos >= len) return false;
			unsigned long orderPtr = as_u16le(content + pos);
			if (maxPointer < orderPtr) maxPointer = orderPtr;
		}
	}

	unsigned long offInstPtrList = as_u16le(content + 16);
	if (offInstPtrList > 0x100000) return false;
	for (int i = 0; i < 256; i++) {
		unsigned int pos = offInstPtrList + i * 2;
		if (pos >= len) return false;
		unsigned long offInst = as_u16le(content + pos);
		if (offInst == 0xFFFF) break;
		if (offInst >= len) return false;
		unsigned long offInstEnd = offInst + 20;
		if (maxPointer < offInstEnd) maxPointer = offInstEnd;
	}

	unsigned long offPatsegPtrList = as_u16le(content + 18);
	if (offPatsegPtrList > 0x100000) return false;
	for (int i = 0; i < 256; i++) {
		unsigned int pos = offPatsegPtrList + i * 2;
		if (pos >= len) return false;
		unsigned long offPatseg = as_u16le(content + pos);
		if (offPatseg == 0xFFFF) break;
		if (offPatseg >= len) return false;
		bool patsegEnd = false;
		for (unsigned long p = 0; p < 1024; p++) {
			unsigned int pos = offPatseg + p;
			if (pos >= len) return false;
			uint8_t cmd = *(content + pos);
			if (cmd == 0xFF) {
				if (maxPointer < pos + 1) maxPointer = pos + 1;
				patsegEnd = true;
				break;
			}
		}
		if (!patsegEnd) return false;
	}

	mc->len = maxPointer;
	mc->cat = check::Music;
	mc->ext = "bsa";
	mc->desc = "The Bone Shaker Architect";
	return true;
}
