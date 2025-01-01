#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define RAM_SIZE 4096
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define NUM_REGS 16
#define STACK_SIZE 16
#define NUM_KEYS 16

#define START_ADDR 0x200

#define FONTSET_SIZE 80

typedef struct chip8emu *chip8; 

chip8 init_emulator(void);
void destroy_emulator(chip8);

uint16_t get_pc(chip8);
void set_pc(chip8, uint16_t);

uint8_t get_ram(chip8, int);
void set_ram(chip8, uint8_t, int);

bool get_screen(chip8, int);
void set_screen(chip8, bool, int);

uint8_t get_vreg(chip8, int);
void set_vreg(chip8, uint8_t, int);

uint16_t get_ireg(chip8);
void set_ireg(chip8, uint16_t);

uint16_t get_sp(chip8);
void set_sp(chip8, uint16_t);

uint16_t get_stack(chip8, int);
void set_stack(chip8, uint16_t, int);

bool get_key(chip8, int);
void set_key(chip8, bool, int);

uint8_t get_dt(chip8);
void set_dt(chip8, uint8_t);

uint8_t get_st(chip8);
void set_st(chip8, uint8_t);

bool* get_display(chip8);
void keypress(chip8, uint16_t, bool);
void load(chip8, uint8_t*, size_t);

void reset(chip8);

void stack_push(chip8, uint16_t);
uint16_t stack_pop(chip8);

void tick(chip8);
uint16_t fetch(chip8);
void tick_timer(chip8);
void execute_draw(chip8, uint8_t, uint8_t, uint8_t);
void execute(chip8, uint16_t);

#endif
