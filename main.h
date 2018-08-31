/* Pipepanic - a game.
Copyright (C) 2006 TheGreenKnight <thegreenknight1500@hotmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA. */

/* Global constants */
/*#define DEBUG */
#define FALSE 0
#define TRUE 1
#define ARRAYSIZE(x) (sizeof(x) / sizeof(x)[0])
#define VERSION "0.1.3"
#define DATADIR "/opt/QtPalmtop/share/pipepanic/"
#define DIGITS24BMP "digits24.bmp"
#define DIGITS48BMP "digits48.bmp"
#define TILES24BMP "tiles24.bmp"
#define TILES48BMP "tiles48.bmp"
#define ASCII15BMP "ascii15.bmp"
#define ASCII30BMP "ascii30.bmp"
#define RESOURCEFILE ".pipepanicrc"

#define MAGENTA 0xff, 0x00, 0xff
#define YELLOW  0xff, 0xcc, 0x66
#define WHITE   0xff, 0xff, 0xff
#define BLACK   0x00, 0x00, 0x00

enum game_mode {
	GAMEOVER,
	GAMEFLASHHIGHSCORE,
	GAMEFILLPIPES,
	GAMECLEARDEADPIPES,
	GAMEON,
	GAMESTART,
	GAMEFINISH,
	GAMEQUIT,
};

#define REDRAWALL (0xFFFF ^ (REDRAWPIPE | REDRAWTILE | REDRAWHELP))
#define REDRAWHELP 256
#define REDRAWBOARD 128
#define REDRAWALLPIPES 64
#define REDRAWHIGHSCORE 32
#define REDRAWTIMER 16
#define REDRAWSCORE 8
#define REDRAWPREVIEW 4
#define REDRAWTILE 2
#define REDRAWPIPE 1
#define REDRAWNONE 0

#define BOARDW 10
#define BOARDH 10
#define GAMETIME 60 * 4

#define PIPEARRAYSIZE 105
#define PREVIEWARRAYSIZE 3
#define POINTSARRAYSIZE 75

#define LEAKYPIPEVAL +50
#define NULLPIPEVAL 255
#define DEADPIPEVAL 253
#define FILLEDCOUNTERBASE 20

#define PIPEOVERWRITESCORE -10
#define PIPEPLACEMENTSCORE 10
#define FILLNOWSCORE 5
#define DEADPIPESCORE -10
#define FILLEDPIPESCORE 50

#define CLEARDEADPIPESTIMEOUT 150
#define FILLPIPESTIMEOUT 200
#define FLASHHIGHSCORETIMEOUT 500

#define HELPPAGE0 "Instructions\n----------------\nConnect the green start pipe to the red end\npipe using the pipes from the green tile.\n\nThe more pipes you utilise the greater your\nscore.\n\nThe pipe network will be filled once the time\nexpires or you manually fill the network by\nclicking Fill or the green start pipe.\n\nThe game will end prematurely if a leaky\npipe is found when filling.\n\nClick High Score to view the high score's\npipe network."

#define HELPPAGE1 "Scoring\n----------\n10 points for each pipe placed.\n\n-10 points for each pipe overwritten.\n\n5 points for each second remaining if you fill\nthe network early.\n\n-10 points for each unconnected pipe found\nwhen filling.\n\n50 points for each filled pipe."

#define HELPPAGE2 "Pipe Frequency\n--------------------\nThe pipes appear on the preview bar\nshuffled from a set frequency. Every 105\npipes will include the following :-\n\n28 corner pieces\n20 T pieces\n18 vertical pipes\n18 horizontal pipes\n16 terminators\n5 crosses"

#define HELPPAGE3 "Strategy\n------------\nFamiliarise yourself with the pipe frequency.\n\nAttempt to utilise every pipe offered to you\noverwriting as few as possible.\n\nBuild speedily and occupy as much of the\nboard as you can.\n\nReserve part of the board for placing the\ncorner pieces, creating a winding snake from\none side to the other.\n\nFilling early rewards a bonus."

#define HELPPAGE4 "Keys\n-------\nEsc (Cancel on the Zaurus) exits Help and\nthe game.\n\nLeft & right keys navigate Help.\n\nPrimarily the game is mouse/stylus driven."

#define HELPPAGE5 "Licence\n-----------\nThe source code is licenced under the GNU\nGPL v2 - http://www.gnu.org/copyleft/gpl.html\n\nThe artwork is licenced under the Free Art\nLicence - http://artlibre.org/licence/lal/en/\n\nPipepanic\nCopyright 2006 TheGreenKnight\nthegreenknight1500@hotmail.com\n\nhttp://thunor.org.uk/pipepanic\n\nVersion " VERSION



