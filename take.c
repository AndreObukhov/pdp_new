//
// Created by User on 07.05.2018.
//

#include "all.h"

struct Data take(word w, int a) {
    struct Data res;
    assert(a == src || a == dst);
    int i;
    int mode;
    if (a == src) {
        i = src_reg(w);
        mode = src_mode(w);
    }
    else {
        i = dst_reg(w);
        mode = dst_mode(w);
    }

    int B = B(w);

    switch (mode) {
        case 0:
            if (t)
                printf("R%o, ", i);
            res.mem_adr = (adr)i;
            res.w = reg[i];
            return res;
        case 1:
            if (t)
                printf( "@R%o, ", i);
            res.mem_adr = reg[i];
            if (B == 0)
                res.w = w_read(res.mem_adr);
            if (B)
                res.w = b_read(res.mem_adr);
            return res;
        case 2:
            if (t) {
                if (i == 7)
                    printf("#%o, ", w_read(pc));     //печать с решеточкой:)
                else
                    printf("(R%o)+, ", i);
            }
            res.mem_adr = reg[i];
            if (B == 0 || i == 7) {
                res.w = w_read(res.mem_adr);
                reg[i] += 2;
            }
            else {
            //if (B && i != 7) {
                res.w = b_read(res.mem_adr);
                reg[i] ++;
            }
            return res;
        case 3:
            if (t)
                printf("@(R%o)+, ", i);
            res.mem_adr = w_read(reg[i]);
            if (i == 7 || B == 0) {
                res.w = w_read(res.mem_adr);
                reg[i] += 2;
                //printf(" mode 3 := @#%o\n", res.w);     //ubrat
            }
            else {
            //if (B && i != 7) {
                res.w = b_read(res.mem_adr);
                reg[i] ++;
            }
            return res;
        case 4:
            if (t)
                printf("-(R%o), ", i);
            if (B == 0) {
                reg[i] -= 2;
                res.mem_adr = reg[i];
                res.w = w_read(res.mem_adr);
            }
            if (B && i != 7) {
                reg[i] --;
                res.mem_adr = reg[i];
                res.w = b_read(res.mem_adr);
            }
            return res;
        case 5:
            if (t)
                printf("-@(R%o), ", i);
            if (B == 0) {
                res.mem_adr = w_read(reg[i]);
                res.w = w_read(res.mem_adr);
            }
            if (B && i != 7)
            {
                reg[i] --;
                res.mem_adr = w_read(reg[i]);
                res.w = b_read(res.mem_adr);
            }
            return res;
        default:        //mode 6, 7 ???
            printf("Ha, loh (take_src) \n");
            break;
    }
}
