/* Pipepanic - a game.
Copyright (C) 2006 TheGreenKnight <thegreenknight1500@hotmail.com>
Copyright (C) 2018 Joshua Clayton <stillcompiling@gmail.com>

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
#include <SDL2/SDL.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "pipepanic.h"

#define blit(stex, srect, drect) SDL_RenderCopy(rrr, stex, srect, drect)


struct gametile {
	int pipe;
	unsigned int flags;
	int row;
	int col;
	int fill;
};

static const struct game_scoring plusscoring = {
	.overwrite = -2,
	.place = 2,
	.fillearly = 1,
	.fill = 2,
	.unfilled = -2,
	.fillend = 20,
};

static const struct game_settings plussettings = {
	.duration = 30,
	.delay_per_clear = 150,
	.filltime = 2000,
	.postfilltime = 200,
	.steps = 48,
	.play_while_filling = true,
};

static const struct game_scoring classicscoring = {
	.overwrite = -10,
	.place = 10,
	.fillearly = 5,
	.fill = 10,
	.unfilled = -10,
	.fillend = 10,
};

static const struct game_settings classicsettings = {
	.duration = 240,
	.delay_per_clear = 150,
	.postfilltime = 200,
	.steps = 48,
};

struct tilering {
	struct gametile **start;
	struct gametile **end;
	struct gametile **read;
	struct gametile **write;
};

/* Variable declarations */
static int xres = 640;
static int yres = 480;
static int tilew = 48;
static int tileh = 48;
static int digitw = 30;
static int digith = 48;
static int asciiw = 30;
static int asciih = 30;
static int sdl_fullscreen = 0;
static SDL_Window *win;
static SDL_Renderer *rrr;
static SDL_Texture *digits;
static SDL_Texture *tiles;
static SDL_Texture *ascii;
static SDL_Rect mouse_scale;
static SDL_Event event;
static char *user_home_dir;
static enum game_mode game_mode = GAMEON;
static int redraw = REDRAWALL;
static int highscoretable[5] = {0, 0, 0, 0, 0};
static int highscoreboard[5][BOARDH * BOARDW];
static int score = 0;
static bool disablescoring = false;
static bool plusmode = true;
static const struct game_settings *settings;
static const struct game_scoring *scoring;
static int gametime;
static int previewarray[PREVIEWARRAYSIZE];
static int pipearray[PIPEARRAYSIZE];
static struct gametile boardarray[BOARDH][BOARDW];
static struct tilering *fill_list;
static struct gametile *start;
static SDL_Rect tile_rects[BOARDH][BOARDW];
static SDL_Rect *plusmode_rect = &tile_rects[0][4];
static SDL_Rect digit_src[11];
static SDL_Rect hiscore_label, score_label, time_label, fill_label, help_label,
	new_game_label, gameboard_rect, help_l_label, help_r_label,
	help_exit_label, hiscoredigits[4], timedigits[3], scoredigits[4],
	preview_rects[PREVIEWARRAYSIZE];
static bool flashhighscorestate = false;
static int helppage = 0;
static const char * const helppages[] = {NULL, HELPPAGE0, HELPPAGE1, HELPPAGE2,
	     HELPPAGE3, HELPPAGE4, HELPPAGE5, HELPPAGE6};

/*						  ! "  #  $  %  &  ' ( ) *  + , - . / 0  1  2  3  4  5  6  7  8  9  : ; <  =  >  ?  @  A  B  C  D  E  F  G  H  I J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z  [ \ ] ^  _  ' a  b  c  d  e  f g  h  i j k  l m  n  o  p  q  r s  t u  v  w  x  y  z  { | } ~ */
static const int asciiwidths[100] = {7,5,10,11,11,18,15,6,6,6,9,11,5,8,5,7,13,10,12,13,14,13,13,13,13,13,5,5,11,11,11,12,15,15,14,13,14,13,12,15,14,6,12,15,12,16,14,16,13,16,14,14,14,14,15,19,13,13,14,8,6,8,11,10,6,13,12,13,12,13,8,13,11,6,6,12,6,17,11,13,12,12,8,12,9,11,12,18,13,12,11,8,4,8,11};

/* Function prototypes */
static int load_bitmaps(bool small);
static void draw_game(void);
static void draw_digits(int value, SDL_Rect *label, int len);
static void initialise_new_game(void);
static void manage_user_input(void);
static int getnextpipepiece(void);
static void fillpipearray(void);
static int fillpipearraypieces(int pipepiece, int frequency, int nextpointer);
static void get_pipe_src(int pipeid, SDL_Rect *rect, bool filled);
static void cleardeadpipes(void);
static bool fillpipes(struct tilering *fill_list);
static void check_highscore();
static void read_rc_file(void);
static void save_rc_file(void);
static void draw_ascii(const char *const text, int xpos, int ypos);
static void manage_help_input(int input);
static void manage_mouse_input(void);
static void initialize_drawables(int w, int h);
static void setup_gameboard(void);
static void setup_img_src_rects(void);
static void mark_neighbors(struct gametile *tile, int flags);
static int get_pipe_directions(int targettype);
static struct tilering *tilering_init(int size);
static void tilering_reset(struct tilering *ring);
static void start_fill(struct gametile *tile, int direction,
		       struct tilering *fill_list);
static struct gametile *get_north_neighbor(struct gametile *tile);
static struct gametile *get_east_neighbor(struct gametile *tile);
static struct gametile *get_south_neighbor(struct gametile *tile);
static struct gametile *get_west_neighbor(struct gametile *tile);



/***************************************************************************
 * Main                                                                    *
 ***************************************************************************/

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char *argv[])
{
	int count, count2;

	/* Initialise the highscoreboard array because there may not yet
	   be a saved highscore[board]/rc file */
	for (count = 0; count < 5; count++) {
		for (count2 = 0; count2 < BOARDH * BOARDW; count2++) {
			highscoreboard[count][count2] = NULLPIPEVAL;
		}
	}

	srand((unsigned) time(NULL));	/* Seed C's random number generator */

	user_home_dir = getenv("HOME");
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
			} else if (!strcmp(argv[count], "-320x240")) {
				xres = 320; yres = 240;
			} else if (!strcmp(argv[count], "-640x480")) {
				xres = 640; yres = 480;
			} else if (!strcmp(argv[count], "-480x640")) {
				xres = 480; yres = 640;
			} else if (!strcmp(argv[count], "-f")) {
				sdl_fullscreen = SDL_WINDOW_FULLSCREEN_DESKTOP;
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

	/* Set SDL video mode */

	win = SDL_CreateWindow("pipepanic plus!", SDL_WINDOWPOS_UNDEFINED,
			       SDL_WINDOWPOS_UNDEFINED, xres, yres,
			       sdl_fullscreen | SDL_WINDOW_RESIZABLE);
	SDL_SetWindowMinimumSize(win, 320, 240);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	rrr = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
	if(win == NULL) {
		printf("%s: Cannot initialise screen: %s\n", __func__, SDL_GetError());
		exit(1);
	}

	initialize_drawables(xres, yres);
	initialise_new_game();
	game_mode = GAMEOVER;
	bool pipes_filled = false;
	Uint32 timeout = 0;
	/* Main game loop */
	while(game_mode != GAMEQUIT) {
		Uint32 ticks = SDL_GetTicks();;

		draw_game();
		manage_user_input();

		if (ticks >= timeout && !helppage) {
			switch (game_mode) {
			case GAMESTART:
				initialise_new_game();
				pipes_filled = false;
				start_fill(start, EAST, fill_list);
				game_mode = GAMEON;
				break;
			case GAMEON:
				redraw = redraw | REDRAWTIMER;
				if (gametime > 0) {
					timeout = ticks + 1000;
					gametime = gametime - 1;
					break;
				}

				if (!settings->play_while_filling ||
				    (pipes_filled = fillpipes(fill_list))) {
					game_mode = GAMEFINISH;
					break;
				}

				timeout = ticks + (settings->filltime /
						   settings->steps);
				break;
			case GAMEFINISH:
				if (!disablescoring)
					score += gametime * scoring->fillearly;
				gametime = 0;
				redraw = redraw | REDRAWSCORE | REDRAWTIMER;
				mark_neighbors(start, CONNECTED);
				game_mode = GAMECLEARDEADPIPES;
				break;
			case GAMECLEARDEADPIPES:
				timeout = ticks + settings->delay_per_clear;
				cleardeadpipes();
				if (game_mode == GAMEFILLPIPES) {
					timeout = 0;
				}
				break;
			case GAMEFILLPIPES:
				if (pipes_filled || fillpipes(fill_list)) {
					game_mode = GAMEOVER;
					check_highscore();
					break;
				}
				timeout = ticks + (settings->postfilltime /
						   settings->steps);
				break;
			case GAMEFLASHHIGHSCORE:
				timeout = ticks + FLASHHIGHSCORETIMEOUT;
				redraw = redraw | REDRAWHIGHSCORE;
				flashhighscorestate = !flashhighscorestate;
				break;
			case GAMEOVER:
				break;
			case GAMEQUIT:
				continue;
			}
		}
		SDL_Delay(9);
	}

	/* Shutdown all subsystems */
	SDL_Quit();
	return 0;
}

static void initialize_drawables(int w, int h)
{
	bool small;

        if (h >= w && h >= 640 && w >= 480) {
		/* Large portrait oriented */
		yres = 640;
		xres = 480;
		small = false;
	} else if (w > h && w >= 640 && h >= 480) {
		/* Large landscape oriented */
		xres = 640;
		yres = 480;
		small = false;
	} else if (h >= w && h >= 320 && w >= 240) {
		/* Small portrait oriented */
		yres = 320;
		xres = 240;
		small = true;
	} else if (w > h && w >= 320 && h >= 240) {
		/* Small landscape oriented */
		xres = 320;
		yres = 240;
		small = true;
	} else {
		/* invalid. do not resize yet */
		return;
	}

	SDL_RenderSetIntegerScale(rrr, true);
	SDL_RenderSetLogicalSize(rrr, xres, yres);
	if (load_bitmaps(small))
		exit(1);
	setup_img_src_rects();
	setup_gameboard();
	redraw = (helppage)? REDRAWHELP | REDRAWALL : REDRAWALL;
	draw_game();
	SDL_RenderPresent(rrr);
}

/***************************************************************************
 * Load Bitmaps                                                            *
 ***************************************************************************/
/* This locates and loads the relevant bitmap images. They are converted to
   the display format for optimisation. Also RLE is enabled for the colorkey
   which makes probably the biggest difference in frame rate I have found yet.
   On exit: returns 1 if an error occured else 0. */
static int load_bitmap(SDL_Texture **tex, const char *fname, Uint32
		cflag, Uint8 r, Uint8 g, Uint8 b)
{

	if(tex)
		SDL_DestroyTexture(*tex);

	SDL_Surface *temp = SDL_LoadBMP(fname);
	SDL_Surface *temp2;
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

	if (cflag) {
		SDL_SetColorKey(temp, cflag, SDL_MapRGB(temp->format, r, g, b));
		SDL_SetSurfaceRLE(temp, cflag);
	}

	temp2 = SDL_ConvertSurfaceFormat(temp, SDL_GetWindowPixelFormat(win), 0);
	SDL_FreeSurface(temp);
	if (!temp2) {
		printf("%s: SDL_ConvertSurfaceFormat error: %s\n",
		       __func__, SDL_GetError());
		return 1;
	}

	*tex = SDL_CreateTextureFromSurface(rrr, temp2);
	SDL_FreeSurface(temp2);
	if (!*tex) {
		printf("%s: SDL_CreateTextureFromSurface error: %s\n",
		       __func__, SDL_GetError());
		return 1;
	}
	return 0;
}

static int load_bitmaps(bool small)
{
	const char *digitsfile, *tilesfile, *asciifile;
	static bool is_small = false;

	if (small == is_small && digits)
		return 0;

	is_small = small;

	if (small) {
		tilew = tileh = digith = 24;
		digitw = asciiw = asciih = 15;
		digitsfile = DIGITS24BMP;
		tilesfile = TILES24BMP;
		asciifile = ASCII15BMP;
	} else {
		tilew = tileh = digith = 48;
		digitw = asciiw = asciih = 30;
		digitsfile = DIGITS48BMP;
		tilesfile = TILES48BMP;
		asciifile = ASCII30BMP;
	}

	if (load_bitmap(&digits, digitsfile, 0, 0, 0, 0) != 0)
		return 1;

	if (load_bitmap(&tiles, tilesfile, true, MAGENTA) != 0)
		return 1;
	if (load_bitmap(&ascii, asciifile, true, WHITE) != 0)
		return 1;

	return 0;
}

static void setup_digits(SDL_Rect *label, int len, int xpos, int ypos)
{
	for (int i = 0; i <  len; ++i) {
		label[i].x = xpos + (digitw * i);
		label[i].y = ypos;
		label[i].w = digitw;
		label[i].h = digith;
	}
}

static void setup_preview_rects(void)
{
	int x = (xres == 240 || xres == 480)? 0.09 * tilew : 0.09 * tilew;
	int y = (xres == 240 || xres == 480)? 12.18 * tileh : 6 * tileh;

	for (int i = 0; i < ARRAYSIZE(preview_rects); ++i) {
		preview_rects[i].x = x + (i * tilew);
		preview_rects[i].y = y;
		preview_rects[i].w = tilew;
		preview_rects[i].h = tileh;
	}
}

static void setup_digit_src_rects(void) {
	for (int i = 0; i < 11; ++i) {
		digit_src[i].x= i * digitw;
		digit_src[i].y= 0;
		digit_src[i].w = digitw;
		digit_src[i].h= digith;
	}
}

static void setup_img_src_rects(void)
{
	setup_digit_src_rects();
}

static void setup_mouse()
{
	float scaleX, scaleY;

	SDL_RenderGetScale(rrr, &scaleX, &scaleY);
	SDL_RenderGetViewport(rrr, &mouse_scale);
	mouse_scale.w = (short)scaleX;
	mouse_scale.x *= mouse_scale.w;
	mouse_scale.h = (short)scaleY;
	mouse_scale.y *= mouse_scale.h;
}

static void setup_gameboard(void)
{
	int y = (xres == 240 || xres == 480)? 2 * tileh : 0;
	int x = xres - BOARDW * tilew;

	FOREACH_TILE(row, col) {
		tile_rects[row][col].w = tilew;
		tile_rects[row][col].h = tileh;
		tile_rects[row][col].x = x + tilew * col;
		tile_rects[row][col].y = y + tileh * row;
	}

	setup_preview_rects();

	x = (xres == 240 || xres == 480)? 4 * tilew : 0.15 * tilew;
	y = 0.9 * tileh;
	setup_digits(hiscoredigits, ARRAYSIZE(hiscoredigits), x, y);

	x = (xres == 240 || xres == 480)? 8 * tilew : 0.15 * tilew;
	y = (xres == 240 || xres == 480)? 0.9 * tileh : 4.9 * tileh;
	setup_digits(timedigits, ARRAYSIZE(timedigits), x, y);

	x = 0.15 * tilew;
	y = (xres == 240 || xres == 480)?  0.9 * tileh : 2.9 * tileh;
	setup_digits(scoredigits, ARRAYSIZE(scoredigits), x, y);


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
	setup_mouse();
}

static void draw_preview(void)
{
	SDL_Rect src;

	for (int i = 0; i < ARRAYSIZE(preview_rects); ++i) {
		src.x = (i < 2)? tilew : 0;
		src.y = 7 * tileh;
		src.w = tilew;
		src.h = tileh;
		blit(tiles, &src, &preview_rects[i]);
		get_pipe_src(previewarray[2 - i], &src, false);
		blit(tiles, &src, &preview_rects[i]);
	}
}

static void draw_partial_tile(int row, int col)
{
	SDL_Rect src, dest;
	int offset;
	const struct gametile *tile = &boardarray[row][col];
	int unfilled = settings->steps - tile->fill;

	get_pipe_src(tile->pipe, &src, true);
	blit(tiles, &src, &tile_rects[row][col]);
	if (tile->fill >= settings->steps)
		return;

	get_pipe_src(tile->pipe, &src, false);

	switch (tile->flags & FILLDIRECTION_MASK) {
	case FROM_NORTH:
		dest.w = src.w;
		dest.h = src.h = unfilled * tileh / settings->steps;
		src.y += tileh - src.h;
		dest.x = tile_rects[row][col].x;
		dest.y = tile_rects[row][col].y + tileh - src.h;
		break;
	case FROM_EAST:
		dest.w = src.w = unfilled * tilew / settings->steps;
		dest.h = src.h;
		dest.x = tile_rects[row][col].x;
		dest.y = tile_rects[row][col].y;
		break;
	case FROM_SOUTH:
		dest.w = src.w;
		dest.h = src.h = unfilled * tileh / settings->steps;
		dest.x = tile_rects[row][col].x;
		dest.y = tile_rects[row][col].y;
		break;
	case FROM_WEST:
		dest.w = src.w = unfilled * tilew / settings->steps;
		dest.h = src.h;
		src.x += tilew - src.w;
		dest.x = tile_rects[row][col].x + tilew - src.w;
		dest.y = tile_rects[row][col].y;
		break;
	default:
		src.w = SDL_sqrt(tilew * tilew * unfilled / settings->steps);
		dest.h = dest.w = src.h = src.w;
		offset = (tilew - src.w) >> 1;
		src.x += offset;
		src.y += offset;
		dest.x = tile_rects[row][col].x + offset;
		dest.y = tile_rects[row][col].y + offset;
		break;

	}

	blit(tiles, &src, &dest);
}

static void draw_tile(int row, int col, bool force)
{
	struct gametile *tile = &boardarray[row][col];

	SDL_Rect src;

	if (!(tile->flags & CHANGED) && !force)
		return;

	src.x = 4 * tilew;
	src.y = 6 * tileh;
	src.w = tilew;
	src.h = tileh;
	blit(tiles, &src, &tile_rects[row][col]);
	if (tile->pipe != NULLPIPEVAL) {
		if (tile->fill) {
			draw_partial_tile(row, col);
		} else {
			get_pipe_src(tile->pipe, &src, false);
			blit(tiles, &src, &tile_rects[row][col]);
		}
	}
	tile->flags &= ~CHANGED;
}

/***************************************************************************
 * Draw Game                                                               *
 ***************************************************************************/
/* This draws the game. Either it draws all of it or just parts of it. */

static void draw_game(void)
{
	SDL_Rect src, dest;
	int x, y;

	if ((redraw & REDRAWBOARD) == REDRAWBOARD) {
		/* Paint the whole screen yellow. */
		SDL_SetRenderDrawColor(rrr, YELLOW, 255);
		SDL_RenderClear(rrr);

		/* Draw all the game board background tiles. */
		src.x = 4 * tilew; src.y = 6 * tileh;
		src.w = tilew; src.h = tileh;
		FOREACH_TILE(row, col)
			blit(tiles, &src, &tile_rects[row][col]);

		/* Draw all the text. */
		/* High Score. */
		src.x = 0 * tilew; src.y = 8 * tileh;
		src.w = 3 * tilew; src.h = tileh;
		blit(tiles, &src, &hiscore_label);

		/* Score. */
		src.x = 3 * tilew; src.y = 8 * tileh;
		src.w = 2 * tilew; src.h = tileh;
		blit(tiles, &src, &score_label);

		/* Time (s). */
		src.x = 0 * tilew; src.y = 9 * tileh;
		src.w = 3 * tilew; src.h = tileh;
		blit(tiles, &src, &time_label);

		/* Fill. */
		src.x = 3 * tilew; src.y = 10 * tileh;
		src.w = tilew; src.h = tileh;
		blit(tiles, &src, &fill_label);

		/* Help. */
		src.x = 3 * tilew; src.y = 9 * tileh;
		src.w = 2 * tilew; src.h = tileh;
		blit(tiles, &src, &help_label);

		/* New Game. */
		src.x = 0 * tilew; src.y = 10 * tileh;
		src.w = 3 * tilew; src.h = tileh;
		blit(tiles, &src, &new_game_label);
	}

	if ((redraw & REDRAWALLPIPES) == REDRAWALLPIPES) {
		FOREACH_TILE(row, col)
			draw_tile(row, col, true);
	}
	if ((redraw & REDRAWHIGHSCORE) == REDRAWHIGHSCORE) {
		/* The top high score */
		/* If flashhighscorestate is true then no score is shown
		   (it is blanked out with the background colour). */
		if (flashhighscorestate) {
			dest.w = 5 * digitw; dest.h = digith;
			SDL_SetRenderDrawColor(rrr, YELLOW, 255);
			SDL_RenderDrawRect(rrr, &dest);
		} else {
			draw_digits(highscoretable[plusmode], hiscoredigits,
				    ARRAYSIZE(hiscoredigits));
		}
	}

	if ((redraw & REDRAWTIMER) == REDRAWTIMER) {
		/* The time */
		draw_digits(gametime, timedigits, ARRAYSIZE(timedigits));
	}

	if ((redraw & REDRAWSCORE) == REDRAWSCORE) {
		/* The score */
		draw_digits(score, scoredigits, ARRAYSIZE(scoredigits));
	}


	if ((redraw & REDRAWPREVIEW) == REDRAWPREVIEW) {
		draw_preview();
	}
	if (redraw & REDRAWPIPE) {
		FOREACH_TILE(row, col)
			draw_tile(row, col, false);
	}

	if ((redraw & REDRAWHELP) == REDRAWHELP) {
		/* Show the help pages */

		/* Draw a black surround */
		dest.x = gameboard_rect.x - 1;
		dest.y = gameboard_rect.y - 1;
		dest.w = gameboard_rect.w + 2;
		dest.h = gameboard_rect.h + 2;
		SDL_SetRenderDrawColor(rrr, BLACK, 255);
		SDL_RenderDrawRect(rrr, &dest);
		/* Draw a white box to cover the game board */
		SDL_SetRenderDrawColor(rrr, WHITE, 255);
		SDL_RenderFillRect(rrr, &gameboard_rect);
		/* Draw the Exit text and the navigation buttons */
		if(helppage > 1) {
			/* Left arrow */
			src.x = 2 * tilew; src.y = 7 * tileh;
			src.w = tilew; src.h = tileh;
			blit(tiles, &src, &help_l_label);
		}
		if(helppage < ARRAYSIZE(helppages) - 1) {
			/* Right arrow */
			src.x = 4 * tilew; src.y = 10 * tileh;
			src.w = tilew; src.h = tileh;
			blit(tiles, &src, &help_r_label);
		}
		/* Exit */
		src.x = 3 * tilew; src.y = 7 * tileh;
		src.w = 2 * tilew; src.h = tileh;
		blit(tiles, &src, &help_exit_label);

		x = xres - (BOARDW - 0.2) * tilew;
		y = 0.2 * BOARDH; if (xres == 240 || xres == 480) y = 2.2 * tileh;
		draw_ascii(helppages[helppage], x, y);
		if (helppage == 7) {
			get_pipe_src(plusmode? 0 : 13, &src, plusmode);
			blit(tiles, &src, plusmode_rect);
		}
	}

	if (redraw != REDRAWNONE)
		SDL_RenderPresent(rrr);

	redraw = REDRAWNONE;
}

/***************************************************************************
 * Draw Ascii                                                              *
 ***************************************************************************/
/* This writes ASCII text. Embedded "\n" are translated into newlines.
   On entry: text = pointer to a (C) null terminated string */

static void draw_ascii(const char *const text, int xpos, int ypos)
{
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
		blit(ascii, &src, &dest);
		x = x + src.w;
		count++;
	}
}

static SDL_Rect *get_digit(int value, int place)
{
	while (place) {
		value /= 10;
		place--;
	}
	return &digit_src[value % 10];
}

/***************************************************************************
 * Draw Digits                                                             *
 ***************************************************************************/
/* This writes an integer value of not more than len digits.
 * A negative value can be no more than len - 1 digits long */

static void draw_digits(int value, SDL_Rect *label, int len) {
	if (value < 0) {
		blit(digits, &digit_src[10], &label[0]);
		value = abs(value);
		label++;
		len--;
	}

	for (int i = 0; i < len; ++i) {
		blit(digits, get_digit(value, len - 1 - i), &label[i]);
	}
}

static void set_pipe(struct gametile *tile, int pipe)
{
	tile->pipe = pipe;
	tile->flags &= ~(NORTH | EAST | SOUTH | WEST);
	tile->flags |= CHANGED | get_pipe_directions(pipe);
	redraw |= REDRAWPIPE;
}

/***************************************************************************
 * Initialise New Game                                                     *
 ***************************************************************************/
/* This sets up the variables necessary to start the game afresh. */

static void initialise_new_game(void)
{
	int start_row, start_col;

	if (plusmode) {
		settings = &plussettings;
		scoring = &plusscoring;
	} else {
		settings = &classicsettings;
		scoring = &classicscoring;
	}
	if (!fill_list)
		fill_list = tilering_init(32);

	redraw = REDRAWALL;
	score = 0;
	disablescoring = false;
	flashhighscorestate = false;
	gametime = settings->duration;

	/* Clear the game board array */
	FOREACH_TILE(row, col) {
		boardarray[row][col].flags = 0;
		boardarray[row][col].fill = 0;
		boardarray[row][col].row = row;
		boardarray[row][col].col = col;
		set_pipe(&boardarray[row][col], NULLPIPEVAL);
	}

	/* Setup and initialise preview pieces/array. */
	for (int count = 0; count < PREVIEWARRAYSIZE; count++) {
		previewarray[count] = getnextpipepiece();
	}

	/* Place end points and record in game board array. */
	set_pipe(&boardarray[rand() % BOARDH][0], PIPEEND);
	start_row = rand() % BOARDH;
	start_col = BOARDW - 1;
	start = &boardarray[start_row][start_col];
	set_pipe(start, PIPESTART);
	tilering_reset(fill_list);
	start_fill(start, EAST, fill_list);
}

/***************************************************************************
 * Get Next Pipe Piece                                                     *
 ***************************************************************************/
/* Get next piece from the pipe array, or if at end
   of array refill it and get piece from start. */

static int getnextpipepiece(void)
{
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

static void fillpipearray(void)
{
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

static int fillpipearraypieces(int pipepiece, int frequency, int nextpointer)
{
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
   into the tiles texture. */

static void get_pipe_src(int pipeid, SDL_Rect *rect, bool filled)
{
	if (pipeid > 16) {
		printf("%s: Invalid pipe: %i\n", __func__, pipeid);
		pipeid = 0; /* draw the clearly wrong starting pipe */
	}

	if (filled)
		pipeid += 17;

	rect->x = (pipeid % 5) * tilew;
	rect->y = (pipeid / 5) * tileh;
	rect->w = tilew;
	rect->h = tileh;
	return;
}

static void toggle_fullscreen(void)
{
	if (sdl_fullscreen & SDL_WINDOW_FULLSCREEN_DESKTOP)
		sdl_fullscreen = 0;
	else
		sdl_fullscreen = SDL_WINDOW_FULLSCREEN_DESKTOP;

	SDL_SetWindowFullscreen(win, sdl_fullscreen);
}

static void manage_window_event(const SDL_Event *event)
{
	switch(event->window.event) {
	case SDL_WINDOWEVENT_SHOWN:
	case SDL_WINDOWEVENT_EXPOSED:
	case SDL_WINDOWEVENT_RESTORED:
	case SDL_WINDOWEVENT_MOVED:
		SDL_RenderPresent(rrr);
		break;
	case SDL_WINDOWEVENT_HIDDEN:
	case SDL_WINDOWEVENT_MINIMIZED:
	case SDL_WINDOWEVENT_MAXIMIZED:
	case SDL_WINDOWEVENT_RESIZED:
		break;
	case SDL_WINDOWEVENT_SIZE_CHANGED:
		initialize_drawables(event->window.data1, event->window.data2);
		break;
	default:
		break;
	}
}

/***************************************************************************
 * Manage User Input                                                       *
 ***************************************************************************/
/* This manages all user input. */

static void manage_user_input(void)
{
	while(SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_KEYDOWN:
			switch(event.key.keysym.sym) {
				case SDLK_ESCAPE:	/* Cancel on the Zaurus */
					if (!helppage)
						game_mode = GAMEQUIT;
				case SDLK_LEFT:
				case SDLK_RIGHT:
					if(helppage)
						manage_help_input(event.key.keysym.sym);
					break;
				case SDLK_RETURN:
					if (!(KMOD_ALT & SDL_GetModState()))
						break;
				case SDLK_F11:
					toggle_fullscreen();
					break;
				default:
					break;
				}
				break;
			case SDL_QUIT:
				game_mode = GAMEQUIT;
				break;
			case SDL_MOUSEBUTTONDOWN:
				manage_mouse_input();
				break;
			case SDL_WINDOWEVENT:
				manage_window_event(&event);
			default:
				break;
		}
	}
}

static int mouse_event_in_rect(int mx, int my, SDL_Rect *rect)
{
	return (mx >= rect->x && mx < rect->x + rect->w &&
		my >= rect->y && my < rect->y + rect->h);
}

static int get_pipe_directions(int targettype)
{
	switch (targettype) {
	case 0 :
		return WEST;
	case 1 :
		return EAST;
	case 2 :
		return NORTH | WEST;
	case 3 :
		return SOUTH | WEST;
	case 4 :
		return EAST | SOUTH ;
	case 5 :
		return NORTH | EAST;
	case 6 :
		return NORTH | SOUTH | WEST;
	case 7 :
		return EAST | SOUTH | WEST;
	case 8 :
		return NORTH | EAST | SOUTH;
	case 9 :
		return NORTH | EAST | WEST;
	case 10 :
		return NORTH | EAST | SOUTH | WEST;
	case 11 :
		return EAST | WEST;
	case 12 :
		return NORTH | SOUTH;
	case 13 :
		return WEST;
	case 14 :
		return NORTH;
	case 15 :
		return EAST;
	case 16 :
		return SOUTH;
	default :
		return 0;
	}
}

static bool is_first_row(struct gametile *tile)
{
	return ((tile - boardarray[0]) < BOARDW);
}

static bool is_first_col(struct gametile *tile)
{
	return (((tile - boardarray[0]) % BOARDW) == 0);
}

static bool is_last_row(struct gametile *tile)
{
	return ((tile - boardarray[0]) >= (BOARDW * (BOARDH -1)));
}

static bool is_last_col(struct gametile *tile)
{
	return (((tile - boardarray[0]) % BOARDW) == (BOARDW - 1));
}

static struct gametile *get_north_neighbor(struct gametile *tile)
{
	if (is_first_row(tile) || !(tile->flags & NORTH))
	    return NULL;

	return tile - BOARDW;
}

static struct gametile *get_east_neighbor(struct gametile *tile)
{
	if (is_last_col(tile) || !(tile->flags & EAST))
	    return NULL;

	return tile + 1;
}

static struct gametile *get_south_neighbor(struct gametile *tile)
{
	if (is_last_row(tile) || !(tile->flags & SOUTH))
		return NULL;

	return tile + BOARDW;
}

static struct gametile *get_west_neighbor(struct gametile *tile)
{
	 if (is_first_col(tile) || !(tile->flags & WEST))
		 return NULL;

	 return tile - 1;
}


static void mark_neighbors(struct gametile *tile, int flags)
{
	struct gametile *neighbor;

	tile->flags |= flags;
	neighbor = get_north_neighbor(tile);
	if (neighbor && (neighbor->flags & SOUTH) && !(neighbor->flags & flags))
		mark_neighbors(neighbor, flags);

	neighbor = get_east_neighbor(tile);
	if (neighbor && (neighbor->flags & WEST) && !(neighbor->flags & flags))
		mark_neighbors(neighbor, flags);

	neighbor = get_south_neighbor(tile);
	if (neighbor && (neighbor->flags & NORTH) && !(neighbor->flags & flags))
		mark_neighbors(neighbor, flags);

	neighbor = get_west_neighbor(tile);
	if (neighbor && (neighbor->flags & EAST) && !(neighbor->flags & flags))
		mark_neighbors(neighbor, flags);
}


/**
 * return true if the tile has any open ends
 */
static bool is_neighbor_open(struct gametile *tile)
{
	struct gametile *neighbor;

	if (tile->flags & NORTH) {
		neighbor = get_north_neighbor(tile);
		if (!(neighbor && (neighbor->flags & SOUTH)))
			return true;
	}

	if (tile->flags & EAST) {
		neighbor = get_east_neighbor(tile);
		if (!(neighbor && (neighbor->flags & WEST)))
			return true;
	}

	if (tile->flags & SOUTH) {
		neighbor = get_south_neighbor(tile);
		if (!(neighbor && (neighbor->flags & NORTH)))
			return true;
	}

	if (tile->flags & WEST) {
		neighbor = get_west_neighbor(tile);
		if (!(neighbor && (neighbor->flags & EAST)))
			return true;
	}

	return false;
}

static void place_pipe(int row, int column)
{
	/* Place pipe piece from start of preview array. */
	if (boardarray[row][column].pipe != NULLPIPEVAL) {
		if (boardarray[row][column].fill)
			return;

		score = score + scoring->overwrite;
	} else {
		score = score + scoring->place;
	}

	set_pipe(&boardarray[row][column], previewarray[0]);
	/* Move all preview pieces down 1 place. */
	for (int count = 0; count < PREVIEWARRAYSIZE - 1; count++) {
		previewarray[count] = previewarray[count + 1];
	}
	/* Add a new preview piece at the end. */
	previewarray[PREVIEWARRAYSIZE - 1] = getnextpipepiece();
	/* Mark tile for drawing and redraw everything related */
	redraw |= REDRAWSCORE | REDRAWPREVIEW;
}

static void load_highscore(int index)
{
	/* Process High Score clicks */
	initialise_new_game();
	/* Copy the highscoreboard into the board array */
	FOREACH_TILE(row, col) {
		int pipe = highscoreboard[index][row * BOARDH + col];
		set_pipe(&boardarray[row][col], pipe);
		if (pipe == PIPESTART)
			start = &boardarray[row][col];
	}
	gametime = 0;
	disablescoring = true;	/* This is only used here to prevent the score from incrementing whilst filling. */
	tilering_reset(fill_list);
	start_fill(start, EAST, fill_list);
	game_mode = GAMEFINISH;
}

static void manage_mouse_input(void)
{
	int mbut, mx, my;

	mbut = SDL_GetMouseState(&mx, &my);
	mx = (mx - mouse_scale.x) / mouse_scale.w;
	my = (my - mouse_scale.y) / mouse_scale.h;

	if (mx < 0 || mx >= xres || my < 0 || my >= yres)
		return;

	if (mbut != SDL_BUTTON_LEFT)
		return;

	if (helppage) {
		if (mouse_event_in_rect(mx, my, &help_l_label)) {
			manage_help_input(SDLK_LEFT);
		} else if (mouse_event_in_rect(mx, my, &help_r_label)) {
			manage_help_input(SDLK_RIGHT);
		} else if (mouse_event_in_rect(mx, my, &help_exit_label)) {
			manage_help_input(SDLK_ESCAPE);
		} else if (mouse_event_in_rect(mx, my, plusmode_rect)) {
			plusmode = !plusmode;
			initialise_new_game();
			game_mode = GAMEOVER;
			redraw = REDRAWHELP | REDRAWHIGHSCORE | REDRAWSCORE;
		}
		return;
	}

	if (mouse_event_in_rect(mx, my, &gameboard_rect)) {
		if (game_mode == GAMEON) {
			/* Process game board clicks */
			/* Convert the mouse coordinates to offsets into the board array */
			int column = (mx - gameboard_rect.x) / tilew;
			int row = (my - gameboard_rect.y) / tileh;

			if (boardarray[row][column].pipe > PIPEEND)
				place_pipe(row, column);
			else if (boardarray[row][column].pipe == PIPESTART)
				game_mode = GAMEFINISH;
		}
	} else if (mouse_event_in_rect(mx, my, &fill_label)) {
		if (game_mode == GAMEON)
			game_mode = GAMEFINISH;
	} else if (mouse_event_in_rect(mx, my, &new_game_label)) {
		game_mode = GAMESTART;
	} else if (mouse_event_in_rect(mx, my, &hiscore_label)) {
		load_highscore(plusmode);
	} else if (mouse_event_in_rect(mx, my, &help_label)) {
		/* Process Help clicks */
		helppage = 1; /* enter help mode */
		redraw = redraw | REDRAWHELP;
	}
}

/***************************************************************************
 * Manage Help Input                                                       *
 ***************************************************************************/
/* This manages all user input relating to Help. */

static void manage_help_input(int input)
{
	switch(input) {
		case SDLK_ESCAPE:
			helppage = 0; /* Do not draw help */
			redraw = REDRAWALL;
			break;
		case SDLK_LEFT:
			if(helppage > 1) {
				helppage = helppage - 1;
				redraw = redraw | REDRAWHELP;
			}
			break;
		case SDLK_RIGHT:
			if(helppage < ARRAYSIZE(helppages) - 1) {
				helppage = helppage + 1;
				redraw = redraw | REDRAWHELP;
			}
			break;
		default:
			break;
	}
}

/***************************************************************************
 * Clear Dead Pipes                                                        *
 ***************************************************************************/
/* This clears one dead pipe at a time from the board array and the screen
   and is designed to be called from a timer so that it appears animated. */

static void cleardeadpipes(void)
{
	FOREACH_TILE(row, col) {
		struct gametile *tile = &boardarray[row][col];

		if ((tile->pipe != NULLPIPEVAL) &&
		    (!(tile->flags & CONNECTED))) {
			set_pipe(tile, NULLPIPEVAL);
			if (!disablescoring)
				score += scoring->unfilled;
			redraw |= REDRAWPIPE | REDRAWSCORE;
			return; /* return immedaitely */
		}
	}

	game_mode = GAMEFILLPIPES;
}

static void tilering_reset(struct tilering *ring)
{
	ring->write = ring->read = ring->start;
	while (ring->write < ring->end) {
		*ring->write = NULL;
		ring->write++;
	}
	ring->write = ring->start;
}

static struct tilering *tilering_init(int size)
{
	struct tilering *ring = malloc(sizeof(struct tilering));
	ring->start = malloc(size * sizeof(struct gametile *));
	ring->end = ring->start + size;
	tilering_reset(ring);
	return ring;
}

/**
 * return the next below capacity tile in the array.
 * If a capacity tile is encountered, remove it from the array
 * If no filling tile is below capacity, return NULL, at which
 * point the game should end
 */
static struct gametile *tilering_read(struct tilering *ring)
{
	struct gametile **tmp_read = ring->read + 1;
	while (tmp_read < ring->end) {
		if (*tmp_read!= NULL) {
			if ((*tmp_read)->fill >= settings->steps) {
				*tmp_read = NULL;
			} else {
			    ring->read = tmp_read;
			    return *tmp_read;
			}
		}
		tmp_read++;
	}
	tmp_read = ring->start;
	while (tmp_read <= ring->read) {
		if (*tmp_read != NULL) {
			if ((*tmp_read)->fill >= settings->steps) {
				*tmp_read = NULL;
			} else {
			    ring->read = tmp_read;
			    return *tmp_read;
			}
		}
		tmp_read++;
	}
	return NULL;
}

static void tilering_push(struct tilering *ring, struct gametile *tile)
{
	*ring->write = tile;
	ring->write++;
	if (ring->write >= ring->end)
		ring->write = ring->start;
}

static void start_fill(struct gametile *tile, int direction,
		       struct tilering *fill_list)
{
	tile->flags |= (direction << FILLDIRECTION);
	tilering_push(fill_list, tile);
}

static void start_filling_neighbors(struct gametile *tile,
				    struct tilering *fill_list)
{
	struct gametile *neighbor;

	neighbor = get_north_neighbor(tile);
	if (neighbor && (neighbor->flags & SOUTH))
		start_fill(neighbor, SOUTH, fill_list);

	neighbor = get_east_neighbor(tile);
	if (neighbor && (neighbor->flags & WEST))
		 start_fill(neighbor, WEST, fill_list);

	neighbor = get_south_neighbor(tile);
	if (neighbor && (neighbor->flags & NORTH))
		start_fill(neighbor, NORTH, fill_list);

	neighbor = get_west_neighbor(tile);
	if (neighbor && (neighbor->flags & EAST))
		start_fill(neighbor, EAST, fill_list);
}

static bool fill_pipe(struct gametile *tile, struct tilering *fill_list)
{
	tile->flags |= CHANGED;
	redraw |= REDRAWPIPE | REDRAWSCORE;
	tile->fill++;

	if (tile->fill >= settings->steps) {
		if (!disablescoring) {
			if (tile->pipe == PIPEEND)
				score += scoring->fillend;
			else
				score += scoring->fill;
		}

		if (is_neighbor_open(tile))
			return true;

		start_filling_neighbors(tile, fill_list);
	}

	return false;
}

/***************************************************************************
 * Fill Pipes                                                              *
 ***************************************************************************/
/* This fills one or several pipes at a time. */

static bool fillpipes(struct tilering *fill_list)
{
	struct gametile *current = tilering_read(fill_list);
	if (!current)
		return true;
	else
		return fill_pipe(current, fill_list);
}

static void check_highscore()
{
	/* Ok, last bit: high score, again ignoring whilst displaying the highscoreboard */
	if (!disablescoring && score > highscoretable[plusmode]) {
		highscoretable[plusmode] = score;
		/* Copy the board into the highscoreboard */
		FOREACH_TILE(row, col)
			highscoreboard[plusmode][row * BOARDH + col] =
				boardarray[row][col].pipe;

		redraw = redraw | REDRAWHIGHSCORE;
		game_mode = GAMEFLASHHIGHSCORE;
		save_rc_file(); /* This saves the new highscore[s] */
	}
}

/***************************************************************************
 * Read Resource File                                                      *
 ***************************************************************************/

static int read_score(FILE* file)
{
	int index, index1, result, value;

	result = fscanf(file," %*[[h]ighscore%d%*[]:] %d "
			"%*[[h]ighscoreboard%d%*[]:]", &index, &value, &index1);
	if (result != 3 || index != index1 || index >= ARRAYSIZE(highscoretable))
		return -1;

	highscoretable[index] = value;
	for (int count = 0; count < BOARDH * BOARDW; count++) {
		result = fscanf(file,"%i[\n ,]", &value);	/* pipe piece id */
		if (result == 1)
			highscoreboard[index][count] = value;
		else {
			printf("count %d result: %d\n", count, result);
			return -1;
		}
	}

	return 0;
}

static void read_rc_file(void)
{
	char buffer[256];
	FILE* file;
	int result;

	strcpy(buffer, user_home_dir);
	strcat(buffer, "/");
	strcat(buffer, RESOURCEFILE);
	if ((file = fopen(buffer,"r")) == NULL) {
		printf("%s: Cannot read from file %s\n", __func__, buffer);
		return;
	}

	for (int i = 0; i < ARRAYSIZE(highscoretable); i++) {
		result = read_score(file);
		if (result)
			break;
	}

	fclose(file);
}

/***************************************************************************
 * Save Resource File                                                      *
 ***************************************************************************/

static void write_score(FILE *file, int index)
{
	fprintf(file,"[highscore%d]\n%i\n[highscoreboard%d]\n",
		index, highscoretable[index], index);

	for (int i = 0; i < BOARDH * BOARDW; i++) {
		fprintf(file, "%3i%c", highscoreboard[index][i],
			(i % BOARDH == 9)? '\n': ' ');
	}
}

static void save_rc_file(void)
{
	char buffer[256];
	FILE* file;

	strcpy(buffer, user_home_dir);
	strcat(buffer, "/");
	strcat(buffer, RESOURCEFILE);

	if ((file = fopen(buffer,"w")) == NULL) {
		printf("%s: Cannot write to file %s\n", __func__, buffer);
		return;
	}

	for (int i = 0; i < ARRAYSIZE(highscoretable); i++) {
		if (highscoretable[i])
			write_score(file, i);
	}

	fclose(file);
}
