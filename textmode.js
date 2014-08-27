/**
 * @file   textmode.js
 * @brief  HTML5 Canvas that looks like VGA/EGA/CGA text mode.
 * http://github.com/Malvineous/textmode-js
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

function TextMode(idCanvas, cellWidth, cellHeight, blink, urlFont, cb)
{
	this.canvas = document.getElementById(idCanvas);
	if (!this.canvas.getContext) return;
	this.cxt = this.canvas.getContext('2d');
	if (!this.cxt) return;

	this.cellWidth = cellWidth;
	this.cellHeight = cellHeight;

	this.width = this.canvas.width;
	this.height = this.canvas.height;
	this.cols = this.width / this.cellWidth | 0;
	this.rows = this.height / this.cellHeight | 0;

	this.cellCanvas = document.createElement('canvas');
	this.cellCanvas.width = this.cellWidth;
	this.cellCanvas.height = this.cellHeight;
	this.cell = this.cellCanvas.getContext('2d');
	if (!this.cell) {
		this.cxt = null;
		return;
	}

	this.overlayCanvas = document.createElement('canvas');
	this.overlayCanvas.width = this.width;
	this.overlayCanvas.height = this.height;
	this.overlayCanvas.style.position = 'absolute';
	this.overlayCanvas.style.left = 0;
	this.overlayCanvas.style.top = 0;
	this.overlay = this.overlayCanvas.getContext('2d');
	this.canvas.parentElement.appendChild(this.overlayCanvas);

	this.palCGA = new Array;
	for (var i = 0; i < 16; i++) {
		this.palCGA[i] = "rgb("
			+ ((i & 4) ? ((i & 8) ? 0xFF : 0xAA) : ((i & 8) ? 0x55 : 0x00))
			+ ", "
			+ ((i == 6) ? 0x55 : ((i & 2) ? ((i & 8) ? 0xFF : 0xAA) : ((i & 8) ? 0x55 : 0x00)))
			+ ", "
			+ ((i & 1) ? ((i & 8) ? 0xFF : 0xAA) : ((i & 8) ? 0x55 : 0x00))
			+ ")";
	}

	this.imgFont = new Image;
	var self = this;
	this.imgFont.onload = function() {
		cb(self);
	};
	this.imgFont.src = urlFont;

	this.blinking = blink;
	if (this.blinking) this.doBlink();
}

TextMode.prototype.setSize = function(w, h)
{
	this.canvas.width = w;
	this.canvas.height = h;
	this.width = this.canvas.width;
	this.height = this.canvas.height;
	this.overlayCanvas.width = this.width;
	this.overlayCanvas.height = this.height;
	this.cols = this.width / this.cellWidth | 0;
	this.rows = this.height / this.cellHeight | 0;
}

TextMode.prototype.doBlink = function()
{
	if (this.overlayCanvas.style.visibility == 'hidden') {
		this.overlayCanvas.style.visibility = 'visible';
	} else {
		this.overlayCanvas.style.visibility = 'hidden';
	}

	// Do it like this rather than using setInterval so that Firefox's stuttering
	// and general unresponsiveness gets smoothed out to a slower overall blink
	// rate rather than really uneven blinking.
	var self = this;
	if (this.blinking) setTimeout(function() { self.doBlink(); }, 500);
}

TextMode.prototype.putChar = function(x, y, f, b, c)
{
	var cellX = x * this.cellWidth;
	var cellY = y * this.cellHeight;

	this.cxt.fillStyle = this.palCGA[this.blinking ? b & 7 : b];
	this.cxt.fillRect(cellX, cellY, this.cellWidth, this.cellHeight);

	this.cell.fillStyle = this.palCGA[f];
	this.cell.fillRect(0, 0, this.cellWidth, this.cellHeight);
	this.cell.save();
	this.cell.globalCompositeOperation = 'destination-in';
	this.cell.drawImage(this.imgFont,
		(c % 32) * this.cellWidth, (c >> 5) * this.cellHeight,
		this.cellWidth, this.cellHeight,
		0, 0, this.cellWidth, this.cellHeight);
	this.cell.restore();

	if (this.blinking && (b & 8)) {
		this.overlay.drawImage(this.cellCanvas, cellX, cellY);
	} else {
		this.overlay.clearRect(cellX, cellY, this.cellWidth, this.cellHeight);
		this.cxt.drawImage(this.cellCanvas, cellX, cellY);
	}
}

TextMode.prototype.drawBox = function(x, y, w, h, f, b)
{
	var cellX = x * this.cellWidth;
	var cellY = y * this.cellHeight;
	var cellW = w * this.cellWidth;
	var cellH = h * this.cellHeight;
	this.cxt.fillStyle = this.palCGA[this.blinking ? b & 7 : b];
	this.cxt.fillRect(cellX, cellY, cellW, cellH);
	this.overlay.clearRect(cellX, cellY, cellW, cellH);

	var right = x + w - 1;
	var bottom = y + h - 1;
	for (var x1 = x + 1; x1 < right; x1++) {
		this.putChar(x1, y, f, b, 205);
		this.putChar(x1, bottom, f, b, 205);
		this.putChar(x1 + 1, bottom + 1, 7, 0, 32); // shadow
	}
	for (var y1 = y + 1; y1 < bottom; y1++) {
		this.putChar(x, y1, f, b, 186);
		this.putChar(right, y1, f, b, 186);
		this.putChar(right + 1, y1, 7, 0, 32); // shadow
		this.putChar(right + 2, y1, 7, 0, 32);
	}
	this.putChar(x, y, f, b, 201);
	this.putChar(right, y, f, b, 187);
	this.putChar(x, bottom, f, b, 200);
	this.putChar(right, bottom, f, b, 188);
	this.putChar(right + 1, bottom, 7, 0, 32);
	this.putChar(right + 1, bottom + 1, 7, 0, 32);
	this.putChar(right + 2, bottom, 7, 0, 32);
	this.putChar(right + 2, bottom + 1, 7, 0, 32);
}

TextMode.prototype.write = function(x, y, f, b, s)
{
	for (var i = 0; i < s.length; i++) {
		this.putChar(x + i, y, f, b, s.charCodeAt(i));
	}
}

TextMode.prototype.centreText = function(x, y, f, b, s)
{
	this.write(x - (s.length >> 1), y, f, b, s);
}
