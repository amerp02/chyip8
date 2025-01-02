#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <stddef.h>
#include <stdio.h>
#include "../include/chip8.h"
#include "../include/helpers.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>
#include <stdint.h>
#include <time.h>

#define SCALE 15
#define WIN_WIDTH SCREEN_WIDTH*SCALE
#define WIN_HEIGHT SCREEN_HEIGHT*SCALE

#define TICKS_PER_FRAME 10

void draw_test(SDL_Renderer*);
uint16_t key2btn(SDL_Keycode);
void draw_screen(chip8, SDL_Renderer*);

void draw_test(SDL_Renderer* renderer) {
    SDL_Surface* image_surface = IMG_Load("../img/51Y6ShMGJHL._AC_UF894,1000_QL80_.jpg");
    if (!image_surface) {
        fprintf(stderr, "Failed to load image: %s\n", IMG_GetError());
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, image_surface);
    SDL_FreeSurface(image_surface);
    if (!texture) {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        return;
    }
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_Delay(3000);
    SDL_DestroyTexture(texture);
}

uint16_t key2btn(SDL_Keycode key) {
    switch(key) {
        case SDLK_1: return 0x1;
        case SDLK_2: return 0x2;
        case SDLK_3: return 0x3;
        case SDLK_4: return 0xC;
        case SDLK_q: return 0x4;
        case SDLK_w: return 0x5;
        case SDLK_e: return 0x6;
        case SDLK_r: return 0xD;
        case SDLK_a: return 0x7;
        case SDLK_s: return 0x8;
        case SDLK_d: return 0x9;
        case SDLK_f: return 0xE;
        case SDLK_z: return 0xA;
        case SDLK_x: return 0x0;
        case SDLK_c: return 0xB;
        case SDLK_v: return 0xF;
        default: return -1;    
    }
}

void draw_screen(chip8 emu, SDL_Renderer* renderer) {
    bool* screen_buf = get_display(emu);
    int active_pixels = 0;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        if (screen_buf[i]) {
            active_pixels++;
            int x = (i % SCREEN_WIDTH) * SCALE;
            int y = (i / SCREEN_WIDTH) * SCALE;
            SDL_Rect rect = {x, y, SCALE, SCALE};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    printf("Active pixels: %d\n", active_pixels);
    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s path/to/game\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window *window = SDL_CreateWindow(
        "CHIP8 EMU",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIN_WIDTH,
        WIN_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );
    if (!window) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    draw_test(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    SDL_Event event;

    chip8 emu = init_emulator();
    printf("Checking loaded fontset...\n");
    for (int i = 0; i < FONTSET_SIZE; i++) {
        printf("Font[%02X] = %02X\n", i, get_ram(emu, i));
    }

    FILE* rom = fopen(argv[1], "rb");
    if (!rom) {
        perror("Unable to open file");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    fseek(rom, 0, SEEK_END);
    size_t rom_size = ftell(rom);
    printf("rom size: %zu bytes\n", rom_size);
    rewind(rom);

    uint8_t *buffer = (uint8_t *)malloc(rom_size);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate memory for ROM\n");
        fclose(rom);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    fread(buffer, 1, rom_size, rom);
    fclose(rom);

    load(emu, buffer, rom_size);
    for (size_t i = START_ADDR; i < START_ADDR + 16; i++) {
        printf("RAM[%04X] = %02X\n", (unsigned int)i, get_ram(emu, i));
    }

    free(buffer);

    bool running = true;
    while (running) {
        printf("RUNNING MAIN LOOP...\n");
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        running = false;
                    } else {
                        int key = key2btn(event.key.keysym.sym);
                        if (key != -1) {
                            keypress(emu, key, true);
                        }
                    }
                    break;
                case SDL_KEYUP: {
                    int key = key2btn(event.key.keysym.sym);
                    if (key != -1) {
                        keypress(emu, key, false);
                    }
                    break;
                }
                default:
                    break;
            }
        }

        for (int i = 0; i < TICKS_PER_FRAME; i++) {
            tick(emu);
        }

        tick_timer(emu);

        draw_screen(emu, renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    destroy_emulator(emu);

    return EXIT_SUCCESS;
}
