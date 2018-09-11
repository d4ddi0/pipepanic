/* Ooze pixel baes slow space filler experiment
Copyright (C) 2018 Joshua Clayton <stillcompiling@gmail.com>
*/

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#ifdef __cplusplus
extern "C"
#endif

#define GRAY(n) ((n) + ((n) << 8) + ((n) << 16) + ((n) << 24))
void ooze(uint32_t buf[480][640])
{
	static int lastcount = 0;
	int count = 0;
	int random_oozer = (lastcount)? rand() % lastcount : 0;
	for (int row = 0; row < 480; ++row) {
		for (int col = 0; col < 640; ++col) {
			int color = buf[row][col] % 256;
			if (color < 255 && color > 0) {
			    if (++count == random_oozer) {
				    buf[row][col] = 0;
			    } else {
				    buf[row][col] = GRAY(color - 1);
			    }
			}
		}
	}
	for (int row = 1; row < 479; ++row) {
		for (int col = 1; col < 639; ++col) {
			int color = buf[row][col] % 256;
			if (!color) {
				if (buf[row - 1][col])
					buf[row - 1][col]--;
				if (buf[row][col + 1])
					buf[row][col + 1]--;
				if (buf[row + 1][col])
					buf[row + 1][col]--;
				if (buf[row][col - 1])
					buf[row][col - 1]--;
			}
		}
	}
	lastcount = count;
}

int main(int argc, char *argv[])
{
	if(SDL_Init(SDL_INIT_VIDEO)) {
		printf("Cannot initialise SDL: %s\n", SDL_GetError());
		return 1;
	}

	srand(time(0)); /* initialize the random number generator */

	atexit(SDL_Quit);

	SDL_Window *win = SDL_CreateWindow("Ooze", SDL_WINDOWPOS_UNDEFINED,
					   SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
	SDL_SetWindowMinimumSize(win, 320, 240);
	SDL_Renderer *rrr = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
	if(win == NULL) {
		printf("Cannot initialise screen: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Texture *tex = SDL_CreateTexture(rrr, SDL_PIXELFORMAT_RGBA8888,
				SDL_TEXTUREACCESS_STATIC, 640, 480);

	uint32_t bigbuf[480][640];
	memset(bigbuf, 255, sizeof(bigbuf));
	bool quit = false;
	bool draw = false;

	while(!quit) {
		SDL_Event event;

		SDL_UpdateTexture(tex, NULL, bigbuf, 640 *
				  sizeof(uint32_t));
		SDL_WaitEvent(&event);
		SDL_PollEvent(&event);
		switch(event.type) {
		case SDL_KEYDOWN:
			switch(event.key.keysym.sym) {
			case SDLK_ESCAPE:
				quit = true;
				break;
			case SDLK_c:
				memset(bigbuf, 255, sizeof(bigbuf));
				printf("cleared\n");
				break;
			}
			break;
		case SDL_QUIT:
			quit= true;
			break;
		case SDL_MOUSEBUTTONUP:
			draw = false;
			break;
		case SDL_MOUSEBUTTONDOWN:
			printf("clicked %d, %d\n", event.motion.x,
			       event.motion.y);
			draw = true;
			/*no break */
		case SDL_MOUSEMOTION:
			if (draw)
				bigbuf[event.motion.y][event.motion.x] = 0;
		case SDL_WINDOWEVENT:
		default:
			break;
		}
		ooze(bigbuf);
		SDL_RenderClear(rrr);
		SDL_RenderCopy(rrr, tex, NULL, NULL);
		SDL_RenderPresent(rrr);
	}

	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(rrr);
	SDL_Quit();
	return 0;
}
