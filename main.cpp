#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {

#include"../szablon2/SDL2-2.0.10/include/SDL.h"
#include"../szablon2/SDL2-2.0.10/include/SDL_main.h"

}

#define SCREEN_WIDTH	1750
#define SCREEN_HEIGHT	700
#define START_POSITION 120
#define MAX_BLOCKS_NUMBER 100
#define BACKGROUND_LENGTH 8
#define FIELD_AVAILABLE 2.3
#define SAVE_POSITION 400
#define JUMP_SPEED_Y 10
struct coordinates_t{
	double x=0;
	double y=0;
};

struct sizes_t {
	int width = 0;
	int height = 0;
};
struct game_object{
	coordinates_t position;
	coordinates_t alt_position;
	coordinates_t speed;
	sizes_t sizes;
	SDL_Surface* graphics;
	int is_jump = 0;

};
struct blocks {
	game_object blocks_arr[MAX_BLOCKS_NUMBER];
	int length = 0;
};
// narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj¹ca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt œrodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};


// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};


// rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1) 
// b¹dŸ poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


// rysowanie prostok¹ta o d³ugoœci boków l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};


// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {

	int games = 1;
	int game_mode = 0; // default
	while (games > 0) {

		int t1, t2, quit, frames, rc;
		double delta, worldTime, fpsTimer, fps, distance, etiSpeed;
		SDL_Event event;
		SDL_Surface* screen, * charset;
		SDL_Surface* background;
		SDL_Surface* player_sprite;
		SDL_Surface* block_sprite;
		SDL_Texture* scrtex;
		SDL_Window* window;
		SDL_Renderer* renderer;

		// okno konsoli nie jest widoczne, jezeli chcemy zobaczyc
		// komunikaty wypisywane printf-em trzeba w opcjach:
		// project -> szablon2 properties -> Linker -> System -> Subsystem
		// zmienic na "Console"
		// console window is not visible, to see the printf output
		// the option:
		// project -> szablon2 properties -> Linker -> System -> Subsystem
		// must be changed to "Console"
		printf("wyjscie printfa trafia do tego okienka\n");
		printf("printf output goes here\n");

		if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
			printf("SDL_Init error: %s\n", SDL_GetError());
			return 1;
		}

		// tryb pe³noekranowy / fullscreen mode
	//	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
	//	                                 &window, &renderer);
		rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
			&window, &renderer);
		if (rc != 0) {
			SDL_Quit();
			printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
			return 1;
		};

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		SDL_SetWindowTitle(window, "Szablon do zdania drugiego 2017");


		screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
			0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

		scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			SCREEN_WIDTH, SCREEN_HEIGHT);


		// wylaczenie widocznosci kursora myszy
		SDL_ShowCursor(SDL_DISABLE);

		// wczytanie obrazka cs8x8.bmp
		charset = SDL_LoadBMP("../szablon2/cs8x8.bmp");
		if (charset == NULL) {
			printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
			SDL_FreeSurface(screen);
			SDL_DestroyTexture(scrtex);
			SDL_DestroyWindow(window);
			SDL_DestroyRenderer(renderer);
			SDL_Quit();
			return 1;
		};
		SDL_SetColorKey(charset, true, 0x000000);

		background = SDL_LoadBMP("../szablon2/background.bmp");
		if (background == NULL) {
			printf("SDL_LoadBMP(background.bmp) error: %s\n", SDL_GetError());
			SDL_FreeSurface(charset);
			SDL_FreeSurface(screen);
			SDL_DestroyTexture(scrtex);
			SDL_DestroyWindow(window);
			SDL_DestroyRenderer(renderer);
			SDL_Quit();
			return 1;
		};

		player_sprite = SDL_LoadBMP("../szablon2/player.bmp");
		if (player_sprite == NULL) {
			printf("SDL_LoadBMP(player.bmp) error: %s\n", SDL_GetError());
			SDL_FreeSurface(charset);
			SDL_FreeSurface(screen);
			SDL_DestroyTexture(scrtex);
			SDL_DestroyWindow(window);
			SDL_DestroyRenderer(renderer);
			SDL_Quit();
			return 1;
		};

		block_sprite = SDL_LoadBMP("../szablon2/block.bmp");
		if (block_sprite == NULL) {
			printf("SDL_LoadBMP(block.bmp) error: %s\n", SDL_GetError());
			SDL_FreeSurface(charset);
			SDL_FreeSurface(screen);
			SDL_DestroyTexture(scrtex);
			SDL_DestroyWindow(window);
			SDL_DestroyRenderer(renderer);
			SDL_Quit();
			return 1;
		};
		char text[128];
		int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
		int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
		int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
		int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

		t1 = SDL_GetTicks();

		frames = 0;
		fpsTimer = 0;
		fps = 0;
		quit = 0;
		worldTime = 0;
		distance = 0;
		etiSpeed = 1;
		// player 
		game_object player;
		player.graphics = player_sprite;
		player.speed.x = 0;
		player.speed.y = 0;
		player.sizes.width = 200;
		player.sizes.height = 200;
		player.position.x = player.sizes.width / 2,
		player.position.y = SCREEN_HEIGHT - player.sizes.height / 2;
		

		blocks blocks;

		for (int i = 0; i < MAX_BLOCKS_NUMBER; i++)
		{
			game_object block;
			block.graphics = block_sprite;
			block.sizes.width = 100;
			block.sizes.height = 100;
			block.speed.x = 3;
			block.speed.y = 0;
			block.position.x = SCREEN_WIDTH * 2 + block.sizes.width + i * SCREEN_WIDTH/2;
			block.position.y = SCREEN_HEIGHT - block.sizes.height;

			blocks.blocks_arr[i] = block;
			blocks.length++;
		}

		

		
		int is_jumping = 0;
		int block_index = -1;
		while (!quit) {
			t2 = SDL_GetTicks();

			// w tym momencie t2-t1 to czas w milisekundach,
			// jaki uplyna³ od ostatniego narysowania ekranu
			// delta to ten sam czas w sekundach
			// here t2-t1 is the time in milliseconds since
			// the last screen was drawn
			// delta is the same time in seconds

			delta = (t2 - t1) * 0.001;
			t1 = t2;
			worldTime += delta;
			if (distance + etiSpeed * delta > 0) {
				distance += etiSpeed * delta;
				
			//	printf("%f\n",float(block1.position.x));
			//	printf("%f\n",float(block1.position.x - player.position.x));
			//	printf("%f\n",float(playe1));
			}
			

			SDL_FillRect(screen, NULL, zielony);
			
			// drawing background
			if (distance > BACKGROUND_LENGTH) {
				distance = 0;
			}
			DrawSurface(screen, background,
				SCREEN_WIDTH * 2 - distance * SCREEN_HEIGHT,
				SCREEN_HEIGHT / 2);


			// player movement
			player.position.x += player.speed.x;
			player.position.y -= player.speed.y;



			// drawing player
			DrawSurface(screen, player.graphics,
				
				player.position.x,          // + sin(distance) * SCREEN_HEIGHT / 3,
				player.position.y);        //+ cos(distance) * SCREEN_HEIGHT / 3);


			//block1 movement
			for (int i = 0; i < blocks.length; i++)
			{
				blocks.blocks_arr[i].position.x -= blocks.blocks_arr[i].speed.x;
				blocks.blocks_arr[i].position.y += blocks.blocks_arr[i].speed.y;
			}
			
			for (int i = 0; i < blocks.length; i++)
			{
				DrawSurface(screen, blocks.blocks_arr[i].graphics,
					blocks.blocks_arr[i].position.x,
					blocks.blocks_arr[i].position.y);
			}
			// check jump
			

			for (int i = 0; i < blocks.length; i++)
			{	
				double difference = blocks.blocks_arr[i].position.x - player.position.x;
				if (difference <= SAVE_POSITION && difference > 0 && is_jumping == 0 && game_mode == 0) {
					player.is_jump = 1;
					player.speed.y = JUMP_SPEED_Y;
					is_jumping = 1;
					block_index = i;
				}
			}
			// jumping 
			if (player.is_jump) {
				player.speed.y -= 0.09;
				if (player.position.y > SCREEN_HEIGHT - blocks.blocks_arr[block_index].sizes.height) {
					player.position.y = SCREEN_HEIGHT - blocks.blocks_arr[block_index].sizes.height;
					player.is_jump = 0;
					player.speed.y = 0;
					player.speed.x = 0;
					
					is_jumping = 0;
				}

			}





			
			fpsTimer += delta;
			if (fpsTimer > 0.5) {
				fps = frames * 2;
				frames = 0;
				fpsTimer -= 0.5;
			};

			// tekst informacyjny / info text
			DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
			//            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
			sprintf(text, "UNICORN ATTACK, czas trwania = %.1lf s  %.0lf klatek / s", worldTime, fps);
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
			//	      "Esc - exit, \030 - faster, \031 - slower"
			sprintf(text, "Esc - wyjscie, \030 - przyspieszenie, \031 - zwolnienie");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

			SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
			//		SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, scrtex, NULL, NULL);
			SDL_RenderPresent(renderer);

			// obs³uga zdarzeñ (o ile jakieœ zasz³y) / handling of events (if there were any)
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) {
						quit = 1;
						games --;
					}
					else if (event.key.keysym.sym == SDLK_RIGHT) etiSpeed = 3.0;
					else if (event.key.keysym.sym == SDLK_LEFT) etiSpeed = -3.0;
					else if (event.key.keysym.sym == 'n')
					{
						printf("n -pressed\n");
						quit = 1;
						games++;
					}
					else if (event.key.keysym.sym == 'd')
					{
						printf("d -pressed\n");
						if (game_mode == 0)
							game_mode = 1;
						else
							game_mode = 0;
					}
					break;
				case SDL_KEYUP:
					etiSpeed = 1; // bylo 1.0 
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};
			frames++;
		};

		// zwolnienie powierzchni / freeing all surfaces
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

		SDL_Quit();

		games--;
	}
	return 0;
	};
