#include "../include/chip8.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> 
#include <string.h>
#include <time.h>
#include "../include/helpers.h"

#define RAM_SIZE 4096
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define NUM_REGS 16
#define STACK_SIZE 16
#define NUM_KEYS 16

#define START_ADDR 0x200

#define FONTSET_SIZE 80
const uint8_t FONTSET[FONTSET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

struct chip8emu {
    uint16_t pc;
    uint8_t ram[RAM_SIZE];
    bool screen[SCREEN_WIDTH * SCREEN_HEIGHT];
    uint8_t v_reg[NUM_REGS];
    uint16_t i_reg;
    uint16_t sp;
    uint16_t stack[STACK_SIZE];
    bool keys[NUM_KEYS];
    uint8_t dt;
    uint8_t st;
};

chip8 init_emulator(void) {
    srand(time(NULL));
    chip8 emu = (chip8)malloc(sizeof(struct chip8emu));
    if (!emu) {
        fprintf(stderr, "Failed to allocate memory for emulator\n");
        exit(EXIT_FAILURE);
    }
    reset(emu);
    return emu;
}

void destroy_emulator(chip8 emu) {
    free(emu);
}

uint16_t get_pc(chip8 emu) {
    return emu->pc;
}

void set_pc(chip8 emu, uint16_t value) {
    emu->pc = value;
}

uint8_t get_ram(chip8 emu, int index) {
    return emu->ram[index];
}

uint8_t* get_ram_ptr(chip8 emu, int address) {
    return &emu->ram[address];
}

void set_ram(chip8 emu, uint8_t value, int index) {
    emu->ram[index] = value;
}

bool get_screen(chip8 emu, int index) {
    return emu->screen[index];
}

void set_screen(chip8 emu, bool value, int index) {
    emu->screen[index] = value;
}

uint8_t get_vreg(chip8 emu, int index) {
    return emu->v_reg[index];
}

void set_vreg(chip8 emu, uint8_t value, int index) {
    emu->v_reg[index] = value;
}

uint16_t get_ireg(chip8 emu) {
    return emu->i_reg;
}

void set_ireg(chip8 emu, uint16_t value) {
    emu->i_reg = value;
}

uint16_t get_sp(chip8 emu) {
    return emu->sp;
}

void set_sp(chip8 emu, uint16_t value) {
    emu->sp = value;
}

uint16_t get_stack(chip8 emu, int index) {
    return emu->stack[index];
}

void set_stack(chip8 emu, uint16_t value, int index) {
    emu->stack[index] = value;
}

bool get_key(chip8 emu, int index) {
    return emu->keys[index];
}

void set_key(chip8 emu, bool value, int index) {
    emu->keys[index] = value;
}

uint8_t get_dt(chip8 emu) {
    return emu->dt;
} 

void set_dt(chip8 emu, uint8_t value) {
    emu->dt = value;
}
uint8_t get_st(chip8 emu) {
    return emu->st; 
}

void set_st(chip8 emu, uint8_t value) {
    emu->st = value;
}

bool* get_display(chip8 emu) {
    return emu->screen;
}

// void keypress(chip8 emu, uint16_t index, bool pressed) {
//     emu->keys[index] = pressed;
// }
//
// void load(chip8 emu, uint8_t* data, size_t size) {
//     if (size > (RAM_SIZE - START_ADDR)) {
//         fprintf(stderr, "ROM size exceeds available memory\n");
//         exit(EXIT_FAILURE);
//     }
//     memcpy(&emu->ram[START_ADDR], data, size);
//
//     // Log loaded ROM
//     for (size_t i = START_ADDR; i < START_ADDR + size; i++) {
//         printf("RAM[%04X] = %02X\n", (unsigned int)i, emu->ram[i]);
//     }
// }
//
// void reset(chip8 emu) {
//     emu->pc = START_ADDR;
//     for(int i = 0; i <= RAM_SIZE; ++i)
//         emu->ram[i] = 0;
//     for(int i = 0; i <= (SCREEN_WIDTH * SCREEN_HEIGHT); ++i)
//         emu->screen[i] = false;
//     for(int i = 0; i <= NUM_REGS; ++i)
//         emu->v_reg[i] = 0;
//     emu->i_reg = 0;
//     emu->sp = 0;
//     for(int i = 0; i <= STACK_SIZE; ++i)
//         emu->stack[i] = 0;
//     for(int i = 0; i <= NUM_KEYS; ++i)
//         emu->keys[i] = false;
//     emu->dt = 0;
//     emu->st = 0;
//     
//     memcpy(emu->ram, FONTSET, FONTSET_SIZE);
// }
//
// void stack_push(chip8 emu, uint16_t value) {
//     emu->stack[emu->sp] = value;
//     emu->sp += 1;
// }
//
// uint16_t stack_pop(chip8 emu) {
//     emu->sp -= 1;
//     return emu->stack[emu->sp];
// }
//
// void tick(chip8 emu) {
//     uint16_t op = fetch(emu);
//     execute(emu, op);
// }
//
// uint16_t fetch(chip8 emu) {
//     uint16_t opcode = (emu->ram[emu->pc] << 8) | emu->ram[emu->pc + 1];
//     printf("Fetched opcode: %04X at PC: %04X\n", opcode, emu->pc);
//     emu->pc += 2;
//     return opcode;
// }
//
// void tick_timer(chip8 emu) {
//     if (emu->dt > 0) {
//         printf("decrementing DT: %d\n", emu->dt);
//         emu->dt -= 1;
//     }
//
//     if (emu->st > 0) {
//         printf("decrementing ST: %d\n", emu->st); 
//         if(emu->st == 1) {
//             // do stuff
//         }
//         emu->st -= 1;
//     }
// }
//
// void execute_draw(chip8 emu, uint8_t x_reg, uint8_t y_reg, uint8_t height) {
//     uint8_t x = emu->v_reg[x_reg];
//     uint8_t y = emu->v_reg[y_reg];
//     emu->v_reg[0xF] = 0; // Reset collision flag
//
//     for (uint8_t row = 0; row < height; ++row) {
//         uint8_t sprite = emu->ram[emu->i_reg + row];
//         for (uint8_t col = 0; col < 8; ++col) {
//             uint16_t idx = ((y + row) % SCREEN_HEIGHT) * SCREEN_WIDTH + ((x + col) % SCREEN_WIDTH);
//             bool sprite_pixel = (sprite & (0x80 >> col)) != 0;
//             if (sprite_pixel && emu->screen[idx]) {
//                 emu->v_reg[0xF] = 1; // Collision detected
//             }
//             emu->screen[idx] ^= sprite_pixel;
//         }
//     }
// }
//
// void execute(chip8 emu, uint16_t opcode) {
//     uint8_t x = (opcode & 0x0F00) >> 8; // digit2
//     uint8_t y = (opcode & 0x00F0) >> 4; // digit3
//     uint8_t n = opcode & 0x000F; // digit4
//     uint8_t nn = opcode & 0x00FF;
//     uint16_t nnn = opcode & 0x0FFF;
//
//     uint16_t sum;
//     bool borrow;
//     uint8_t lsb;
//     uint8_t msb;
//     uint8_t rng = rand() % 256;
//     uint16_t vx;
//     uint16_t key;
//     bool pressed;
//     uint16_t c;
//     uint8_t hundreds;
//     uint8_t tens;
//     uint8_t ones; 
//     uint16_t i;
//     
//     switch (opcode >> 12) { // digit1
//         case 0x0:
//             if (opcode == 0x00E0) {
//                 memset(emu->screen, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(bool));
//                 printf("Screen cleared\n");
//             } else if (opcode == 0x00EE) {
//                 uint16_t ret_addr = stack_pop(emu);
//                 emu->pc = ret_addr;
//                 printf("Returned from subroutine\n");
//             } else {
//                 printf("Unknown 0x0NNN opcode: 0x%04X\n", opcode);
//             }
//             break;
//         
//         case 0x1:
//             emu->pc = nnn;
//             printf("Jumped\n");
//             break;
//
//         case 0x2:
//             stack_push(emu, emu->pc);
//             emu->pc = nnn;
//             printf("Called subroutine\n");
//             break;
//
//         case 0x3:
//             if(emu->v_reg[x] == nn) {
//                 emu->pc += 2;
//             }
//             printf("Skipped next VX == NN\n");
//             break;
//
//         case 0x4:
//             if(emu->v_reg[x] != nn) {
//                 emu->pc += 2;
//             }
//             printf("Skipped next VX != NN\n");
//             break;
//
//         case 0x5:
//             if(emu->v_reg[x] == emu->v_reg[y]) {
//                 emu->pc += 2;
//             }
//             printf("Skipped next VX == VY\n");
//             break;
//
//         case 0x6: // Set Vx = NN
//             emu->v_reg[x] = nn;
//             printf("Set V%X = %02X\n", x, nn);
//             break;
//
//         case 0x7: // Add NN to Vx
//             emu->v_reg[x] += nn;
//             printf("Add %02X to V%X\n", nn, x);
//             break;
//
//         case 0x8:
//             if(n == 0x0) {
//                 emu->v_reg[x] = emu->v_reg[y];
//                 printf("Set VX = VY\n");
//             } else if(n == 0x1) {
//                 emu->v_reg[x] |= emu->v_reg[y];
//                 printf("Set VX |= VY\n");
//             } else if(n == 0x2) {
//                 emu->v_reg[x] &= emu->v_reg[y];
//                 printf("Set VX &= VY\n");
//             } else if(n == 0x3) {
//                 emu->v_reg[x] ^= emu->v_reg[y];
//                 printf("Set VX ^= VY\n");
//             } else if(n == 0x4) {
//                 sum = emu->v_reg[x] + emu->v_reg[y];
//                 emu->v_reg[x] = (uint8_t)sum;
//                 emu->v_reg[0xF] = (sum > 255) ? 1 : 0;
//                 printf("Set VX += VY\n");
//             } else if(n == 0x5) {
//                 borrow = emu->v_reg[x] < emu->v_reg[y];
//                 emu->v_reg[x] = emu->v_reg[x] - emu->v_reg[y];
//                 emu->v_reg[0xF] = borrow ? 0 : 1;
//                 printf("Set VX -= VY\n");
//             } else if(n == 0x6) {
//                 lsb = emu->v_reg[x] & 1;
//                 emu->v_reg[x] >>= 1;
//                 emu->v_reg[0xF] = lsb;
//                 printf("Set VX >>= 1\n");
//             } else if(n == 0x7) {
//                 borrow = emu->v_reg[y] < emu->v_reg[x];
//                 emu->v_reg[x] = emu->v_reg[y] - emu->v_reg[x];
//                 emu->v_reg[0xF] = borrow ? 0 : 1;
//                 printf("Set VX = VY - VX\n");
//             } else if(n == 0xE) {
//                 msb = (emu->v_reg[x] >> 7) & 1;
//                 emu->v_reg[x] <<= 1;
//                 emu->v_reg[0xF] = msb;
//                 printf("Set VX <<= 1\n");
//             }
//             break;
//         
//         case 0x9:
//             if(emu->v_reg[x] != emu->v_reg[y]) {
//                 emu->pc += 2;
//             } 
//             printf("Skipped next VX != VY\n");
//             break;
//
//         case 0xA:
//             emu->i_reg = nnn;
//             printf("Set I = 0x%03X\n", nnn);
//             break;
//         
//         case 0xB:
//             emu->pc = (emu->v_reg[0]) + nnn;
//             printf("Jumps to address NNN + V0\n");
//             break;
//
//         case 0xC:
//             emu->v_reg[x] = rng & nn;
//             printf("Set VX = rand() & NN\n");
//             break;
//
//         case 0xD:
//             execute_draw(emu, x, y, n);
//             printf("Draw sprite at V%X,V%X with height %X\n", x, y, n);
//             break;
//             
//         case 0xE:
//             if(y == 0x9 && n == 0xE) {
//                 vx = emu->v_reg[x];
//                 key = emu->keys[vx];
//                 if(key) {
//                     emu->pc += 2;
//                     printf("Skipped next key == VX\n");
//                 }
//             } else if (y == 0xA && n == 0x1) {
//                 vx = emu->v_reg[x];
//                 key = emu->keys[vx];
//                 if(!key) {
//                     emu->pc += 2;
//                     printf("Skipped next key != VX\n");
//                 }
//             }
//             break;
//
//         case 0xF:
//             if(y == 0x0 && n == 0x7) {
//                 emu->v_reg[x] = emu->dt;
//                 printf("Set VX = DT\n");
//             } else if(y == 0x0 && n == 0xA) {
//                 pressed = false;
//                 for(uint8_t i = 0; i < 16; i++) {
//                     if(emu->keys[i]) {
//                         emu->v_reg[x] = i;
//                         pressed = true;
//                         break;
//                     }
//                 }
//                 if(!pressed) {
//                     emu->pc -= 2;
//                 }
//                 printf("Did FX0A\n");
//             } else if(y == 0x1 && n == 0x5) {
//                 emu->dt = emu->v_reg[x];
//                 printf("Set DT = VX\n");
//             } else if(y == 0x1 && n == 0x8) {
//                 emu->st = emu->v_reg[x];
//                 printf("Set ST = VX\n");
//             } else if(y == 0x1 && n == 0xE) {
//                 vx = emu->v_reg[x];
//                 emu->i_reg += vx;
//                 printf("Set I += VX\n");
//             } else if(y == 0x2 && n == 0x9) {
//                 c = emu->v_reg[x];
//                 emu->i_reg = c * 5;
//                 printf("Did FX29\n");
//             } else if(y == 0x3 && n == 0x3) {
//                 vx = emu->v_reg[x];
//                 hundreds = floor((vx / 100));
//                 tens = floor(vx / 10);
//                 tens = tens % 10;
//                 ones = ((int)floor(vx) % 10);
//                 emu->ram[emu->i_reg] = hundreds;
//                 emu->ram[emu->i_reg + 1] = tens;
//                 emu->ram[emu->i_reg + 2] = ones;
//                 printf("Did FX33\n");
//             } else if(y == 0x5 && n == 0x5) {
//                 i = emu->i_reg;
//                 for(int idx = 0; idx < x; idx++) {
//                     emu->ram[i + idx] = emu->v_reg[idx];
//                 }
//                 printf("Did FX55\n");
//             } else if(y == 0x6 && n == 0x5) {
//                 i = emu->i_reg;
//                 for(int idx = 0; idx < x; idx++) {
//                     emu->v_reg[idx] = emu->ram[i + idx];
//                 }
//                 printf("Did FX65\n");
//             }
//             break;
//         default:
//             printf("Unhandled opcode: 0x%04X\n", opcode);
//             break;
//     }
// }
