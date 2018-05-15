//
// Created by User on 14.04.2018.
//

#ifndef EMULATOR_ALL_H
#define EMULATOR_ALL_H
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
//#include "functions.c"
//#include "take.c"

typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;
typedef unsigned int double_word;   //для проверки carry по 17 биту

typedef union s_byte {
    char s_b;
    byte u_b;
} s_byte;

typedef union s_word {
    short int s_w;
    word u_w;
} s_word;

struct Data {
    word w;
    adr mem_adr;
};

struct status {
    byte N;
    byte Z;
    byte V;
    byte C;
};

#define pc reg[7]   //7 регистр используем как прог. каунтер

#define out 0177564

#define NO_PARAM 0
#define HAS_SS 1
#define HAS_DD (1<<1)
#define HAS_XX (1<<2)
#define HAS_NN (1<<3)

#define LO(x) ((x) & 0xFF)
#define HI(x) (((x) >> 8) & 0xFF)
#define src 1
#define dst 2

#define B(w) (((w) & 0100000) >> 15)

#define src_mode(w) (((w) & 007000) >> 9)
#define dst_mode(w) (((w) & 000070) >> 3)

#define src_reg(w) (((w) & 000700) >> 6)
#define dst_reg(w) ((w) & 000007)

word get_nn(word w);
word get_ss(word w);
byte get_offset(word w);
byte b_read (adr a);
void b_write (adr a, byte val);
void w_write (adr a, word val);
word w_read (adr a);
void reg_check();
void smart_reg_check(int a, int b);
void flag_check();
void set_zero (int a);
void set_negative (int a);
void do_halt(word w);
void do_add(word w);
void do_mov(word w);
void do_sob(word w);
void do_clear(word w);
void do_br(word w);
void do_beq(word w);
void do_tstb(word w);
void do_bpl(word w);
void do_jsr(word w);
void do_rts(word w);
void do_ror(word w);
void do_unknown(word w);
void run (adr pc0);
void load_file(FILE* f);
void mem_dump(adr start, word n);
void f_mem_dump(adr start, word n, FILE* f);
void testmem();
struct Data take(word w, int a);


#endif //EMULATOR_ALL_H
