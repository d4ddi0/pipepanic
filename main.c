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

/* Includes */
#include <SDL/SDL_main.h>
#include <SDL/SDL.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "main.h"

struct drawpipe {
	int row;
	int col;
	int filled;
};

/* Variable declarations */
int xres = 640;
int yres = 480;
int tilew = 48;
int tileh = 48;
int digitw = 30;
int digith = 48;
int asciiw = 30;
int asciih = 30;
int sdl_fullscreen = FALSE;
SDL_Surface *screen;
SDL_Surface *digits;
SDL_Surface *tiles;
SDL_Surface *ascii;
SDL_Event event;
char *current_dir;
char *user_home_dir;
int quit = 0;
int game_mode = GAMEON;
int previous_game_mode = 0;
int redraw = REDRAWALL;
int highscoretable[5] = {0, 0, 0, 0, 0};
int highscoreboard[5][BOARDH * BOARDW];
int score = 0;
int disablescoring = FALSE;
int gametime = GAMETIME;
Uint32 fps_ticks = 0;
long int frames = 0;
struct drawpipe drawpipearray[BOARDH * BOARDW + 1];
int previewarray[PREVIEWARRAYSIZE];
int pipearray[PIPEARRAYSIZE];
int boardarray[BOARDH][BOARDW];
SDL_Rect tile_rects[BOARDH][BOARDW];
int deadpipesarray[BOARDH][BOARDW];
SDL_Rect hiscore_label, score_label, time_label, fill_label, help_label,
	new_game_label, gameboard_rect, help_l_label, help_r_label,
	help_exit_label;
int cleardeadpipesy = 0, cleardeadpipesx = 0;
int fillpipespasscounter = FILLEDCOUNTERBASE;
int flashhighscorestate = FALSE;
int helppage = 0;
/*						  ! "  #  $  %  &  ' ( ) *  + , - . / 0  1  2  3  4  5  6  7  8  9  : ; <  =  >  ?  @  A  B  C  D  E  F  G  H  I J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z  [ \ ] ^  _  ' a  b  c  d  e  f g  h  i j k  l m  n  o  p  q  r s  t u  v  w  x  y  z  { | } ~ */
int asciiwidths[100] = {7,5,10,11,11,18,15,6,6,6,9,11,5,8,5,7,13,10,12,13,14,13,13,13,13,13,5,5,11,11,11,12,15,15,14,13,14,13,12,15,14,6,12,15,12,16,14,16,13,16,14,14,14,14,15,19,13,13,14,8,6,8,11,10,6,13,12,13,12,13,8,13,11,6,6,12,6,17,11,13,12,12,8,12,9,11,12,18,13,12,11,8,4,8,11};

/* Function prototypes */
int get_machine_id(void);
void clear_screen(void);
int load_bitmaps(void);
void draw_game(void);
void draw_digits(int value, int digitcount, int xpos, int ypos);
void initialise_new_game(void);
void manage_user_input(void);
int getnextpipepiece(void);
void fillpipearray(void);
int fillpipearraypieces(int pipepiece, int frequency, int nextpointer);
void get_pipe_src_xy(int pipeid, int *x, int *y, int drawpipefilled);
void createdeadpipesarray(void);
void cleardeadpipes(void);
void fillpipes(void);
void read_rc_file(void);
void save_rc_file(void);
void draw_ascii(char *text, int xpos, int ypos);
void manage_help_input(int input);
void manage_mouse_input(void);
void setup_gameboard(void);


/***************************************************************************
 * Main                                                                    *
 ***************************************************************************/

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char *argv[]) {
	int count, count2;
	
	/* Initialise the highscoreboard array because there may not yet
	   be a saved highscore[board]/rc file */
	for (count = 0; count < 5; count++) {
		for (count2 = 0; count2 < BOARDH * BOARDW; count2++) {
			highscoreboard[count][count2] = NULLPIPEVAL;
		}
	}
	
	srand((unsigned) time(NULL));	/* Seed C's random number generator */
	
	current_dir = getenv("PWD");
	user_home_dir = getenv("HOME");
	
	#ifdef DEBUG
	printf("PWD=%s\n", current_dir);
	printf("HOME=%s\n", user_home_dir);
	#endif
	
	if (get_machine_id()) return 1;	/* This sets up the screen and tile sizes. */
	
	#ifdef DEBUG
	printf("Reading resource file -> ");
	#endif
	read_rc_file();	/* This gets the saved highscore[s] */
	
	/* Process any command line arguments. These will override any found in the resource file. */
	if (argc > 1) {
		int count;
		for (count = argc - 1; count > 0; count--) {
			if (!strcmp(argv[count], "--help")) {
					/*  1234567890123456789012345678901234567890 <- Formatting for small terminal. */
				printf("\nPipepanic version %s\n", VERSION);
				printf("Usage: pipepanic [option] [option]...\n");
				printf("Options:\n");
				printf("  -240x320   Suitable for the SL5X00\n");
				printf("  -320x240   Suitable for the SL5X00\n");
				printf("  -640x480   Suitable for the C Series\n");
				printf("  -480x640   Suitable for the SL6000\n");
				printf("  -f         Run the game fullscreen\n");
				return 0;
			} else if (!strcmp(argv[count], "-240x320")) {
				xres = 240; yres = 320;
				tilew = tileh = digith = 24; digitw = 15; asciiw = asciih = 15;
			} else if (!strcmp(argv[count], "-320x240")) {
				xres = 320; yres = 240;
				tilew = tileh = digith = 24; digitw = 15; asciiw = asciih = 15;
			} else if (!strcmp(argv[count], "-640x480")) {
				xres = 640; yres = 480;
				tilew = tileh = digith = 48; digitw = 30; asciiw = asciih = 30;
			} else if (!strcmp(argv[count], "-480x640")) {
				xres = 480; yres = 640;
				tilew = tileh = digith = 48; digitw = 30; asciiw = asciih = 30;
			} else if (!strcmp(argv[count], "-f")) {
				sdl_fullscreen = SDL_FULLSCREEN;
			} else {
				printf("\n'%s' not recognised. Try '--help'.\n\n", argv[count]);
				return 0;
			}
		} 
	}

	/* Initialise SDL */
	if(SDL_Init(SDL_INIT_VIDEO)) {
		printf("%s: Cannot initialise SDL: %s\n", __func__, SDL_GetError());
		return 1;
	}

	atexit(SDL_Quit);

	#ifdef DEBUG
	printf("Setting video mode %ix%i\n", xres, yres);
	#endif
	
	/* Set SDL video mode */
	screen = SDL_SetVideoMode(xres, yres, 16, SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_ANYFORMAT | sdl_fullscreen);
	if(screen == NULL) {
		printf("%s: Cannot initialise screen: %s\n", __func__, SDL_GetError());
		exit(1);
	}
	
	#ifdef DEBUG	
	printf ("Loading bitmaps\n");
	#endif
	
	if (load_bitmaps()) exit(1);
	
	/* Initialise new game */
	setup_gameboard();
	initialise_new_game();
	Uint32 timeout = 0;
	/* Main game loop */
	while(quit == 0) {
		Uint32 ticks;

		do {
			ticks = SDL_GetTicks();
			draw_game();
			manage_user_input();
			SDL_Delay(9);
		} while (ticks < timeout);

		switch (game_mode) {
		case GAMEON:
			timeout = ticks + 1000;
			redraw = redraw | REDRAWTIMER;
			gametime = gametime - 1;
			if (gametime <= 0) {
				gametime = 0;
				createdeadpipesarray();
				timeout = 0;
			}
			break;
		case GAMECLEARDEADPIPES:
			timeout = ticks + CLEARDEADPIPESTIMEOUT;
			cleardeadpipes();
			if (game_mode == GAMEFILLPIPES) {
				timeout = 0;
			}
			break;
		case GAMEFILLPIPES:
			timeout = ticks + FILLPIPESTIMEOUT;
			fillpipes();
			break;
		case GAMEFLASHHIGHSCORE:
			timeout = ticks + FLASHHIGHSCORETIMEOUT;
			redraw = redraw | REDRAWHIGHSCORE;
			flashhighscorestate = !flashhighscorestate;
			break;
		}
	}
	
	/* Shutdown all subsystems */
	SDL_Quit();
	return 0;
}

/***************************************************************************
 * Get Machine ID                                                          *
 ***************************************************************************/
/* This looks for the file /proc/deviceinfo/product which exists on the
   Zaurus and not the PC and then sets the screen resolution accordingly.
   On exit: returns 1 if file found but product unknown else 0. */
   
int get_machine_id(void) {  
	char buffer[256];
	int returnval = 0;
	FILE *file = fopen( "/proc/deviceinfo/product", "r" );
	
	if (file && fgets(buffer, 255, file)) {
		#ifdef DEBUG
		printf("product=%s\n", buffer);
		#endif
		if (!strncmp(buffer, "SL-5", 4)) {
			xres = 240; yres = 320;
			tilew = tileh = digith = 24; digitw = 15; asciiw = asciih = 15;
		} else if (!strncmp(buffer, "SL-C", 4)) { 
			xres = 640; yres = 480;
			tilew = tileh = digith = 48; digitw = 30; asciiw = asciih = 30;
		} else if (!strncmp(buffer, "SL-6", 4)) { 
			xres = 480; yres = 640;
			tilew = tileh = digith = 48; digitw = 30; asciiw = asciih = 30;
		} else {
			printf("product=%s\n", buffer);
			printf("Unknown Zaurus model! Please email me the product shown above.\n");
			returnval = 1;
		}
		fclose (file);
	}
	return returnval;
}

/***************************************************************************
 * Clear Screen                                                            *
 ***************************************************************************/
/* This simply sets the screen to black. */

void clear_screen(void) {
	SDL_Rect dest;

	dest.x = dest.y = 0; dest.w = screen->w; dest.h = screen->h;
	SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, BLACK));
}

/***************************************************************************
 * Load Bitmaps                                                            *
 ***************************************************************************/
/* This locates and loads the relevant bitmap images. They are converted to
   the display format for optimisation. Also RLE is enabled for the colorkey
   which makes probably the biggest difference in frame rate I have found yet.
   On exit: returns 1 if an error occured else 0. */
int load_bitmap(SDL_Surface **surface, const char *fname, Uint32
		cflag, Uint8 r, Uint8 g, Uint8 b)
{
	SDL_Surface *temp = SDL_LoadBMP(fname);
	char buf[128];
	if (!temp) {
		strncpy(buf, DATADIR, sizeof(buf));
		strncat(buf, fname, sizeof(buf) - strnlen(DATADIR, 128));
		temp = SDL_LoadBMP(buf);
		if (!temp) {
			printf("%s: Cannot find file %s\n", __func__, buf);
			return 1;
		}
	}

	if (cflag)
		SDL_SetColorKey(temp, cflag, SDL_MapRGB(temp->format, r, g, b));

	*surface = SDL_DisplayFormat(temp);
	if (!surface) {
		printf("%s: SDL_DisplayFormat error: %s\n",
		       __func__, SDL_GetError());
		return 1;
	}
	SDL_FreeSurface(temp);

	return 0;
}

int load_bitmaps(void) {
	const char *digitsfile, *tilesfile, *asciifile;

	if (xres <= 320) {
		digitsfile = DIGITS24BMP;
		tilesfile = TILES24BMP;
		asciifile = ASCII15BMP;
	} else {
		digitsfile = DIGITS48BMP;
		tilesfile = TILES48BMP;
		asciifile = ASCII30BMP;
	}

	if (load_bitmap(&digits, digitsfile, 0, 0, 0, 0) != 0)
		return 1;

	if (load_bitmap(&tiles, tilesfile,
			SDL_SRCCOLORKEY | SDL_RLEACCEL, MAGENTA) != 0)
		return 1;
	if (load_bitmap(&ascii, asciifile,
			SDL_SRCCOLORKEY | SDL_RLEACCEL, WHITE) != 0)
		return 1;

	return 0;
}

void setup_gameboard(void)
{
	int row, column, x, y;

	y = (xres == 240 || xres == 480)? 2 * tileh : 0;
	for (row = 0; row < BOARDH; row++) {
		x = xres - BOARDW * tilew;
		for (column = 0; column < BOARDW; column++) {
			tile_rects[row][column].w = tilew;
			tile_rects[row][column].h = tileh;
			tile_rects[row][column].x = x;
			tile_rects[row][column].y = y;
			x += tilew;
		}
		y += tileh;
	}

	if (xres == 240 || xres == 480) {
		hiscore_label.y = 0;
		hiscore_label.x = 3.7 * tilew;
		hiscore_label.w = 3 * tilew;
		hiscore_label.h = tileh;

		score_label.x = 0;
		score_label.y = 0 ;
		score_label.w = 2 * tilew;
		score_label.h = tileh;

		time_label.x = 7.6 * tilew;
		time_label.y = 0 ;
		time_label.w = 3 * tilew;
		time_label.h = tileh;

		fill_label.x = 4 * tilew;
		fill_label.y = 12.18 * tileh;
		fill_label.w = tilew;
		fill_label.h = tileh;

		help_label.x = 5.25 * tilew;
		help_label.y = 12.18 * tileh;
		help_label.w = 2 * tilew;
		help_label.h = tileh;

		new_game_label.x = 6.9 * tilew;
		new_game_label.y = 12.18 * tileh;
		new_game_label.w = 3 * tilew;
		new_game_label.h = tileh;

		gameboard_rect.x = xres -  BOARDW * tilew;
		gameboard_rect.y = 2 * tileh;
		gameboard_rect.w = BOARDW * tilew;
		gameboard_rect.h = BOARDH * tileh;

		help_l_label.x = 0 ;
		help_l_label.y = (BOARDH - 1) * tileh + 2 * tileh;
		help_l_label.w = tilew;
		help_l_label.h = tileh;

		help_r_label.x = xres - tilew;
		help_r_label.y = (BOARDH - 1) * tileh + 2 * tileh;
		help_r_label.w = tilew;
		help_r_label.h = tileh;

		help_exit_label.x = xres - 5.6 * tilew;
		help_exit_label.y = (BOARDH - 1) * tileh + 2 * tileh;
		help_exit_label.w = 2 * tilew;
		help_exit_label.h = tileh;
	} else {
		hiscore_label.y = 0;
		hiscore_label.x =  0;
		hiscore_label.w = 3 * tilew;
		hiscore_label.h = tileh;

		score_label.x = 0;
		score_label.y =  2 * tileh;
		score_label.w = 2 * tilew;
		score_label.h = tileh;

		time_label.x =  0;
		time_label.y =  4 * tileh;
		time_label.w = 3 * tilew;
		time_label.h = tileh;

		fill_label.x =  0;
		fill_label.y =  7 * tileh;
		fill_label.w = tilew;
		fill_label.h = tileh;

		help_label.x =  0;
		help_label.y =  8 * tileh;
		help_label.w = 2 * tilew;
		help_label.h = tileh;

		new_game_label.x =  0;
		new_game_label.y =  9 * tileh;
		new_game_label.w = 3 * tilew;
		new_game_label.h = tileh;

		gameboard_rect.x = xres -  BOARDW * tilew;
		gameboard_rect.y =  0;
		gameboard_rect.w = BOARDW * tilew;
		gameboard_rect.h = BOARDH * tileh;

		help_l_label.x = xres - BOARDW * tilew;
		help_l_label.y =  (BOARDH - 1) * tileh;
		help_l_label.w = tilew;
		help_l_label.h = tileh;

		help_r_label.x = xres - tilew;
		help_r_label.y =  (BOARDH - 1) * tileh;
		help_r_label.w = tilew;
		help_r_label.h = tileh;

		help_exit_label.x = xres - 5.6 * tilew;
		help_exit_label.y =  (BOARDH - 1) * tileh;
		help_exit_label.w = 2 * tilew;
		help_exit_label.h = tileh;
	}

}

/***************************************************************************
 * Draw Game                                                               *
 ***************************************************************************/
/* This draws the game. Either it draws all of it or just parts of it. */

void draw_game(void) {
	SDL_Rect src, dest;
	int row, column, x, y;

	if ((redraw & REDRAWBOARD) == REDRAWBOARD) {
		/* Paint the whole screen yellow. */
		dest.x = dest.y = 0; dest.w = screen->w; dest.h = screen->h;
		SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, YELLOW));

		/* Draw all the game board background tiles. */
		src.x = 4 * tilew; src.y = 6 * tileh;
		src.w = tilew; src.h = tileh;
		for (row = 0; row < BOARDH; row++) {
			for (column = 0; column < BOARDW; column++) {
				if(SDL_BlitSurface(tiles, &src, screen, &tile_rects[row][column]) < 0)
					printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());
			}
		}

		/* Draw all the text. */
		/* High Score. */
		src.x = 0 * tilew; src.y = 8 * tileh;
		src.w = 3 * tilew; src.h = tileh;
		if(SDL_BlitSurface(tiles, &src, screen, &hiscore_label) < 0)
			printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());

		/* Score. */
		src.x = 3 * tilew; src.y = 8 * tileh;
		src.w = 2 * tilew; src.h = tileh;
		if(SDL_BlitSurface(tiles, &src, screen, &score_label) < 0)
			printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());

		/* Time (s). */
		src.x = 0 * tilew; src.y = 9 * tileh;
		src.w = 3 * tilew; src.h = tileh;
		if(SDL_BlitSurface(tiles, &src, screen, &time_label) < 0)
			printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());

		/* Fill. */
		src.x = 3 * tilew; src.y = 10 * tileh;
		src.w = tilew; src.h = tileh;
		if(SDL_BlitSurface(tiles, &src, screen, &fill_label) < 0)
			printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());

		/* Help. */
		src.x = 3 * tilew; src.y = 9 * tileh;
		src.w = 2 * tilew; src.h = tileh;
		if(SDL_BlitSurface(tiles, &src, screen, &help_label) < 0)
			printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());

		/* New Game. */
		src.x = 0 * tilew; src.y = 10 * tileh;
		src.w = 3 * tilew; src.h = tileh;
		if(SDL_BlitSurface(tiles, &src, screen, &new_game_label) < 0)
			printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());
	}

	if ((redraw & REDRAWALLPIPES) == REDRAWALLPIPES) {
		/* Draw any pipes found in the board array. 
		   NOTE that this doesn't draw the background tile first.
		   This is done in REDRAWBOARD above. */
		src.w = tilew; src.h = tileh;
		dest.y = 0; if (xres == 240 || xres == 480) dest.y = 2 * tileh;
		dest.w = tilew; dest.h = tileh;
		for (row = 0; row < BOARDH; row++) {
			dest.x = xres -  BOARDW * tilew;
			for (column = 0; column < BOARDW; column++) {
				if (boardarray[row][column] != NULLPIPEVAL) {
					get_pipe_src_xy(boardarray[row][column], &x, &y, FALSE);
					src.x = x; src.y = y;
					if(SDL_BlitSurface(tiles, &src, screen, &dest) < 0)
						printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());
				}
				dest.x = dest.x + tilew;
			}
			dest.y = dest.y + tileh;
		}
	}
	
	if ((redraw & REDRAWHIGHSCORE) == REDRAWHIGHSCORE) {
		/* The top high score */
		/* If flashhighscorestate is TRUE then no score is shown
		   (it is blanked out with the background colour). */
		dest.x = 0.15 * tilew; if (xres == 240 || xres == 480) dest.x = 4 * tilew;
		dest.y = 0.9 * tileh;
		if (flashhighscorestate) {
			dest.w = 5 * digitw; dest.h = digith;
			SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, YELLOW));
		} else {
			draw_digits(highscoretable[0], 4, dest.x, dest.y);
		}
	}
	
	if ((redraw & REDRAWTIMER) == REDRAWTIMER) {
		/* The time */
		dest.x = 0.15 * tilew; if (xres == 240 || xres == 480) dest.x = 8 * tilew;
		dest.y = 4.9 * tileh; if (xres == 240 || xres == 480) dest.y = 0.9 * tileh;
		draw_digits(gametime, 3, dest.x, dest.y);
	}
	
	if ((redraw & REDRAWSCORE) == REDRAWSCORE) {
		/* The score */
		dest.x = 0.15 * tilew; if (xres == 240 || xres == 480) dest.x = 0.15 * tilew;
		dest.y = 2.9 * tileh; if (xres == 240 || xres == 480) dest.y = 0.9 * tileh;
		draw_digits(score, 4, dest.x, dest.y);
	}
	
	if ((redraw & REDRAWPREVIEW) == REDRAWPREVIEW) {
		/* Draw preview array */
		/* Left */
		src.x = 1 * tilew; src.y = 7 * tileh;
		src.w = tilew; src.h = tileh;
		dest.x = 0.09 * tilew; if (xres == 240 || xres == 480) dest.x = 0.09 * tilew;
		dest.y = 6 * tileh; if (xres == 240 || xres == 480) dest.y = 12.18 * tileh;
		dest.w = tilew; dest.h = tileh;
		if(SDL_BlitSurface(tiles, &src, screen, &dest) < 0)
			printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());
		get_pipe_src_xy(previewarray[2], &x, &y, FALSE);
		src.x = x; src.y = y;
		if(SDL_BlitSurface(tiles, &src, screen, &dest) < 0)
			printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());

		/* Middle */
		src.x = 1 * tilew; src.y = 7 * tileh;
		src.w = tilew; src.h = tileh;
		dest.x = 1.18 * tilew; if (xres == 240 || xres == 480) dest.x = 1.18 * tilew;
		dest.y = 6 * tileh; if (xres == 240 || xres == 480) dest.y = 12.18 * tileh;
		dest.w = tilew; dest.h = tileh;
		if(SDL_BlitSurface(tiles, &src, screen, &dest) < 0)
			printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());
		get_pipe_src_xy(previewarray[1], &x, &y, FALSE);
		src.x = x; src.y = y;
		if(SDL_BlitSurface(tiles, &src, screen, &dest) < 0)
			printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());

		/* Right */
		src.x = 0; src.y = 7 * tileh;
		src.w = tilew; src.h = tileh;
		dest.x = 2.27 * tilew; if (xres == 240 || xres == 480) dest.x = 2.27 * tilew;
		dest.y = 6 * tileh; if (xres == 240 || xres == 480) dest.y = 12.18 * tileh;
		dest.w = tilew; dest.h = tileh;
		if(SDL_BlitSurface(tiles, &src, screen, &dest) < 0)
			printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());
		get_pipe_src_xy(previewarray[0], &x, &y, FALSE);
		src.x = x; src.y = y;
		if(SDL_BlitSurface(tiles, &src, screen, &dest) < 0)
			printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());
	}

	if ((redraw & REDRAWTILE) == REDRAWTILE) {
		/* Draw one or more background tiles within the board.
		   The offsets into the board are in the draw pipes array */
		row = 0;
		while(drawpipearray[row].row != NULLPIPEVAL) {
			src.x = 4 * tilew; src.y = 6 * tileh;
			src.w = tilew; src.h = tileh;
			if(SDL_BlitSurface(tiles, &src, screen, &tile_rects[drawpipearray[row].row][drawpipearray[row].col]) < 0)
				printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());
			row++;
		}
	}
	
	if ((redraw & REDRAWPIPE) == REDRAWPIPE) {
		/* Draw one or more pipe pieces within the board.
		   The offsets into the board array are in the
		   draw pipes array */
		row = 0;
		while(drawpipearray[row].row != NULLPIPEVAL) {
			get_pipe_src_xy(boardarray[drawpipearray[row].row][drawpipearray[row].col], &x, &y, drawpipearray[row].filled);
			src.x = x; src.y = y;
			src.w = tilew; src.h = tileh;
			dest.x = (xres - BOARDW * tilew) + drawpipearray[row].col * tilew; if (xres == 240 || xres == 480) dest.x = drawpipearray[row].col * tilew;
			dest.y = drawpipearray[row].row * tileh; if (xres == 240 || xres == 480) dest.y = drawpipearray[row].row * tileh + 2 * tileh;
			dest.w = tilew; dest.h = tileh;
			if(SDL_BlitSurface(tiles, &src, screen, &dest) < 0)
				printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());
			row++;
		}
	}
	
	if ((redraw & REDRAWHELP) == REDRAWHELP) {
		/* Show the help pages */

		/* Draw a black surround */
		dest.x = gameboard_rect.x - 1;
		dest.y = gameboard_rect.y - 1;
		dest.w = gameboard_rect.w + 2;
		dest.h = gameboard_rect.h + 2;
		SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, BLACK));
		/* Draw a white box to cover the game board */
		SDL_FillRect(screen, &gameboard_rect, SDL_MapRGB(screen->format, WHITE));
		/* Draw the Exit text and the navigation buttons */
		if(helppage > 0) {
			/* Left arrow */
			src.x = 2 * tilew; src.y = 7 * tileh;
			src.w = tilew; src.h = tileh;
			if(SDL_BlitSurface(tiles, &src, screen, &help_l_label) < 0)
				printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());
		}
		if(helppage < HELPPAGES - 1) {
			/* Right arrow */
			src.x = 4 * tilew; src.y = 10 * tileh;
			src.w = tilew; src.h = tileh;
			if(SDL_BlitSurface(tiles, &src, screen, &help_r_label) < 0)
				printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());
		}
		/* Exit */
		src.x = 3 * tilew; src.y = 7 * tileh;
		src.w = 2 * tilew; src.h = tileh;
		if(SDL_BlitSurface(tiles, &src, screen, &help_exit_label) < 0)
			printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());

		x = xres - (BOARDW - 0.2) * tilew;
		y = 0.2 * BOARDH; if (xres == 240 || xres == 480) y = 2.2 * tileh;
		if(helppage == 0) {
			draw_ascii(HELPPAGE0, x, y);
		} else if(helppage == 1) {
			draw_ascii(HELPPAGE1, x, y);
		} else if(helppage == 2) {
			draw_ascii(HELPPAGE2, x, y);
		} else if(helppage == 3) {
			draw_ascii(HELPPAGE3, x, y);
		} else if(helppage == 4) {
			draw_ascii(HELPPAGE4, x, y);
		} else if(helppage == 5) {
			draw_ascii(HELPPAGE5, x, y);
		}
	}
	
	if (redraw != REDRAWNONE) SDL_Flip(screen);
	
	redraw = REDRAWNONE;
}

/***************************************************************************
 * Draw Ascii                                                              *
 ***************************************************************************/
/* This writes ASCII text. Embedded "\n" are translated into newlines.
   On entry: text = pointer to a (C) null terminated string */

void draw_ascii(char *text, int xpos, int ypos) {
	SDL_Rect src, dest;
	int count = 0, x = xpos, y = ypos;
	
	while(text[count] != 0) {
		while(text[count] == '\n') {
			x = xpos;
			y = y + asciih * 0.8;
			count++;
		}
		src.x = ((text[count] - 32) % 10) * asciiw;
		src.y = ((text[count] - 32) / 10) * asciih;
		src.w = asciiwidths[text[count] - 32]; if (xres == 320 || xres == 240) src.w = src.w / 2;
		src.h = asciih;
		dest.x = x; dest.y = y;
		dest.w = src.w; dest.h = asciih;
		if(SDL_BlitSurface(ascii, &src, screen, &dest) < 0)
			printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());
		x = x + src.w;
		count++;
	}
}

/***************************************************************************
 * Draw Digits                                                             *
 ***************************************************************************/
/* This writes an integer value. If it is negative the minus sign will be
   shown at the end of the number.
   On entry: value = an integer value positive or negative
             digitcount = number of digits to draw
             xpos = x position to write the number
             ypos = y position to write the number */

void draw_digits(int value, int digitcount, int xpos, int ypos) {
	SDL_Rect src, dest;
	int tempvalue = abs(value);
	int count, count2, base;
	
	dest.x = xpos; dest.y = ypos;
	dest.w = digitw; dest.h = digith;
	src.y = 0;
	src.w = digitw; src.h = digith;
	
	for (count = digitcount - 1; count >= 0; count--) {
		base = count == 0 ? 1 : 10;
		for (count2 = count; count2 > 1; count2--) base = base * 10;
		
		src.x = tempvalue / base * digitw;
		if(SDL_BlitSurface(digits, &src, screen, &dest) < 0)
			printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());
		
		tempvalue = tempvalue % base;
		dest.x = dest.x + digitw;
	}
	
	if (value < 0) {
		src.x = 10 * digitw;
		if(SDL_BlitSurface(digits, &src, screen, &dest) < 0)
			printf("%s: BlitSurface error: %s\n", __func__, SDL_GetError());
	} else {
		SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, YELLOW));
	}
}

/***************************************************************************
 * Initialise New Game                                                     *
 ***************************************************************************/
/* This sets up the variables necessary to start the game afresh. */

void initialise_new_game(void) {
	int rowloop, colloop, count;
	
	game_mode = GAMEON;
	redraw = REDRAWALL;
	score = 0;
	disablescoring = FALSE;
	flashhighscorestate = FALSE;
	gametime = GAMETIME;
	
	/* Clear the game board array */
	for (rowloop = 0; rowloop < BOARDH; rowloop++) {
		for (colloop = 0; colloop < BOARDW; colloop++) {
			boardarray[rowloop][colloop] = NULLPIPEVAL;
		}
	}

	/* Setup and initialise preview pieces/array. */
	for (count = 0; count < PREVIEWARRAYSIZE; count++) {
		previewarray[count] = getnextpipepiece();
	}
	
	/* Place end points and record in game board array. */
	boardarray[rand() % BOARDH][0] = 1;	/* yx */
	boardarray[rand() % BOARDH][BOARDW - 1] = 0;	/* yx */
	
	drawpipearray[0].row = NULLPIPEVAL;
}

/***************************************************************************
 * Get Next Pipe Piece                                                     *
 ***************************************************************************/
/* Get next piece from the pipe array, or if at end
   of array refill it and get piece from start. */

int getnextpipepiece(void) {
	static int pipearraypointer = PIPEARRAYSIZE;

	pipearraypointer++;
	if (pipearraypointer >= PIPEARRAYSIZE) {
		fillpipearray();
		pipearraypointer = 0;
	}
	return pipearray[pipearraypointer];
}

/***************************************************************************
 * Fill Pipe Array                                                         *
 ***************************************************************************/
/* This fills the pipe array with a set frequency of pipe pieces. The array
   is then shuffled. */
   
void fillpipearray(void) {
	int nextpointer = 0, count, temp, swap;
	
	/* Fill pipe array with our recommended frequency */
	nextpointer = fillpipearraypieces(2, 7, nextpointer);
	nextpointer = fillpipearraypieces(3, 7, nextpointer);
	nextpointer = fillpipearraypieces(4, 7, nextpointer);
	nextpointer = fillpipearraypieces(5, 7, nextpointer);
	nextpointer = fillpipearraypieces(6, 5, nextpointer);
	nextpointer = fillpipearraypieces(7, 5, nextpointer);
	nextpointer = fillpipearraypieces(8, 5, nextpointer);
	nextpointer = fillpipearraypieces(9, 5, nextpointer);
	nextpointer = fillpipearraypieces(10, 5, nextpointer);
	nextpointer = fillpipearraypieces(11, 18, nextpointer);
	nextpointer = fillpipearraypieces(12, 18, nextpointer);
	nextpointer = fillpipearraypieces(13, 4, nextpointer);
	nextpointer = fillpipearraypieces(14, 4, nextpointer);
	nextpointer = fillpipearraypieces(15, 4, nextpointer);
	nextpointer = fillpipearraypieces(16, 4, nextpointer);

	/* Go through the pipe array and shuffle the pieces. This maintains frequency :)
	   Fiddle with the above figures to tweek it, but don't forget the array size
	   is set as a constant and will need to be adjusted accordingly */
	for (count = 0; count < PIPEARRAYSIZE; count++) {
		temp = pipearray[count];
		swap = rand() % PIPEARRAYSIZE;
		pipearray[count] = pipearray[swap];
		pipearray[swap] = temp;
	}
}

/***************************************************************************
 * Fill Pipe Array Pieces                                                  *
 ***************************************************************************/
/* This is called by Fill Pipe Array only. */

int fillpipearraypieces(int pipepiece, int frequency, int nextpointer) {
	while (frequency > 0) {
		pipearray[nextpointer] = pipepiece;
		nextpointer++;
		if (nextpointer >= PIPEARRAYSIZE) nextpointer = 0;
		frequency--;
	}
	return nextpointer;
}

/***************************************************************************
 * Get Pipe XY                                                             *
 ***************************************************************************/
/* This translates a number identifying a pipe into x and y coordinates
   into the tiles surface.
   On entry: pipeid = 0 to 16, the same order the pipes are within the bmp
             *x
             *y 
             drawpipefilled = FALSE for empty or TRUE for filled */
 
void get_pipe_src_xy(int pipeid, int *x, int *y, int drawpipefilled) {
	if (pipeid > 16) {
		printf("%s: Invalid pipe: %i\n", __func__, pipeid);
	} else {
		if (drawpipefilled) pipeid = pipeid + 17;
		*x = (pipeid % 5) * tilew;
		*y = (pipeid / 5) * tileh;
	}
} 

/***************************************************************************
 * Manage User Input                                                       *
 ***************************************************************************/
/* This manages all user input. */

void manage_user_input(void) {
	while(SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_KEYDOWN:
			switch(event.key.keysym.sym) {
				case SDLK_ESCAPE:	/* Cancel on the Zaurus */
					if (game_mode != GAMESHOWHELP)
						quit = 1;
				case SDLK_LEFT:
				case SDLK_RIGHT:
					if(game_mode == GAMESHOWHELP) {
						manage_help_input(event.key.keysym.sym);
					}
				default:
					break;
				}
				break;
			case SDL_QUIT:
					quit = 1;
				break;
			case SDL_MOUSEBUTTONDOWN:
				manage_mouse_input();
				break;
			default:
				break;
		}
	}
}

int mouse_event_in_rect(int mx, int my, SDL_Rect *rect)
{
	return (mx >= rect->x && mx < rect->x + rect->w &&
		my >= rect->y && my < rect->y + rect->h);
}

void manage_mouse_input(void)
{
	int mbut, mx, my, count;
	int column = 0, row = 0;

	mbut = SDL_GetMouseState(&mx, &my);
	if (mbut != SDL_BUTTON_LEFT)
		return;

	#ifdef DEBUG
	printf("mbut=%i mx=%i my=%i\n", mbut, mx, my);
	#endif

	switch(game_mode) {
	case GAMEON:
		if (mouse_event_in_rect(mx, my, &gameboard_rect)) {
			/* Process game board clicks */
			/* Convert the mouse coordinates to offsets into the board array */
			column = (mx - gameboard_rect.x) / tilew;
			row = (my - gameboard_rect.y) / tileh;

			#ifdef DEBUG
			printf("row column : %i %i\n", row, column);
			#endif

			/* Don't allow replacing of the end points. */
			if (boardarray[row][column] > 1) {
				/* Place pipe piece from start of preview array. */
				if (boardarray[row][column] != NULLPIPEVAL) {
					score = score + PIPEOVERWRITESCORE;
				} else {
					score = score + PIPEPLACEMENTSCORE;
				}
				boardarray[row][column] = previewarray[0];
				/* Move all preview pieces down 1 place. */
				for (count = 0; count < PREVIEWARRAYSIZE - 1; count++) {
					previewarray[count] = previewarray[count + 1];
				}
				/* Add a new preview piece at the end. */
				previewarray[PREVIEWARRAYSIZE - 1] = getnextpipepiece();
				/* Mark tile for drawing and redraw everything related */
				drawpipearray[0].row = row; drawpipearray[0].col = column; drawpipearray[0].filled = FALSE;
				drawpipearray[1].row = NULLPIPEVAL;
				redraw = redraw | REDRAWTILE | REDRAWPIPE | REDRAWSCORE | REDRAWPREVIEW;
			} else if (boardarray[row][column] == 0) {
				score = score + gametime * FILLNOWSCORE;
				gametime = 0;
				redraw = redraw | REDRAWSCORE | REDRAWTIMER;
				createdeadpipesarray();
			}

			#ifdef DEBUG
			printf("boardarray:-\n");
			for (row = 0; row < BOARDH; row++) {
				for (column = 0; column < BOARDW; column++) {
					printf("%3i ", boardarray[row][column]);
				}
				printf("\n");
			}
			printf("\n");
			#endif
		} else if (mouse_event_in_rect(mx, my, &fill_label)) {
			/* Process Fill clicks */
			score = score + gametime * FILLNOWSCORE;
			gametime = 0;
			redraw = redraw | REDRAWSCORE | REDRAWTIMER;
			createdeadpipesarray();
			#ifdef DEBUG
			printf("Fill\n");
			#endif
		}
		/* No break. fall through for GAMEON */
	case GAMEFLASHHIGHSCORE:
	case GAMEOVER:
		/* Process New Game clicks */
		if (mouse_event_in_rect(mx, my, &new_game_label)) {
			/* Initialise new game */
			initialise_new_game();
			#ifdef DEBUG
			printf("New Game\n");
			#endif
		} else if (mouse_event_in_rect(mx, my, &hiscore_label)) {
			/* Process High Score clicks */
			initialise_new_game();
			/* Copy the highscoreboard into the board array */
			for (row = 0; row < BOARDH; row++) {
				for (column = 0; column < BOARDW; column++) {
					boardarray[row][column] = highscoreboard[0][row * BOARDH + column];
				}
			}
			gametime = 0;
			disablescoring = TRUE;	/* This is only used here to prevent the score from incrementing whilst filling. */
			createdeadpipesarray();
			#ifdef DEBUG
			printf("High Score\n");
			#endif
		} else if (mouse_event_in_rect(mx, my, &help_label)) {
			/* Process Help clicks */
			previous_game_mode = game_mode;
			game_mode = GAMESHOWHELP;
			redraw = redraw | REDRAWHELP;
			#ifdef DEBUG
			printf("Help\n");
			#endif
		}
		break;
	case GAMESHOWHELP:
		if (mouse_event_in_rect(mx, my, &help_l_label)) {
			manage_help_input(SDLK_LEFT);
		} else if (mouse_event_in_rect(mx, my, &help_r_label)) {
			manage_help_input(SDLK_RIGHT);
		} else if (mouse_event_in_rect(mx, my, &help_exit_label)) {
			manage_help_input(SDLK_ESCAPE);
		}
	}
}

/***************************************************************************
 * Manage Help Input                                                       *
 ***************************************************************************/
/* This manages all user input relating to Help. */

void manage_help_input(int input) {
	int rowloop, colloop, count = 0, leakypipefound = FALSE;
	int nomorepipes, passcounter = FILLEDCOUNTERBASE, filled = TRUE, endpipefound = FALSE;
	
	switch(input) {
		case SDLK_ESCAPE:
			game_mode = previous_game_mode;
			redraw = REDRAWALL;
			if (game_mode == GAMEFLASHHIGHSCORE || game_mode == GAMEOVER) {
				/* I originally wrote this game in Javascript for a web browser
				   and it wasn't designed to have anything drawn over the top of
				   it, so the pipe network needs to be rebuilt. At this point the
				   deadpipesarray still holds the filled pipe network so it just
				   needs to be decoded.*/ 
				do {
					nomorepipes = TRUE;
					for (rowloop = 0; rowloop < BOARDH; rowloop++) {
						for (colloop = 0; colloop < BOARDW; colloop++) {
							if (deadpipesarray[rowloop][colloop] == passcounter || deadpipesarray[rowloop][colloop] - LEAKYPIPEVAL == passcounter) {
								/* Mark pipe to be drawn */
								drawpipearray[count].row = rowloop;
								drawpipearray[count].col = colloop;
								drawpipearray[count].filled = filled;
								count++;
								/* If a leaky pipe is found then after this pass all pipes will be unfilled. */
								if (deadpipesarray[rowloop][colloop] >= FILLEDCOUNTERBASE + LEAKYPIPEVAL) {
									leakypipefound = TRUE;
								}
								/* At least one pipe was found for this pass */
								nomorepipes = FALSE;
							} else if (deadpipesarray[rowloop][colloop] == DEADPIPEVAL && boardarray[rowloop][colloop] == 1 && !endpipefound) {
								/* Mark the unvisited end pipe for drawing */
								drawpipearray[count].row = rowloop;
								drawpipearray[count].col = colloop;
								drawpipearray[count].filled = FALSE;
								count++;
								endpipefound = TRUE;	/* Only record it once */
							}
						}
					}
					if(leakypipefound) filled = FALSE;	/* Draw unfilled pipes from now on */
					passcounter++;
				} while(!nomorepipes);
				drawpipearray[count].row = NULLPIPEVAL;
				redraw = (redraw | REDRAWPIPE) ^ REDRAWALLPIPES;
			}
			#ifdef DEBUG
			printf("Help->Exit\n");
			#endif
			break;
		case SDLK_LEFT:
			if(helppage > 0) {
				helppage = helppage - 1;
				redraw = redraw | REDRAWHELP;
				#ifdef DEBUG
				printf("Help->Left\n");
				#endif
			}
			break;
		case SDLK_RIGHT:
			if(helppage < HELPPAGES - 1) {
				helppage = helppage + 1;
				redraw = redraw | REDRAWHELP;
				#ifdef DEBUG
				printf("Help->Right\n");
				#endif
			}
			break;
		default:
			break;
	}
}

/***************************************************************************
 * Create Dead Pipes Array                                                 *
 ***************************************************************************/
/* This is the main guts of the game. Originally I wrote it in JavaScript for
   use in a browser and have simply pasted it here and converted it.
   I believe it's quite clever and I found it very enjoyable to write, having
   no idea how the pipe-filling would be accomplished. Basically a point is
   created at the start pipe and proceeds to travel through any connected
   pipes and splits at junctions. Each pipe that is filled is given a number
   starting at 20 and is incremented as the network fills. This is used to
   animate the fill with a timer so that all pipes marked 20 get shown, then
   next timer event all 21s get shown and so on. Leaky pipes are marked and
   so are dead pipes (those with no connection to the network). Enable DEBUG
   and look at the array dump in the console for a better understanding. */

void createdeadpipesarray(void) {
	int count = 0, rowloop = 0, colloop = 0;
	int pointexists = FALSE, pointsconverge = FALSE;
	int leakcount = 0, deadcount = 0, freepointer = 0;
	int filledcounter = FILLEDCOUNTERBASE;
	int north = FALSE, south = FALSE, east = FALSE, west = FALSE;
	int targety = 0, targetx = 0;
	int targettype = NULLPIPEVAL, targetvalid = FALSE, targetvisited = FALSE;
	/* --- Create 2 dimensional array for the route tracing points. --- */
	int pointsarray[POINTSARRAYSIZE][4];
	for (rowloop = 0; rowloop < POINTSARRAYSIZE; rowloop++) {
		/* Array Format: y|x|direction|passcounter. */
		pointsarray[rowloop][0] = pointsarray[rowloop][1] = pointsarray[rowloop][3] = 0;
		pointsarray[rowloop][2] = NULLPIPEVAL;	/* dead point. */
	}
	
	/* Find endpoints in game board array. */
	for (rowloop = 0; rowloop < BOARDH; rowloop++) {
		for (colloop = 0; colloop < BOARDW; colloop++) {
			if (boardarray[rowloop][colloop] == 0) {
				/* Create a single point at start heading west (0=n|1=e|2=s|3=w). */
				pointsarray[POINTSARRAYSIZE - 1][0] = rowloop;
				pointsarray[POINTSARRAYSIZE - 1][1] = colloop;
				pointsarray[POINTSARRAYSIZE - 1][2] = 3;
				pointsarray[POINTSARRAYSIZE - 1][3] = filledcounter;
			}
			/* Duplicate the game board array into the dead pipes array. */
			deadpipesarray[rowloop][colloop] = boardarray[rowloop][colloop];
		}
	}
	/* MAIN LOOP. Do this while points exist (no points means no more routes). */
	do {
		pointexists = FALSE;
		for (rowloop = 0; rowloop < POINTSARRAYSIZE; rowloop++) {
			if (pointsarray[rowloop][2] != NULLPIPEVAL && pointsarray[rowloop][3] == filledcounter) {
				/* Get target yx.*/
				switch (pointsarray[rowloop][2]) {	/* point direction. */
					case 0 :	/* north. */
						targety = pointsarray[rowloop][0] - 1;
						targetx = pointsarray[rowloop][1];
						break;				
					case 1 :	/* east. */
						targety = pointsarray[rowloop][0];
						targetx = pointsarray[rowloop][1] + 1;
						break;				
					case 2 :	/* south. */
						targety = pointsarray[rowloop][0] + 1;
						targetx = pointsarray[rowloop][1];
						break;				
					case 3 :	/* west. */
						targety = pointsarray[rowloop][0];
						targetx = pointsarray[rowloop][1] - 1;
						break;				
				}				
				/* Get target pipe type. */
				if (targety < 0 || targety >= BOARDH || targetx < 0 || targetx >= BOARDW) {
					targettype = NULLPIPEVAL;	/* targets outside the game board are invalid. */
				} else {
					targettype = boardarray[targety][targetx];
				}
				/* Get direction information on the target piece. */
				switch (targettype) {
					case 0 :
						north = FALSE; east = FALSE; south = FALSE; west = TRUE;
						break;				
					case 1 :
						north = FALSE; east = TRUE; south = FALSE; west = FALSE;
						break;				
					case 2 :
						north = TRUE; east = FALSE; south = FALSE; west = TRUE;
						break;				
					case 3 :
						north = FALSE; east = FALSE; south = TRUE; west = TRUE;
						break;				
					case 4 :
						north = FALSE; east = TRUE; south = TRUE; west = FALSE;
						break;				
					case 5 :
						north = TRUE; east = TRUE; south = FALSE; west = FALSE;
						break;				
					case 6 :
						north = TRUE; east = FALSE; south = TRUE; west = TRUE;
						break;				
					case 7 :
						north = FALSE; east = TRUE; south = TRUE; west = TRUE;
						break;				
					case 8 :
						north = TRUE; east = TRUE; south = TRUE; west = FALSE;
						break;				
					case 9 :
						north = TRUE; east = TRUE; south = FALSE; west = TRUE;
						break;				
					case 10 :
						north = TRUE; east = TRUE; south = TRUE; west = TRUE;
						break;				
					case 11 :
						north = FALSE; east = TRUE; south = FALSE; west = TRUE;
						break;				
					case 12 :
						north = TRUE; east = FALSE; south = TRUE; west = FALSE;
						break;				
					case 13 :
						north = FALSE; east = FALSE; south = FALSE; west = TRUE;
						break;				
					case 14 :
						north = TRUE; east = FALSE; south = FALSE; west = FALSE;
						break;				
					case 15 :
						north = FALSE; east = TRUE; south = FALSE; west = FALSE;
						break;				
					case 16 :
						north = FALSE; east = FALSE; south = TRUE; west = FALSE;
						break;				
					default :
						north = FALSE; east = FALSE; south = FALSE; west = FALSE;
						break;
				}
				/* Get validity of target move. */
				targetvalid = FALSE;
				if (pointsarray[rowloop][2] == 0 && south == TRUE) targetvalid = TRUE;
				if (pointsarray[rowloop][2] == 1 && west == TRUE) targetvalid = TRUE;
				if (pointsarray[rowloop][2] == 2 && north == TRUE) targetvalid = TRUE;
				if (pointsarray[rowloop][2] == 3 && east == TRUE) targetvalid = TRUE;
				/* Get visited status of target pipe. */
				targetvisited = FALSE;
				if (targetvalid) {	/* don't try and reference array elements using -1 ;) */
					if (deadpipesarray[targety][targetx] >= FILLEDCOUNTERBASE && deadpipesarray[targety][targetx] != NULLPIPEVAL) targetvisited = TRUE;
				}
				/* Now that we have all the info we make the MAIN DECISIONS HERE. */
				/* If source is THE endpoint... */
				if (boardarray[pointsarray[rowloop][0]][pointsarray[rowloop][1]] == 1) {
					if (deadpipesarray[pointsarray[rowloop][0]][pointsarray[rowloop][1]] >= 0 && deadpipesarray[pointsarray[rowloop][0]][pointsarray[rowloop][1]] < FILLEDCOUNTERBASE) deadpipesarray[pointsarray[rowloop][0]][pointsarray[rowloop][1]] = filledcounter;	/* mark source as filled. */
					pointsarray[rowloop][2] = NULLPIPEVAL;	/* kill current point. */
				} else {
					/* ElseIf target is valid and not visited... */
					if (targetvalid && !targetvisited) {
						if (deadpipesarray[pointsarray[rowloop][0]][pointsarray[rowloop][1]] >= 0 && deadpipesarray[pointsarray[rowloop][0]][pointsarray[rowloop][1]] < FILLEDCOUNTERBASE) deadpipesarray[pointsarray[rowloop][0]][pointsarray[rowloop][1]] = filledcounter;	/* mark source as filled. */
						pointsarray[rowloop][2] = NULLPIPEVAL;	/* kill current point. */
						/* Create a new point at target for each direction. */
						/* Merge converging paths, caused when 2 sources converge on the same target at the same time. */
						if (north) {
							pointsconverge = FALSE;
							for (count = 0; count < POINTSARRAYSIZE; count++) {
								if (pointsarray[count][2] != NULLPIPEVAL) {
									/* We don't want 2 points in the same square going in the same direction. */
									if (pointsarray[count][0] == targety && pointsarray[count][1] == targetx && pointsarray[count][2] == 0) {
										pointsconverge = TRUE;
									}
								} else {
									freepointer = count;	/* record this as we'll use it to store the new point. */
								}
							}						
							if (!pointsconverge) {
								pointsarray[freepointer][0] = targety; pointsarray[freepointer][1] = targetx;
								pointsarray[freepointer][2] = 0; pointsarray[freepointer][3] = filledcounter + 1;	/* process it next do-while iteration. */
								pointexists = TRUE;
							}
						}
						if (east) {
							pointsconverge = FALSE;
							for (count = 0; count < POINTSARRAYSIZE; count++) {
								if (pointsarray[count][2] != NULLPIPEVAL) {
									/* We don't want 2 points in the same square going in the same direction. */
									if (pointsarray[count][0] == targety && pointsarray[count][1] == targetx && pointsarray[count][2] == 1) {
										pointsconverge = TRUE;
									}
								} else {
									freepointer = count;	/* record this as we'll use it to store the new point. */
								}
							}
							if (!pointsconverge) {
								pointsarray[freepointer][0] = targety; pointsarray[freepointer][1] = targetx;
								pointsarray[freepointer][2] = 1; pointsarray[freepointer][3] = filledcounter + 1;	/* process it next do-while iteration. */
								pointexists = TRUE;
							}
						}
						if (south) {
							pointsconverge = FALSE;
							for (count = 0; count < POINTSARRAYSIZE; count++) {
								if (pointsarray[count][2] != NULLPIPEVAL) {
									/* We don't want 2 points in the same square going in the same direction. */
									if (pointsarray[count][0] == targety && pointsarray[count][1] == targetx && pointsarray[count][2] == 2) {
										pointsconverge = TRUE;
									}
								} else {
									freepointer = count;	/* record this as we'll use it to store the new point. */
								}
							}
							if (!pointsconverge) {
								pointsarray[freepointer][0] = targety; pointsarray[freepointer][1] = targetx;
								pointsarray[freepointer][2] = 2; pointsarray[freepointer][3] = filledcounter + 1;	/* process it next do-while iteration. */
								pointexists = TRUE;
							}
						}
						if (west) {
							pointsconverge = FALSE;
							for (count = 0; count < POINTSARRAYSIZE; count++) {
								if (pointsarray[count][2] != NULLPIPEVAL) {
									/* We don't want 2 points in the same square going in the same direction. */
									if (pointsarray[count][0] == targety && pointsarray[count][1] == targetx && pointsarray[count][2] == 3) {
										pointsconverge = TRUE;
									}
								} else {
									freepointer = count;	/* record this as we'll use it to store the new point. */
								}
							}
							if (!pointsconverge) {
								pointsarray[freepointer][0] = targety; pointsarray[freepointer][1] = targetx;
								pointsarray[freepointer][2] = 3; pointsarray[freepointer][3] = filledcounter + 1;	/* process it next do-while iteration. */
								pointexists = TRUE;
							}
						}
					} else {
						/* ElseIf target is valid and visited... */
						if (targetvalid && targetvisited) {
							if (deadpipesarray[pointsarray[rowloop][0]][pointsarray[rowloop][1]] >= 0 && deadpipesarray[pointsarray[rowloop][0]][pointsarray[rowloop][1]] < FILLEDCOUNTERBASE) deadpipesarray[pointsarray[rowloop][0]][pointsarray[rowloop][1]] = filledcounter;	/* mark source as filled. */
							pointsarray[rowloop][2] = NULLPIPEVAL;	/* kill current point. */
						} else {
							/* ElseIf target is not valid... */
							if  (!targetvalid) {
								deadpipesarray[pointsarray[rowloop][0]][pointsarray[rowloop][1]] = filledcounter + LEAKYPIPEVAL;	/* mark source as leaky retaining fill count (important). */
								pointsarray[rowloop][2] = NULLPIPEVAL;	/* kill current point. */
								leakcount++;
							}
						}
					}
				}
			}
		}
		filledcounter++;
	} while (pointexists);

	/* MARK DEAD PIPES. The easy bit :) */
	deadcount = 0;
	for (rowloop = 0; rowloop < BOARDH; rowloop++) {
		for (colloop = 0; colloop < BOARDW; colloop++) {
			if (deadpipesarray[rowloop][colloop] >= 0 && deadpipesarray[rowloop][colloop] < FILLEDCOUNTERBASE) {
				deadpipesarray[rowloop][colloop] = DEADPIPEVAL;
				deadcount++;	/* count can include endpoint if it is unvisited. */
			}
		}
	}
	
	#ifdef DEBUG
	printf("deadpipesarray:-\n");
	for (rowloop = 0; rowloop < BOARDH; rowloop++) {
		for (colloop = 0; colloop < BOARDW; colloop++) {
			printf("%3i ", deadpipesarray[rowloop][colloop]);
		}
		printf("\n");
	}
	printf("\n");
	#endif
	
	cleardeadpipesy = 0;
	cleardeadpipesx = 0;
	game_mode = GAMECLEARDEADPIPES; /* And off we go next main loop cycle... */
}

/***************************************************************************
 * Clear Dead Pipes                                                        *
 ***************************************************************************/
/* This clears one dead pipe at a time from the board array and the screen
   and is designed to be called from a timer so that it appears animated. */

void cleardeadpipes(void) {
	int deadpipefound = FALSE, nomorepipes = FALSE;

	do {
		/* Officially if the endpoint is unvisited it's dead, but we'll leave it onscreen anyway. */
		if (deadpipesarray[cleardeadpipesy][cleardeadpipesx] == DEADPIPEVAL && boardarray[cleardeadpipesy][cleardeadpipesx] != 1) {
			/* Erase dead pipe from the screen. */
			drawpipearray[0].row = cleardeadpipesy; drawpipearray[0].col = cleardeadpipesx;
			drawpipearray[1].row = NULLPIPEVAL;
			redraw = redraw | REDRAWTILE;
			/* Erase dead pipe from the board array. */
			boardarray[cleardeadpipesy][cleardeadpipesx] = NULLPIPEVAL;
			score = score + DEADPIPESCORE;
			redraw = redraw | REDRAWSCORE;
			deadpipefound = TRUE;
		}
		/* Work our way from top left to bottom right. */
		cleardeadpipesx++;
		if (cleardeadpipesx >= BOARDW) {
			cleardeadpipesx = 0; cleardeadpipesy++;
			if (cleardeadpipesy >= BOARDH) nomorepipes = TRUE;
		}
	} while (!deadpipefound && !nomorepipes);
	if (nomorepipes) {
		fillpipespasscounter = FILLEDCOUNTERBASE;
		game_mode = GAMEFILLPIPES;	/* And off we go next main loop cycle... */
	}
}

/***************************************************************************
 * Fill Pipes                                                              *
 ***************************************************************************/
/* This fills one or several pipes at a time. */
   
void fillpipes(void) {
	int rowloop, colloop, count = 0;
	int leakypipefound, nomorepipes;
	
	/* Show all filled pipes onscreen for this passcounter */
	leakypipefound = FALSE;
	nomorepipes = TRUE;
	for (rowloop = 0; rowloop < BOARDH; rowloop++) {
		for (colloop = 0; colloop < BOARDW; colloop++) {
			if (deadpipesarray[rowloop][colloop] == fillpipespasscounter || deadpipesarray[rowloop][colloop] - LEAKYPIPEVAL == fillpipespasscounter) {
				/* Draw filled pipe. */
				drawpipearray[count].row = rowloop;
				drawpipearray[count].col = colloop;
				drawpipearray[count].filled = TRUE;
				drawpipearray[count + 1].row = NULLPIPEVAL;
				count++;
				redraw = redraw | REDRAWTILE | REDRAWPIPE;
				/* When displaying the highscoreboard ignore scoring */
				if (!disablescoring) {
					score = score + FILLEDPIPESCORE;
					redraw = redraw | REDRAWSCORE;
				}
				/* If a leaky pipe is found then flag it */
				if (deadpipesarray[rowloop][colloop] >= FILLEDCOUNTERBASE + LEAKYPIPEVAL) leakypipefound = TRUE;
			} else {
				if (deadpipesarray[rowloop][colloop] > fillpipespasscounter && deadpipesarray[rowloop][colloop] < DEADPIPEVAL) nomorepipes = FALSE;
			}
		}
	}
	
	fillpipespasscounter++;
	
	if (leakypipefound || nomorepipes) {
		/* Ok, last bit: high score, again ignoring whilst displaying the highscoreboard */
		if (!disablescoring && score > highscoretable[0]) {
			highscoretable[0] = score;
			/* Copy the board into the highscoreboard */
			for (rowloop = 0; rowloop < BOARDH; rowloop++) {
				for (colloop = 0; colloop < BOARDW; colloop++) {
					highscoreboard[0][rowloop * BOARDH + colloop] = boardarray[rowloop][colloop];
				}
			}
			redraw = redraw | REDRAWHIGHSCORE;
			game_mode = GAMEFLASHHIGHSCORE;
			
			#ifdef DEBUG
			printf("Saving resource file -> ");
			#endif
			
			save_rc_file();	/* This saves the new highscore[s] */
		} else {
			game_mode = GAMEOVER;
		}
	}
}

/***************************************************************************
 * Read Resource File                                                      *
 ***************************************************************************/

void read_rc_file(void) {
	char buffer[256];
	FILE* file;
	int result, value, count;
		
	strcpy(buffer, user_home_dir);
	strcat(buffer, "/");
	strcat(buffer, RESOURCEFILE);
	
	#ifdef DEBUG
	printf("%s\n", buffer);
	#endif
	
	if ((file = fopen(buffer,"r")) == NULL) {
		printf("%s: Cannot read from file %s\n", __func__, buffer);
		return;
	}

	result = fscanf(file,"%s", buffer);	/* [highscore0] */
	if (result != 1 || strcmp(buffer, "[highscore0]") != 0) {
		printf("%s: Data from resource file is unreliable\n", __func__);
		fclose(file);
		return;
	} else {
		#ifdef DEBUG
		printf("%s\n", buffer);
		#endif
	}
	result = fscanf(file,"%i", &value);	/* highscore0 */
	if (result != 1) {
		printf("%s: Data from resource file is unreliable\n", __func__);
		fclose(file);
		return;
	} else {
		#ifdef DEBUG
		printf("%d\n", value);
		#endif
		highscoretable[0] = value;
	}
	
	result = fscanf(file,"%s", buffer);	/* [highscoreboard0] */
	if (result != 1 || strcmp(buffer, "[highscoreboard0]") != 0) {
		printf("%s: Data from resource file is unreliable\n", __func__);
		fclose(file);
		return;
	} else {
		#ifdef DEBUG
		printf("%s\n", buffer);
		#endif
	}
	for (count = 0; count < BOARDH * BOARDW; count++) {
		result = fscanf(file,"%i", &value);	/* pipe piece id */
		if (result != 1) {
			printf("%s: Data from resource file is unreliable\n", __func__);
			fclose(file);
			return;
		} else {
			#ifdef DEBUG
			if (count > 0 && count % BOARDH == 0) printf("\n");
			printf("%3i ", value);
			#endif
			highscoreboard[0][count] = value;
		}
	}
	#ifdef DEBUG
	printf("\n");
	#endif
	
	fclose(file);
}

/***************************************************************************
 * Save Resource File                                                      *
 ***************************************************************************/

void save_rc_file(void) {
	char buffer[256];
	FILE* file;
	int count;
		
	strcpy(buffer, user_home_dir);
	strcat(buffer, "/");
	strcat(buffer, RESOURCEFILE);
	
	#ifdef DEBUG
	printf("%s\n", buffer);
	#endif
	
	if ((file = fopen(buffer,"w")) == NULL) {
		printf("%s: Cannot write to file %s\n", __func__, buffer);
		return;
	}

	fprintf(file,"[highscore0]\n%i\n", highscoretable[0]);
	
	fprintf(file,"[highscoreboard0]\n");
	for (count = 0; count < BOARDH * BOARDW; count++) {
		if (count > 0 && count % BOARDH == 0) fprintf(file,"\n");
		fprintf(file,"%3i ", highscoreboard[0][count]);
	}
	fprintf(file,"\n");

	fclose(file);
}


