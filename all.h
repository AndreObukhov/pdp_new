//
// Created by User on 14.04.2018.
//

#ifndef EMULATOR_ALL_H
#define EMULATOR_ALL_H

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

byte mem[64*1024];  //вся память
word reg[8];    //регистры

int t; //переменная, отвечающая за трассировку

s_byte bb;
s_word ww;

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

struct status PSW;

#define pc reg[7]   //7 регистр используем как прог. каунтер

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


#endif //EMULATOR_ALL_H
