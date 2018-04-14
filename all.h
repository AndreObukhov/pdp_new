//
// Created by User on 14.04.2018.
//

#ifndef EMULATOR_ALL_H
#define EMULATOR_ALL_H

typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;

byte mem[64*1024];  //вся память
word reg[8];    //регистры
int t; //переменная, отвечающая за трассировку

#define pc reg[7]   //7 регистр используем как прог. каунтер

#define NO_PARAM 0
#define HAS_SS 1
#define HAS_DD (1<<1)
#define HAS_XX (1<<2)
#define HAS_NN (1<<3)

#define LO(x) ((x) & 0xFF)
#define HI(x) (((x) >> 8) & 0xFF)


#endif //EMULATOR_ALL_H
