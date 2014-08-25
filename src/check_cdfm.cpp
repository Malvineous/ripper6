/**
 * @file   check_cdfm.cpp
 * @brief  Check for Renaissance CDFM module format.
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

/// Maximum realistic length of a sample
#define CDFM_MAX_SAMPLE_LEN 0x100000

bool check_cdfm(const uint8_t *content, unsigned long len, Match *mc)
{
	// Too short
	if (len < 6 + 4 + 1 + 4 + 11) return false;

	const uint8_t *end = content + len;

	// Need a realistic speed value
	REQUIRE_RANGE(content[0], 1, 32);

	const uint8_t& numOrders = content[1];
	// Need at least one order, but not too many
	REQUIRE_RANGE(numOrders, 1, 128);

	const uint8_t& numPatterns = content[2];
	// Need at least one pattern, but not too many
	REQUIRE_RANGE(numPatterns, 1, 128);

	const uint8_t& numDigInst = content[3];
	const uint8_t& numOPLInst = content[4];
	// Need at least one instrument, but not too many
	REQUIRE_RANGE(numDigInst + numOPLInst, 1, 48);

	const uint8_t& loopDest = content[5];
	// Loop destination must point to a valid index in the order list
	if (loopDest >= numOrders) return false;

	uint32_t sampleOffset = as_u32le(content + 6);
	if (sampleOffset > len) return false;

	unsigned long totalSize = sampleOffset;

	const uint8_t *orders = content + 10;
	if (orders + numOrders >= end) return false;
	for (unsigned int i = 0; i < numOrders; i++) {
		if (orders[i] >= numPatterns) return false;
	}

	const uint8_t *offPatternPtrs = orders + numOrders;

	const uint8_t *instDig = offPatternPtrs + numPatterns * 4;
	if (instDig >= end) return false;
	for (unsigned int i = 0; i < numDigInst; i++) {
		const uint8_t *inst = instDig + 16 * i;
		REQUIRE(inst, "\x00\x00\x00\x00"); // address
		uint32_t lenSample = as_u32le(inst + 4);
		if (lenSample > CDFM_MAX_SAMPLE_LEN) return false;
		uint32_t loopStart = as_u32le(inst + 8);
		if (loopStart > lenSample) return false;
		totalSize += lenSample;
		uint32_t loopEnd = as_u32le(inst + 12);
		if ((loopEnd != 0x00FFFFFF) && (loopEnd > lenSample)) return false;
		if (loopEnd == 0) return false; // no loop should be 0x00FFFFFF
		if (loopEnd <= loopStart) return false; // wrong way around
	}

	const uint8_t *instOPL = instDig + 16 * numDigInst;
	for (unsigned int i = 0; i < numOPLInst; i++) {
		const uint8_t *inst = instOPL + 11 * i;
		if (
			(inst[0] > 0x0F)
		) return false; // OPL reg 0xC0
		// todo: other registers
	}
	if (totalSize == 0) return false;

	// Read the pattern offsets and make sure they are before the sample data.
	// Technically this is allowed, but as no official files are written with
	// pattern data after sample data we will use this as part of the check.
	const uint8_t *offPatterns = instOPL + 11 * numOPLInst;
	unsigned long offLastPattern = 0;
	for (unsigned int i = 0; i < numPatterns; i++) {
		unsigned long offThisPattern = as_u32le(offPatternPtrs + 4 * i);
		const uint8_t *pattern = offPatterns + offThisPattern;
		if (pattern >= end) return false;
		if (pattern >= content + totalSize) return false;
		if ((i > 0) && (offThisPattern <= offLastPattern)) return false; // patterns are always in order
		offLastPattern = offThisPattern;
		// Read the pattern data
		bool endPattern = false;
		for (unsigned int p = 0; (p < 2048) && ((pattern + p) < end); p++) {
			if (pattern[p] <= 0x0C) { // note on
				p += 2;
			} else if ((pattern[p] >= 0x20) && (pattern[p] <= 0x2C)) { // set volume
				p++;
			} else if (pattern[p] == 0x40) { // delay
				p++;
			} else if (pattern[p] == 0x60) { // end of pattern
				endPattern = true;
				break;
			} else {
				// Invalid command byte
				return false;
			}
		}
		if (!endPattern) {
			// Pattern didn't finish with an end-of-pattern marker
			return false;
		}
	}

	mc->len = totalSize;
	mc->cat = check::Music;
	mc->ext = "670";
	mc->desc = "Renaissance CDFM";
	return true;
}
