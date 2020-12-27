#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include <stdlib.h>
#include<string.h>
#include<time.h>
extern "C" {

#include"../szablon2/SDL2-2.0.10/include/SDL.h"
#include"../szablon2/SDL2-2.0.10/include/SDL_main.h"

}

#define SCREEN_WIDTH	1750
#define SCREEN_HEIGHT	900
#define START_POSITION 120
#define MAX_BLOCKS_NUMBER 100
#define BACKGROUND_SPEED 4
#define FIELD_AVAILABLE 2.3
#define SAVE_POSITION 200
#define JUMP_SPEED_X 0
#define JUMP_SPEED_Y 8
#define delta_distance 10
#define dash_k  5
#define MACHINE_CONTROL 0
#define PLAYER_CONTROL 1
#define G 0.09
#define START_Y_POSITION 400
#define DELTA_K 200
#define TIME_DASH 100
struct coordinates_t {
	double x = 0;
	double y = 0;
};

struct sizes_t {
	int width = 0;
	int height = 0;
};
struct game_object {
	coordinates_t position;
	coordinates_t alt_position;
	coordinates_t speed;
	sizes_t sizes;
	SDL_Surface* graphics;
};
struct blocks_t {
	game_object blocks_arr[MAX_BLOCKS_NUMBER];
	int length = 0;
};
// narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj¹ca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
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
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};


// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};


// rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1) 
// b¹dŸ poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};


// rysowanie prostok¹ta o d³ugoœci boków l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
	Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};

int jump(game_object* player, int* is_jump, float delta) {
	player->speed.y -= G * DELTA_K * delta;
	if (player->position.y > SCREEN_HEIGHT - player->sizes.height / 2) {
		player->position.y = SCREEN_HEIGHT - player->sizes.height / 2;
		*is_jump = 0;
		player->speed.y = 0;
		player->speed.x = 0;
		return 0;
	}
	else  return 1;
}

int check_collision(game_object object1, game_object object2, int save_distance_x, int save_distance_y) {
	if (
		(object1.position.x + object1.sizes.width / 2 + save_distance_x) - (object2.position.x - object2.sizes.width / 2) >= 0
		&& !((object1.position.x - object1.sizes.width / 2) - (object2.position.x + object2.sizes.width / 2) >= 0)
		&& (object1.position.y + object1.sizes.height / 2 + save_distance_y) - (object2.position.y - object1.sizes.height / 2) >= 0
		&& !((object1.position.y - object1.sizes.height / 2) - (object2.position.y + object2.sizes.height / 2) >= 0)

		)
	{
		return 1;

	}
	else {
		return 0;
	}
}
int check_platform(game_object object1, game_object object2, int save_distance_x) {
	if (
		(object1.position.x + object1.sizes.width / 2 + save_distance_x) - (object2.position.x - object2.sizes.width / 2) >= 0
		&& !((object1.position.x - object1.sizes.width / 2) - (object2.position.x + object2.sizes.width / 2) >= 0)
		&& (object1.position.y + object1.sizes.height / 2) - (object2.position.y - object1.sizes.height / 2) >= 0
		&& !((object1.position.y - object1.sizes.height / 2) - (object2.position.y + object2.sizes.height / 2) >= 0)


		)
	{
		if ((object2.position.y + object2.sizes.height / 2) - (object1.position.y + object1.sizes.height / 2) >= 0
			&&(object2.position.x - object2.sizes.width / 2) - (object1.position.x - object1.sizes.width / 2) < object1.sizes.width / 2
			) {
			return 1;
		}
		else {
			return -1;
		}
	}
	else {

		return 0;
	}
}
void check_machine_jump(int game_mode, game_object* player, int* is_jumping, int* is_jump, int* is_double_jump, blocks_t* blocks, float delta) {
	if (game_mode == MACHINE_CONTROL && *is_double_jump == 0) {
		for (int i = 0; i < blocks->length; i++)
		{

			if (check_collision(*player, blocks->blocks_arr[i], SAVE_POSITION, SAVE_POSITION)) {
				player->position.y -= delta_distance;
				player->speed.x = JUMP_SPEED_X;
				//player->speed.y = JUMP_SPEED_Y;
				*is_jump = 1;
				*is_jumping = 1;
				break;
				//block_index = i;
			}
		}
		// jumping 
		if (*is_jump) {
			*is_jumping = jump(player, is_jump, delta);
		}
	}
}
// main
#ifdef __cplusplus
extern "C"
#endif


void move_gameobject(game_object * object, double delta) {
	object->position.x += object->speed.x * delta * DELTA_K;
	object->position.y -= object->speed.y * delta * DELTA_K;
}

void draw_gameobject(SDL_Surface* screen, game_object object) {
	DrawSurface(screen, object.graphics,
		object.position.x,
		object.position.y);
}
void block_movement(blocks_t* blocks, SDL_Surface* screen, float a, double delta) {
	//block movement
	for (int i = 0; i < blocks->length; i++)
	{
		blocks->blocks_arr[i].speed.x -= a;

		move_gameobject(&blocks->blocks_arr[i], delta);
		draw_gameobject(screen, blocks->blocks_arr[i]);
	}
}
void free_and_destroy(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer) {
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}
void do_double_jump(game_object* player, int* is_jumping, int* is_double_jump, int* is_jump) {
	if (*is_jumping) {
		*is_double_jump = 1;
	}
	*is_jump = 1;
	*is_jumping = 1;
	player->speed.x = JUMP_SPEED_X;
	player->speed.y = JUMP_SPEED_Y;
}

int  get_back_color(int game_mode, SDL_Surface* screen) {
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	if (game_mode == MACHINE_CONTROL) {
		return  niebieski;
	}
	else {
		return czerwony;
	}
}
void check_dashing(int* is_dash, int time_dash, game_object* background, blocks_t* blocks, blocks_t* platforms) {
	if (*is_dash) {
		int time_dash1 = SDL_GetTicks();
		if (time_dash1 - time_dash > TIME_DASH) {
			background->speed.x /= dash_k;
			*is_dash = 0;
			for (int i = 0; i < blocks->length; i++)
			{
				blocks->blocks_arr[i].speed.x /= dash_k;
			}
			for (int i = 0; i < platforms->length; i++)
			{
				platforms->blocks_arr[i].speed.x /= dash_k;
			}
		}
	}
}

int main(int argc, char** argv) {
	srand(time(0));
	int games = 1;
	while (games > 0) {

		int t1, t2, quit, frames, rc;
		double delta, worldTime, fpsTimer, fps, distance, backSpeed;
		SDL_Event event;
		SDL_Surface* screen, * charset;
		SDL_Surface* background_spr;
		SDL_Surface* player_sprite;
		SDL_Surface* block_sprite;
		SDL_Surface* stalactite_sprite;
		
		SDL_Surface* platform_sprite;
		SDL_Texture* scrtex;
		SDL_Window* window;
		SDL_Renderer* renderer;

		int game_mode = 1; // default
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
			free_and_destroy(screen, scrtex, window, renderer);
			return 1;
		};
		SDL_SetColorKey(charset, true, 0x000000);

		background_spr = SDL_LoadBMP("../szablon2/background.bmp");
		if (background_spr == NULL) {
			printf("SDL_LoadBMP(background.bmp) error: %s\n", SDL_GetError());
			free_and_destroy(screen, scrtex, window, renderer);
			return 1;
		};

		player_sprite = SDL_LoadBMP("../szablon2/player.bmp");
		if (player_sprite == NULL) {
			printf("SDL_LoadBMP(player.bmp) error: %s\n", SDL_GetError());
			free_and_destroy(screen, scrtex, window, renderer);
			return 1;
		};

		block_sprite = SDL_LoadBMP("../szablon2/block.bmp");
		if (block_sprite == NULL) {
			printf("SDL_LoadBMP(block.bmp) error: %s\n", SDL_GetError());
			free_and_destroy(screen, scrtex, window, renderer);
			return 1;
		};
		platform_sprite = SDL_LoadBMP("../szablon2/platform.bmp");
		if (platform_sprite == NULL) {
			printf("SDL_LoadBMP(platform.bmp) error: %s\n", SDL_GetError());
			free_and_destroy(screen, scrtex, window, renderer);
			return 1;
		};
		stalactite_sprite = SDL_LoadBMP("../szablon2/stalactite.bmp");
		if (stalactite_sprite == NULL) {
			printf("SDL_LoadBMP(stalactite.bmp) error: %s\n", SDL_GetError());
			free_and_destroy(screen, scrtex, window, renderer);
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
		backSpeed = 1;
		// player 
		game_object player;
		player.graphics = player_sprite;
		player.speed.x = 0;
		player.speed.y = 0;
		player.sizes.width = 100;
		player.sizes.height = 100;
		player.position.x = player.sizes.width / 2;
		player.position.y = START_Y_POSITION - player.sizes.height;//SCREEN_HEIGHT - player.sizes.height / 2 ;

		game_object background;
		background.graphics = background_spr;
		background.speed.x = -(BACKGROUND_SPEED);
		background.speed.y = 0;
		background.sizes.width = 5 * SCREEN_WIDTH; // 5*SCREEN_WIDTH
		background.sizes.height = 2 * SCREEN_HEIGHT;
		background.position.x = SCREEN_WIDTH * 2.5,
		background.position.y = SCREEN_HEIGHT / 2;

		// genrate blocks and platforms
		blocks_t blocks;

		blocks_t platforms;

		for (int i = 0; i < MAX_BLOCKS_NUMBER / 2; i++)
		{
			game_object platform;
			platform.graphics = platform_sprite;
			platform.sizes.width = 800;
			platform.sizes.height = 100;
			platform.speed.x = -(BACKGROUND_SPEED);
			platform.speed.y = 0;
			
			platform.position.x = i * SCREEN_WIDTH + platform.sizes.width / 2;

			int random_height = (rand() % SCREEN_HEIGHT) / 2 - player.sizes.height;

			platform.position.y = SCREEN_HEIGHT - platform.sizes.height - random_height;
			if (i == 0) {
				platform.position.y = START_Y_POSITION + platform.sizes.height;
			}

			platforms.blocks_arr[platforms.length] = platform;
			platforms.length++;
			//higher platform
			if ((i + 4) % 7 == 0) {
				game_object platform2;
				platform2.graphics = platform_sprite;
				platform2.sizes.width = 800;
				platform2.sizes.height = 100;
				platform2.speed.x = -(BACKGROUND_SPEED);
				platform2.speed.y = 0;

				platform2.position.x = platform.position.x + platform.sizes.width/3;
				platform2.position.y = platform.position.y -6*player.sizes.height;
				platforms.blocks_arr[platforms.length] = platform2;
				platforms.length++;
			}
			//stalaktit
			else if ((i + 4) % 3 == 0) {
				game_object platform2;
				platform2.graphics = stalactite_sprite;
				platform2.sizes.width = 400;
				platform2.sizes.height = 100;
				platform2.speed.x = -(BACKGROUND_SPEED);
				platform2.speed.y = 0;
				platform2.position.x = platform.position.x + platform2.sizes.width/2;
				platform2.position.y = platform.position.y - platform.sizes.height/2 ;
				platforms.blocks_arr[platforms.length] = platform2;
				platforms.length++;
			}
			else {
				game_object block;
				block.graphics = block_sprite;
				block.sizes.width = 100;
				block.sizes.height = 100;
				block.speed.x = -(BACKGROUND_SPEED);
				block.speed.y = 0;
				block.position.x = 7 * block.sizes.width + i * SCREEN_WIDTH;
				block.position.y = platform.position.y - block.sizes.height;
				if (i == 0) {
					block.position.y = -100000;
				}
				blocks.blocks_arr[blocks.length] = block;
				blocks.length++;
			}
			
			

		}
		// 




		int is_jump = 0;
		int is_jumping = 0;
		int is_double_jump = 0;
		int is_dash = 0;
		int is_on_platform = 0;

		int time_dash = 400;

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
			if (distance + backSpeed * delta > 0) {
				distance += backSpeed * delta;


			}


			SDL_FillRect(screen, NULL, zielony);
			/*

			// drawing background
			if (distance > BACKGROUND_LENGTH) {
				distance = 0;
			}
			*/
			// background movement

			if (background.position.x < -background.sizes.width / 3.5) {
				background.position.x = SCREEN_WIDTH * 2.5;
			}
			float a = 0.001 * delta * DELTA_K;// acceleration
			//background.position.y = player.position.y ;
			//background.speed.x -= a;
			background.speed.y = -player.speed.y / 6;

			move_gameobject(&background, delta);
			draw_gameobject(screen, background);
			if (!is_dash) {
				move_gameobject(&player, delta);
			}

			// drawing player
			draw_gameobject(screen, player);
			//block movement
			block_movement(&blocks, screen, a, delta);
			block_movement(&platforms, screen, a, delta);
			check_machine_jump(game_mode, &player, &is_jumping, &is_jump, &is_double_jump, &blocks, delta);
			if (game_mode == PLAYER_CONTROL && is_jump) {
				is_jumping = jump(&player, &is_jump, delta);
				if (!is_jumping) {
					is_double_jump = 0;
				}
			}
			for (int i = 0; i < blocks.length; i++)
			{
				//check collision
				if (check_collision(player, blocks.blocks_arr[i], 0, 0)) {
					quit = 1;
					games++;
				}

			}
			// check is_dashing
			check_dashing(&is_dash, time_dash, &background, &blocks, &platforms);
			int flag = 0;
			for (int i = 0; i < MAX_BLOCKS_NUMBER; i++)
			{
				game_object block = platforms.blocks_arr[i];
				if (check_platform(player, block, 0) == 1) {
					flag = 1;
					is_on_platform = 1;
					is_jump = 0;
					is_jumping = 0;
					is_double_jump = 0;
					player.position.y = block.position.y - block.sizes.height;
					player.speed.y = 0;
					player.speed.x = 0;

				}
				else if (check_platform(player, block, 0) == -1) {
					quit = 1;
					games++;
				}
			}
			if (flag == 0 && !is_jumping) {
				if (player.position.y + player.sizes.height / 2 < SCREEN_HEIGHT) {
					player.speed.y -= 0.09;
				}
				else {
					player.speed.y = 0;
					player.position.y = SCREEN_HEIGHT - player.sizes.height / 2;
				}

			}

			fpsTimer += delta;
			if (fpsTimer > 0.5) {
				fps = frames * 2;
				frames = 0;
				fpsTimer -= 0.5;
			};

			int color = get_back_color(game_mode, screen);
			//czerwony;
		// tekst informacyjny / info text

			DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, color);
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
						games--;
					}
					else if (event.key.keysym.sym == SDLK_RIGHT) backSpeed = 3.0;
					else if (event.key.keysym.sym == SDLK_LEFT) backSpeed = -3.0;
					else if (event.key.keysym.sym == 'n')
					{
						printf("n -pressed\n");
						quit = 1;
						games++;
					}
					else if (event.key.keysym.sym == 'd')
					{
						printf("d -pressed\n");
						if (game_mode == MACHINE_CONTROL)
							game_mode = PLAYER_CONTROL;
						else
							game_mode = MACHINE_CONTROL;
					}
					else if (event.key.keysym.sym == 'z' && game_mode == PLAYER_CONTROL)
					{
						if (!is_double_jump) {
							// do double jump
							do_double_jump(&player, &is_jumping, &is_double_jump, &is_jump);
						}
					}
					else if (event.key.keysym.sym == 'x' && game_mode == PLAYER_CONTROL)
					{

						if (!is_dash) {
							time_dash = SDL_GetTicks();
							background.speed.x *= dash_k;
							is_dash = 1;
							for (int i = 0; i < blocks.length; i++)
							{
								blocks.blocks_arr[i].speed.x *= dash_k;
							}
							for (int i = 0; i < platforms.length; i++)
							{
								platforms.blocks_arr[i].speed.x *= dash_k;
							}
						}


					}
					break;
				case SDL_KEYUP:
					backSpeed = 1; // bylo 1.0 
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
