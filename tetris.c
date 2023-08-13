/**
 * tetris.c - tetris source code
 * author: Isaac Dowagiac
 *
 * This source code is absolute crap. I *know* it's crap. I had no
 * intentions, whatsoever, of writing good code. I just wanted to
 * speed blitz this mf to see how quickly I could do it.
 *
 * Pretty much all of the mechanics were done in 5 hours. About one
 * more hour was spent getting sounds, music, and sprites into the
 * game so that it looked better.
 **/

#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#define CELL_SIZE        32
#define CELLS_PER_ROW    10
#define CELLS_PER_COLUMN 20

#define MARGIN 4

#define WINDOW_WIDTH  (2 * MARGIN + CELLS_PER_ROW * CELL_SIZE)
#define WINDOW_HEIGHT (2 * MARGIN + CELLS_PER_COLUMN * CELL_SIZE)

struct color
{
    unsigned char r,
                  g, 
                  b;
};

struct piece
{
    int type;
    int color;
    int x,
        y;
    int angle;   // 0 - 3
};

enum
{
    PIECE_NONE,
    PIECE_LONG,
    PIECE_CUBE,
    PIECE_L1,
    PIECE_L2,
    PIECE_S1,
    PIECE_S2,
    PIECE_T,
    PIECE_COUNT = 7,
};

static TTF_Font *font = NULL;
static SDL_Texture *sprite_atlas = NULL;

struct piece current_piece = {0};

const struct color cell_color[] =
{
    {  60,  60,  60 },  // empty cell
    { 180,   0,   0 },  // red cell
    {   0, 180,   0 },  // green cell
    {   0,   0, 180 },  // blue cell
    { 180, 180,   0 },  // yellow cell
    { 180,   0, 180 },  // purple cell
};

char cell[CELLS_PER_ROW * CELLS_PER_COLUMN];

char piece_cell[PIECE_COUNT + 1][4][4*4] =
{
    [PIECE_NONE] =
    {
        [0] = {0},
        [1] = {0},
        [2] = {0},
        [3] = {0},
    },
    [PIECE_LONG] =
    {
        [0] =
        {
            0, 0, 0, 0,
            0, 0, 0, 0,
            1, 1, 1, 1,
            0, 0, 0, 0,
        },
        [1] =
        {
            0, 1, 0, 0,
            0, 1, 0, 0,
            0, 1, 0, 0,
            0, 1, 0, 0,
        },
        [2] =
        {
            0, 0, 0, 0,
            0, 0, 0, 0,
            1, 1, 1, 1,
            0, 0, 0, 0,
        },
        [3] =
        {
            0, 1, 0, 0,
            0, 1, 0, 0,
            0, 1, 0, 0,
            0, 1, 0, 0,
        },
    },
    [PIECE_CUBE] =
    {
        [0] =
        {
            0, 0, 0, 0,
            0, 1, 1, 0,
            0, 1, 1, 0,
            0, 0, 0, 0,
        },
        [1] =
        {
            0, 0, 0, 0,
            0, 1, 1, 0,
            0, 1, 1, 0,
            0, 0, 0, 0,
        },
        [2] =
        {
            0, 0, 0, 0,
            0, 1, 1, 0,
            0, 1, 1, 0,
            0, 0, 0, 0,
        },
        [3] =
        {
            0, 0, 0, 0,
            0, 1, 1, 0,
            0, 1, 1, 0,
            0, 0, 0, 0,
        },
    },
    [PIECE_L1] =
    {
        [0] =
        {
            0, 0, 0, 0,
            1, 0, 0, 0,
            1, 1, 1, 0,
            0, 0, 0, 0,
        },
        [1] =
        {
            0, 0, 0, 0,
            0, 1, 1, 0,
            0, 1, 0, 0,
            0, 1, 0, 0,
        },
        [2] =
        {
            0, 0, 0, 0,
            0, 0, 0, 0,
            1, 1, 1, 0,
            0, 0, 1, 0,
        },
        [3] =
        {
            0, 0, 0, 0,
            0, 1, 0, 0,
            0, 1, 0, 0,
            1, 1, 0, 0,
        },
    },
    [PIECE_L2] =
    {
        [0] =
        {
            0, 0, 0, 0,
            0, 0, 1, 0,
            1, 1, 1, 0,
            0, 0, 0, 0,
        },
        [1] =
        {
            0, 0, 0, 0,
            0, 1, 0, 0,
            0, 1, 0, 0,
            0, 1, 1, 0,
        },
        [2] =
        {
            0, 0, 0, 0,
            0, 0, 0, 0,
            1, 1, 1, 0,
            1, 0, 0, 0,
        },
        [3] =
        {
            0, 0, 0, 0,
            1, 1, 0, 0,
            0, 1, 0, 0,
            0, 1, 0, 0,
        },
    },
    [PIECE_S1] =
    {
        [0] =
        {
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 1, 1, 0,
            1, 1, 0, 0,
        },
        [1] =
        {
            0, 0, 0, 0,
            1, 0, 0, 0,
            1, 1, 0, 0,
            0, 1, 0, 0,
        },
        [2] =
        {
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 1, 1, 0,
            1, 1, 0, 0,
        },
        [3] =
        {
            0, 0, 0, 0,
            1, 0, 0, 0,
            1, 1, 0, 0,
            0, 1, 0, 0,
        },
    },
    [PIECE_S2] =
    {
        [0] =
        {
            0, 0, 0, 0,
            0, 0, 0, 0,
            1, 1, 0, 0,
            0, 1, 1, 0,
        },
        [1] =
        {
            0, 0, 0, 0,
            0, 0, 1, 0,
            0, 1, 1, 0,
            0, 1, 0, 0,
        },
        [2] =
        {
            0, 0, 0, 0,
            0, 0, 0, 0,
            1, 1, 0, 0,
            0, 1, 1, 0,
        },
        [3] =
        {
            0, 0, 0, 0,
            0, 0, 1, 0,
            0, 1, 1, 0,
            0, 1, 0, 0,
        },
    },
    [PIECE_T] =
    {
        [0] =
        {
            0, 0, 0, 0,
            0, 1, 0, 0,
            1, 1, 1, 0,
            0, 0, 0, 0,
        },
        [1] =
        {
            0, 0, 0, 0,
            0, 1, 0, 0,
            0, 1, 1, 0,
            0, 1, 0, 0,
        },
        [2] =
        {
            0, 0, 0, 0,
            0, 0, 0, 0,
            1, 1, 1, 0,
            0, 1, 0, 0,
        },
        [3] =
        {
            0, 0, 0, 0,
            0, 1, 0, 0,
            1, 1, 0, 0,
            0, 1, 0, 0,
        },
    },
};

int score = 0;

static struct timeval now, last_time;

void
fatal_error(char *error, ...)
{
    va_list args;
    va_start(args, error);
    fprintf(stderr, "error: ");
    vfprintf(stderr, error, args);
    fputc('\n', stderr);
    va_end(args);
    exit(EXIT_FAILURE);
}

int
random_integer_between(int min, int max)
{
    return (int)(((double)(max - min + 1) / RAND_MAX) * rand() + min);
}

void
draw_background(SDL_Renderer *renderer)
{
    SDL_Rect rect = (SDL_Rect)
    {
        .x = 0,
        .y = 0,
        .w = WINDOW_WIDTH,
        .h = WINDOW_HEIGHT,
    };
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderFillRect(renderer, &rect);
}

void
initialize_cells(void)
{
    memset(cell, 0, sizeof(cell));
}

void
draw_cells(SDL_Renderer *renderer)
{
    SDL_Rect srcrect, dstrect;
    srcrect = (SDL_Rect)
    {
        .x = 0,
        .w = 8,
        .h = 8,
    };
    dstrect = (SDL_Rect)
    {
        .w = CELL_SIZE,
        .h = CELL_SIZE,
    };
    struct color c;
    for (int y = 0; y < CELLS_PER_COLUMN; y++)
    {
        for (int x = 0; x < CELLS_PER_ROW; x++)
        {
            dstrect.x = MARGIN + x * CELL_SIZE,
            dstrect.y = MARGIN + y * CELL_SIZE,
            srcrect.y = 8 * cell[y * CELLS_PER_ROW + x];
            c = cell_color[cell[y * CELLS_PER_ROW + x]];
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
            if (sprite_atlas != NULL)
            {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
                SDL_RenderCopy(renderer, sprite_atlas, &srcrect, &dstrect);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_MOD);
            }
            SDL_RenderFillRect(renderer, &dstrect);
        }
    }
}

void
draw_piece(SDL_Renderer *renderer)
{
    SDL_Rect srcrect, dstrect;
    srcrect = (SDL_Rect)
    {
        .x = 0,
        .w = 8,
        .h = 8,
    };
    dstrect = (SDL_Rect)
    {
        .w = CELL_SIZE,
        .h = CELL_SIZE,
    };
    srcrect.y = 8 * current_piece.color;
    struct color c = cell_color[current_piece.color];
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (piece_cell[current_piece.type]
                          [current_piece.angle]
                          [y * 4 + x] == 0)
                continue;
            dstrect.x = MARGIN + (current_piece.x + x) * CELL_SIZE;
            dstrect.y = MARGIN + (current_piece.y + y) * CELL_SIZE;
            if (sprite_atlas != NULL)
            {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
                SDL_RenderCopy(renderer, sprite_atlas, &srcrect, &dstrect);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_MOD);
            }
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_MOD);
            SDL_RenderFillRect(renderer, &dstrect);
        }
    }
}

void
draw_score(SDL_Renderer *renderer, int score)
{
    static TTF_Font *font = NULL;
    SDL_Texture *texture;
    SDL_Surface *image;
    SDL_Rect rect;
    char text[100];

    if (font == NULL)
    {
        TTF_Init();
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 24);
    }

    sprintf(text, "score: %i", score);
    image = TTF_RenderText_Solid(font, text, (SDL_Color) { 255, 255, 255});
    texture = SDL_CreateTextureFromSurface(renderer, image);

    rect.x = WINDOW_WIDTH - 100 - MARGIN;
    rect.y = MARGIN;
    rect.w = 100;
    rect.h = CELL_SIZE;

    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_FreeSurface(image);
    SDL_DestroyTexture(texture);
}

void
draw_game_over_text(SDL_Renderer *renderer)
{
    SDL_Texture *texture;
    SDL_Surface *image;
    SDL_Rect rect;
    char text[100];

    if (font == NULL)
    {
        TTF_Init();
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 24);
    }

    image = TTF_RenderText_Solid(font, "Game Over",
                                 (SDL_Color) { 255, 255, 255});
    texture = SDL_CreateTextureFromSurface(renderer, image);

    rect.x = MARGIN;
    rect.y = MARGIN + (CELLS_PER_COLUMN - 2) / 2 * CELL_SIZE;
    rect.w = CELLS_PER_ROW * CELL_SIZE;
    rect.h = 2 * CELL_SIZE;

    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_FreeSurface(image);
    SDL_DestroyTexture(texture);
}

bool
piece_will_collide(int type, int x, int y, int angle)
{
    char *pcell1 = piece_cell[type][angle],
          pcell2[4*4];
    int px1, py1,
        px2, py2;


    // get the left boundary of the piece.
    for (int x = 0; x < 4; x++)
    {
        for (int y = 0; y < 4; y++)
        {
            if (pcell1[y * 4 + x] == 0)
                continue;
            px1 = x;
            goto got_left_boundary;
        }
    }
    got_left_boundary:

    // get the right boundary of the piece.
    for (int x = 3; x >= 0; x--)
    {
        for (int y = 0; y < 4; y++)
        {
            if (pcell1[y * 4 + x] == 0)
                continue;
            px2 = x;
            goto got_right_boundary;
        }
    }
    got_right_boundary:

    // get the top boundary of the piece.
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (pcell1[y * 4 + x] == 0)
                continue;
            py1 = y;
            goto got_top_boundary;
        }
    }
    got_top_boundary:

    // get the bottom boundary of the piece.
    for (int y = 3; y >= 0; y--)
    {
        for (int x = 0; x < 4; x++)
        {
            if (pcell1[y * 4 + x] == 0)
                continue;
            py2 = y;
            goto got_bottom_boundary;
        }
    }
    got_bottom_boundary:

    // get the cells on screen for the target location
    for (int yoff = 0; yoff < 4; yoff++)
    {
        for (int xoff = 0; xoff < 4; xoff++)
        {
            pcell2[yoff * 4 + xoff] =
                (cell[(y + yoff) * CELLS_PER_ROW
                      + x + xoff] != 0);
        }
    }

    // check if it will collide with the left wall
    if (px1 + x < 0)
        return true;

    // check if it will collide with the right wall
    if (px2 + x >= CELLS_PER_ROW)
        return true;

    // check if it will collide with the floor
    if (py2 + y >= CELLS_PER_COLUMN)
        return true;

    // check if it will collide with cells
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (pcell1[y * 4 + x] == 1 &&
                pcell2[y * 4 + x] == 1)
                return true;
        }
    }

    return false;
}

void
write_piece(void)
{
    char *pcell = piece_cell[current_piece.type][current_piece.angle];
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (pcell[y * 4 + x] != 0)
                cell[(current_piece.y + y) * CELLS_PER_ROW
                     + current_piece.x + x] = current_piece.color;
        }
    }
}

int
lines_completed(void)
{
    int old_score = score;
    for (int y = 0; y < CELLS_PER_COLUMN; y++)
    {
        int count = 0;
        for (int x = 0; x < CELLS_PER_ROW; x++)
        {
            if (cell[y * CELLS_PER_ROW + x] == 0)
                break;
            count++;
        }
        if (count < CELLS_PER_ROW || y == 0)
            continue;
        for (int i = y; i > 0; i--)
            memmove(&cell[i * CELLS_PER_ROW],
                    &cell[(i - 1) * CELLS_PER_ROW],
                    CELLS_PER_ROW);
        score++;
    }
    return score - old_score;
}

float
getdt(void)
{
    struct timeval res;
    gettimeofday(&now, NULL);
    timersub(&now, &last_time, &res);
    return (float)res.tv_usec / 1000000;
}

int
main(int argc, char *argv[])
{
    bool game_running = true;
    bool waiting_for_keypress = false;
    float timer = 0, speed = 4.0;
    int new_angle;
    int previous_color = 0;

    SDL_Window *window = NULL;
    SDL_Renderer *renderer;
    SDL_Event event;
    SDL_Surface *sprites = NULL;
    Mix_Music *music = NULL;
    Mix_Chunk *snd_piece_move = NULL,
              *snd_piece_rotate = NULL,
              *snd_piece_landed = NULL,
              *snd_line_clear = NULL,
              *snd_game_over = NULL;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN,
                                &window, &renderer);
    if (window == NULL)
        fatal_error("failed to create window");
    SDL_SetWindowTitle(window, "tetris");

    sprites = IMG_Load("sprites.png");
    if (sprites != NULL)
    {
        sprite_atlas = SDL_CreateTextureFromSurface(renderer, sprites);
        SDL_FreeSurface(sprites);
    }

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    music = Mix_LoadMUS("music.ogg");
    snd_piece_move = Mix_LoadWAV("./sounds/piece_move.wav");
    snd_piece_rotate = Mix_LoadWAV("./sounds/piece_rotate.wav");
    snd_piece_landed = Mix_LoadWAV("./sounds/piece_landed.wav");
    snd_line_clear = Mix_LoadWAV("./sounds/line_clear.wav");
    snd_game_over = Mix_LoadWAV("./sounds/game_over.wav");

    initialize_cells();

    srand(time(NULL));
    gettimeofday(&last_time, NULL);

    while (game_running)
    {
        if (!waiting_for_keypress)
        {
            if (Mix_PlayingMusic() == 0 && music != NULL)
                Mix_PlayMusic(music, -1);
            if (current_piece.type == PIECE_NONE)
            {
                current_piece.type = random_integer_between(1, PIECE_COUNT);
                current_piece.color = random_integer_between(1, 5);
                if (current_piece.color == previous_color)
                {
                    if (current_piece.color < 5)
                        current_piece.color++;
                    else
                        current_piece.color--;
                }
                current_piece.x = (CELLS_PER_ROW - 4) / 2;
                current_piece.y = -1;
                current_piece.angle = 0;
                previous_color = current_piece.color;
                if (piece_will_collide(current_piece.type,
                                       current_piece.x,
                                       current_piece.y,
                                       current_piece.angle))
                {
                    if (Mix_PlayingMusic())
                        Mix_HaltMusic();
                    if (snd_game_over != NULL)
                        Mix_PlayChannel(-1, snd_game_over, 0);
                    waiting_for_keypress = true;
                }
            }
            if (timer <= 0.0)
            {
                if (piece_will_collide(current_piece.type,
                                       current_piece.x,
                                       current_piece.y + 1,
                                       current_piece.angle))
                {
                    write_piece();
                    if (lines_completed() > 0 && snd_line_clear != NULL)
                        Mix_PlayChannel(-1, snd_line_clear, 0);
                    else if (snd_piece_landed != NULL)
                        Mix_PlayChannel(-1, snd_piece_landed, 0);
                    current_piece.type = PIECE_NONE;
                }
                else current_piece.y += 1;
                timer += 1.0;
            }
            else timer -= speed * getdt();

            if (current_piece.y == CELLS_PER_COLUMN - 1)
                current_piece.type = PIECE_NONE;
        }

        draw_background(renderer);
        draw_cells(renderer);
        draw_piece(renderer);
        draw_score(renderer, score);

        if (waiting_for_keypress)
            draw_game_over_text(renderer);

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    game_running = 0;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_LEFT:
                            if (!piece_will_collide(current_piece.type,
                                                    current_piece.x - 1,
                                                    current_piece.y,
                                                    current_piece.angle))
                            {
                                current_piece.x -= 1;
                                if (snd_piece_move != NULL)
                                    Mix_PlayChannel(-1, snd_piece_move, 0);
                            }
                            break;
                        case SDLK_RIGHT:
                            if (!piece_will_collide(current_piece.type,
                                                    current_piece.x + 1,
                                                    current_piece.y,
                                                    current_piece.angle))
                            {
                                current_piece.x += 1;
                                if (snd_piece_move != NULL)
                                    Mix_PlayChannel(-1, snd_piece_move, 0);
                            }
                            break;
                        case SDLK_DOWN:
                            speed = 16.0;  8.0;
                            break;
                        case 'z':
                            new_angle = (current_piece.angle + 3) % 4;
                            if (!piece_will_collide(current_piece.type,
                                                    current_piece.x,
                                                    current_piece.y,
                                                    new_angle))
                            {
                                current_piece.angle = new_angle;
                                if (snd_piece_rotate != NULL)
                                    Mix_PlayChannel(-1, snd_piece_rotate, 0);
                            }
                            break;
                        case 'x':
                            if (waiting_for_keypress)
                            {
                                waiting_for_keypress = false;
                                score = 0;
                                initialize_cells();
                                break;
                            }
                            new_angle = (current_piece.angle + 1) % 4;
                            if (!piece_will_collide(current_piece.type,
                                                    current_piece.x,
                                                    current_piece.y,
                                                    new_angle))
                            {
                                current_piece.angle = new_angle;
                                if (snd_piece_rotate != NULL)
                                    Mix_PlayChannel(-1, snd_piece_rotate, 0);
                            }
                            break;
                    }
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_DOWN:
                            speed = 4.0;
                            break;
                    }
                    break;
            }
        }

        last_time = now;
    }

    if (snd_game_over != NULL)
        Mix_FreeChunk(snd_game_over);

    if (snd_line_clear != NULL)
        Mix_FreeChunk(snd_line_clear);

    if (snd_piece_landed != NULL)
        Mix_FreeChunk(snd_piece_landed);

    if (snd_piece_rotate != NULL)
        Mix_FreeChunk(snd_piece_rotate);

    if (snd_piece_move != NULL)
        Mix_FreeChunk(snd_piece_move);

    if (music != NULL)
        Mix_FreeMusic(music);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
