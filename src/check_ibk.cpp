/**
 * @file   check_ibk.cpp
 * @brief  Check for OPL2 instrument bank (.ibk) files.
 *
 * Copyright (C) 2014-2015 Adam Nielsen <malvineous@shikadi.net>
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

#define IBK_COUNT 128
#define IBK_LEN (4 + 16*IBK_COUNT + 9*IBK_COUNT)

bool check_ibk(const uint8_t *content, unsigned long len, Match *mc)
{
	REQUIRE(content, "IBK\x1A");
	if (IBK_LEN > len) return false;

	// Make sure the unused bytes in each instrument are zero
	content += 4;
	for (unsigned int i = 0; i < IBK_COUNT; i++) {
		content += 11; // skip over OPL data
		REQUIRE(content, "\0\0\0\0\0");
		content += 5;
	}

	mc->len = IBK_LEN;
	mc->cat = check::Music;
	mc->ext = "ibk";
	mc->desc = "OPL2 Instrument Bank";
	return true;
}
