#include "../include/helpers.h"
#include "../include/chip8.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>

void keypress(chip8 emu, uint16_t index, bool pressed) {
    set_key(emu, pressed, index);
}

void load(chip8 emu, uint8_t* data, size_t size) {
    if (size > (RAM_SIZE - START_ADDR)) {
        fprintf(stderr, "ROM size exceeds available memory\n");
        exit(EXIT_FAILURE);
    }
    memcpy(get_ram_ptr(emu, START_ADDR), data, size);

    for (size_t i = START_ADDR; i < START_ADDR + size; i++) {
        printf("RAM[%04X] = %02X\n", (unsigned int)i, get_ram(emu, i));
    }
}

void reset(chip8 emu) {
    set_pc(emu, START_ADDR);
    for(int i = 0; i <= RAM_SIZE; ++i)
        set_ram(emu, 0, i);
    for(int i = 0; i <= (SCREEN_WIDTH * SCREEN_HEIGHT); ++i)
        set_screen(emu, false, i);
    for(int i = 0; i <= NUM_REGS; ++i)
        set_vreg(emu, 0, i);
    set_ireg(emu, 0);
    set_sp(emu, 0);
    for(int i = 0; i <= STACK_SIZE; ++i)
        set_stack(emu, 0, i);
    for(int i = 0; i <= NUM_KEYS; ++i)
        set_key(emu, false, i);
    set_dt(emu, 0);
    set_st(emu, 0);
    
    memcpy(get_ram_ptr(emu, 0), FONTSET, FONTSET_SIZE);
}

void stack_push(chip8 emu, uint16_t value) {
    set_stack(emu, value, get_sp(emu));
    set_sp(emu, get_sp(emu) + 1);
}

uint16_t stack_pop(chip8 emu) {
    set_sp(emu, get_sp(emu) - 1);
    return get_stack(emu, get_sp(emu));
}

void tick(chip8 emu) {
    uint16_t op = fetch(emu);
    execute(emu, op);
}

uint16_t fetch(chip8 emu) {
    uint16_t opcode = get_ram(emu, get_pc(emu)) << 8 | get_ram(emu, get_pc(emu) + 1);
    printf("Fetched opcode: %04X at PC: %04X\n", opcode, get_pc(emu));
    set_pc(emu, get_pc(emu) + 2);
    return opcode;
}

void tick_timer(chip8 emu) {
    if (get_dt(emu) > 0) {
        printf("decrementing DT: %d\n", get_dt(emu));
        set_dt(emu, get_dt(emu) - 1);
    }

    if (get_st(emu) > 0) {
        printf("decrementing ST: %d\n", get_st(emu)); 
        if(get_st(emu) == 1) {
            // do stuff
        }
        set_st(emu, get_st(emu) - 1);
    }
}

void execute_draw(chip8 emu, uint8_t x_reg, uint8_t y_reg, uint8_t height) {
    uint8_t x = get_vreg(emu, x_reg);
    uint8_t y = get_vreg(emu, y_reg);
    set_vreg(emu, 0, 0xF);

    for (uint8_t row = 0; row < height; ++row) {
        uint8_t sprite = get_ram(emu, get_ireg(emu) + row);
        for (uint8_t col = 0; col < 8; ++col) {
            uint16_t idx = ((y + row) % SCREEN_HEIGHT) * SCREEN_WIDTH + ((x + col) % SCREEN_WIDTH);
            bool sprite_pixel = (sprite & (0x80 >> col)) != 0;
            if (sprite_pixel && get_screen(emu, idx)) {
                set_vreg(emu, 1, 0xF);
            }
            set_screen(emu, get_screen(emu, idx) ^ sprite_pixel, idx);
        }
    }
}

void execute(chip8 emu, uint16_t opcode) {
    uint8_t x = (opcode & 0x0F00) >> 8; // digit2
    uint8_t y = (opcode & 0x00F0) >> 4; // digit3
    uint8_t n = opcode & 0x000F; // digit4
    uint8_t nn = opcode & 0x00FF;
    uint16_t nnn = opcode & 0x0FFF;

    uint16_t sum;
    bool borrow;
    uint8_t lsb;
    uint8_t msb;
    uint8_t rng = rand() % 256;
    uint16_t vx;
    uint16_t key;
    bool pressed;
    uint16_t c;
    uint8_t hundreds;
    uint8_t tens;
    uint8_t ones; 
    uint16_t i;
    
    switch (opcode >> 12) { // digit1
        case 0x0:
            if (opcode == 0x00E0) {
                memset(get_display(emu), 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(bool));
                printf("Screen cleared\n");
            } else if (opcode == 0x00EE) {
                uint16_t ret_addr = stack_pop(emu);
                //emu->pc = ret_addr;
                set_pc(emu, ret_addr); 
                printf("Returned from subroutine\n");
            } else {
                printf("Unknown 0x0NNN opcode: 0x%04X\n", opcode);
            }
            break;
        
        case 0x1:
            set_pc(emu, nnn); 
            printf("Jumped\n");
            break;

        case 0x2:
            stack_push(emu, get_pc(emu));
            set_pc(emu, nnn);
            printf("Called subroutine\n");
            break;

        case 0x3:
            if(get_vreg(emu, x) == nn) {
                set_pc(emu, get_pc(emu) + 2);
            }
            printf("Skipped next VX == NN\n");
            break;

        case 0x4:
            if(get_vreg(emu, x) != nn) {
                set_pc(emu, get_pc(emu) + 2);
            }
            printf("Skipped next VX != NN\n");
            break;

        case 0x5:
            if(get_vreg(emu, x) == get_vreg(emu, y)) {
                set_pc(emu, get_pc(emu) + 2);
            }
            printf("Skipped next VX == VY\n");
            break;

        case 0x6:
            set_vreg(emu, nn, x);
            printf("Set V%X = %02X\n", x, nn);
            break;

        case 0x7:
            set_vreg(emu, get_vreg(emu, x) + nn, x); 
            printf("Add %02X to V%X\n", nn, x);
            break;

        case 0x8:
            if(n == 0x0) {
                set_vreg(emu, get_vreg(emu, y), x); 
                printf("Set VX = VY\n");
            } else if(n == 0x1) {
                set_vreg(emu, get_vreg(emu, x) | get_vreg(emu, y), x); 
                printf("Set VX |= VY\n");
            } else if(n == 0x2) {
                set_vreg(emu, get_vreg(emu, x) & get_vreg(emu, y), x); 
                printf("Set VX &= VY\n");
            } else if(n == 0x3) {
                set_vreg(emu, get_vreg(emu, x) ^ get_vreg(emu, y), x); 
                printf("Set VX ^= VY\n");
            } else if(n == 0x4) {
                sum = get_vreg(emu, x) + get_vreg(emu, y);
                set_vreg(emu, (uint8_t)sum, x);
                set_vreg(emu, (sum > 255) ? 1 : 0, 0xF);
                printf("Set VX += VY\n");
            } else if(n == 0x5) {
                borrow = get_vreg(emu, x) < get_vreg(emu, y);
                set_vreg(emu, get_vreg(emu, x) - get_vreg(emu, y), x);
                set_vreg(emu, borrow ? 0 : 1, 0xF);
                printf("Set VX -= VY\n");
            } else if(n == 0x6) {
                lsb = get_vreg(emu, x) & 1;
                set_vreg(emu, get_vreg(emu, x) >> 1, x);
                set_vreg(emu, lsb, 0xF);
                printf("Set VX >>= 1\n");
            } else if(n == 0x7) {
                borrow = get_vreg(emu, y) < get_vreg(emu, x);
                set_vreg(emu, get_vreg(emu, y) - get_vreg(emu, x), x);
                set_vreg(emu, borrow ? 0 : 1, 0xF);
                printf("Set VX = VY - VX\n");
            } else if(n == 0xE) {
                msb = (get_vreg(emu, x) >> 7) & 1;
                set_vreg(emu, get_vreg(emu, x) << 1, x);
                set_vreg(emu, msb, 0xF);
                printf("Set VX <<= 1\n");
            }
            break;
        
        case 0x9:
            if(get_vreg(emu, x) != get_vreg(emu, y)) {
                set_pc(emu, get_pc(emu) + 2);
            } 
            printf("Skipped next VX != VY\n");
            break;

        case 0xA:
            set_ireg(emu, nnn);
            printf("Set I = 0x%03X\n", nnn);
            break;
        
        case 0xB:
            set_pc(emu, get_vreg(emu, 0) + nnn); 
            printf("Jumps to address NNN + V0\n");
            break;

        case 0xC:
            set_vreg(emu, rng & nn, x); 
            printf("Set VX = rand() & NN\n");
            break;

        case 0xD:
            execute_draw(emu, x, y, n);
            printf("Draw sprite at V%X,V%X with height %X\n", x, y, n);
            break;
            
        case 0xE:
            if(y == 0x9 && n == 0xE) {
                vx = get_vreg(emu, x);
                key = get_key(emu, vx);
                if(key) {
                    set_pc(emu, get_pc(emu) + 2);
                    printf("Skipped next key == VX\n");
                }
            } else if (y == 0xA && n == 0x1) {
                vx = get_vreg(emu, x);
                key = get_key(emu, vx);
                if(!key) {
                    set_pc(emu, get_pc(emu) + 2);
                    printf("Skipped next key != VX\n");
                }
            }
            break;

        case 0xF:
            if(y == 0x0 && n == 0x7) {
                //emu->v_reg[x] = emu->dt;
                set_vreg(emu, get_dt(emu), x); 
                printf("Set VX = DT\n");
            } else if(y == 0x0 && n == 0xA) {
                pressed = false;
                for(uint8_t i = 0; i < 16; i++) {
                    if(get_key(emu, i)) {
                        set_vreg(emu, i, x);
                        pressed = true;
                        break;
                    }
                }
                if(!pressed) {
                    set_pc(emu, get_pc(emu) - 2);
                }
                printf("Did FX0A\n");
            } else if(y == 0x1 && n == 0x5) {
                set_dt(emu, get_vreg(emu, x));
                printf("Set DT = VX\n");
            } else if(y == 0x1 && n == 0x8) {
                set_st(emu, get_vreg(emu, x));
                printf("Set ST = VX\n");
            } else if(y == 0x1 && n == 0xE) {
                vx = get_vreg(emu, x);
                set_ireg(emu, get_ireg(emu) + vx);
                printf("Set I += VX\n");
            } else if(y == 0x2 && n == 0x9) {
                c = get_vreg(emu, x);
                set_ireg(emu, c * 5);                
                printf("Did FX29\n");
            } else if(y == 0x3 && n == 0x3) {
                vx = get_vreg(emu, x);
                hundreds = floor((vx / 100));
                tens = floor(vx / 10);
                tens = tens % 10;
                ones = ((int)floor(vx) % 10);
                set_ram(emu, hundreds, get_ireg(emu)); 
                set_ram(emu, tens, get_ireg(emu) + 1); 
                set_ram(emu, ones, get_ireg(emu) + 2); 

                printf("Did FX33\n");
            } else if(y == 0x5 && n == 0x5) {
                i = get_ireg(emu);
                for(int idx = 0; idx < x; idx++) {
                    set_ram(emu, get_vreg(emu, idx), i + idx); 
                }
                printf("Did FX55\n");
            } else if(y == 0x6 && n == 0x5) {
                i = get_ireg(emu);
                for(int idx = 0; idx < x; idx++) {
                    set_vreg(emu, get_ram(emu, i + idx), idx); 
                }
                printf("Did FX65\n");
            }
            break;
        default:
            printf("Unhandled opcode: 0x%04X\n", opcode);
            break;
    }
}
