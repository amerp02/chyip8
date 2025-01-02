#ifndef HELPERS_H
#define HELPERS_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "chip8.h"

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
