/**
 * @file   check_midi.cpp
 * @brief  Check for standard MIDI files.
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

#define MID_MAX_TRACKS 256

bool check_midi(const uint8_t *content, unsigned long len, Match *mc)
{
	REQUIRE(content, "MThd");
	uint32_t lenMThd = as_u32be(content + 4);
	if (lenMThd > len) return false;

	unsigned int numTracks = as_u16be(content + 10);
	if (numTracks > MID_MAX_TRACKS) return false;

	const uint8_t *end = content + len;

	unsigned long lenTotal = 8 + lenMThd;
	const uint8_t *track = content + lenTotal;
	for (unsigned int i = 0; i < numTracks; i++) {
		REQUIRE(track, "MTrk");
		uint32_t lenMTrk = 8 + as_u32be(track + 4);
		track += lenMTrk;
		lenTotal += lenMTrk;
		if (track > end) return false;
	}

	mc->len = lenTotal;
	mc->cat = check::Music;
	mc->ext = "mid";
	mc->desc = "Standard MIDI";
	return true;
}
