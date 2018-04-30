#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "all.h"

byte nn;

word get_nn(word w) {
    return w & 077;     //??
}

word get_ss(word w) {
    return w & 077;     //???
}

byte get_offset(word w) {
    return w & 0377;
}

byte b_read (adr a) {
    return mem[a];
}

void b_write (adr a, byte val) {
    if (a < 8)
        reg[a] = val;
    else
        mem[a] = val;
}

void w_write (adr a, word val) {
    if (a < 8)
        reg[a] = val;
    else {
        assert(a % 2 == 0);
        mem[a] = (byte) (val & 0xFF);
        mem[a + 1] = (byte) ((val >> 8) & 0xFF); //в следующий байт записываем вторую часть слова
            // (для этого двигаем его)
    }
}

word w_read (adr a) {
    assert(a % 2 == 0);
    word w = (word)(mem[a + 1]);
    w = w << 8; //сдвигаем, чтобы записать первый байт на место нулей
    w = w | (word)(mem[a]);
    return w;
}

void reg_check() {
    printf("\n---TEST BEGIN---\n");
    printf("Reg check:\n");
    printf("R0=%06o,   R2=%06o,   R4=%06o,   R6=%06o\n", reg[0], reg[2], reg[4], reg[6]);
    printf("R1=%06o,   R3=%06o,   R5=%06o,   PC=%06o\n", reg[1], reg[3], reg[5], pc);
    printf("N=%06o,    Z=%06o     V=%06o    C=%06o\n", PSW.N, PSW.Z, PSW.V, PSW.C );
    printf("---TEST END---\n");
}

void smart_reg_check(int a, int b) {
    printf("   R%d=%06o ", a, reg[a]);
    printf("R%d=%06o\n", b, reg[b]);
}

void flag_check() {
    printf("---FLAG TEST BEGIN---\n");
    printf("N=%06o,    Z=%06o     V=%06o    C=%06o\n", PSW.N, PSW.Z, PSW.V, PSW.C );
    printf("---FLAG TEST END---\n");
}

void set_flags (int res) {
    if (res == 0)
        PSW.Z = 1;
    else
        PSW.Z = 0;

    if (res < 0)
        PSW.N = 1;
    else
        PSW.N = 0;

    if (((res>>16) & 1) == 1)
        PSW.C = 1;  //carry появляется с минусом
    else
        PSW.C = 0;
}

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
            if (B == 0) {
                res.w = w_read(res.mem_adr);
                reg[i] += 2;
            }
            if (B && i != 7) {
                res.w = b_read(res.mem_adr);
                reg[i] ++;
            }
            return res;
        case 3:
            if (t)
                printf("@(R%o)+, ", i);
            res.mem_adr = w_read(reg[i]);
            if (B == 0) {
                res.w = w_read(res.mem_adr);
                reg[i] += 2;
            }
            if (B && i != 7) {
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

void do_halt(word w) {
    reg_check();
    printf("THE END (halt)\n");
    exit(0);
}

void do_add(word w) {
    if (t)
        printf("ADD   ");

    int res;

    struct Data source = take(w, src);
    struct Data dest = take(w, dst);
            //printf("%o %o\n",source, dest);

    ww.u_w = dest.w;
    res = ww.s_w;

    ww.u_w = source.w;
    res = res + ww.s_w;

    dest.w = (word) res;
    w_write(dest.mem_adr, dest.w);

    set_flags(res);
    if (t)
        flag_check;

    if (t)
        smart_reg_check(src_reg(w), dst_reg(w));
}

void do_mov(word w) {
    if (t) {
        if (B(w) == 0)
            printf("MOV   ");
        if (B(w))
            printf("MOVb  ");
    }
    struct Data source = take(w, src);
    struct Data dest = take(w, dst);
    //dst_push(w, source.w);
    if (t)
        printf("   [%06o]:%06o\n", source.mem_adr, source.w);
   // dest.w = source.w;

    if (B(w) == 0) {
        ww.u_w = source.w;
        w_write(dest.mem_adr, source.w);
    }
    if (B(w)) {
        if (dest.mem_adr > 7) {
            ww.u_w = source.w;
            b_write(dest.mem_adr, (byte) source.w);
        }

        else {
            bb.u_b = (byte) source.w;
            ww.s_w = bb.s_b;
            source.w = ww.u_w;
            w_write(dest.mem_adr, source.w);
        }
        //source.w = negative_byte((byte)source.w);
    }
    set_flags(ww.s_w);
    flag_check();
}

void do_sob(word w) {
    if (t)
        printf("SOB   ");

    adr reg_adr = (adr)src_reg(w);
    word nn = get_nn(w);

    reg[reg_adr] --;
    if (reg[reg_adr] > 0) {
        pc = pc - 2*nn;
    }
    if (t)
        printf("R%o, %06o\n", reg_adr, pc);
}

void do_clear(word w) {
    if (t)
        printf("CLR   ");

    struct Data dest = take(w, dst);
    if (B(w) == 0)
        w_write(dest.mem_adr, 0);
    if (B(w))
        b_write(dest.mem_adr, 0);

    set_flags(0);
    if (t)
        flag_check();

    printf("\n");
}

void do_beq(word w) {
    if (t)
        printf("BEQ\n");

    if (PSW.Z) {
        bb.u_b = get_offset(w);
        pc += 2 * bb.s_b;
    }
}

void do_unknown(word w) {
    printf("ha, loh! UNKNOWN function\n");
}

struct Command {
    word opcode;
    word mask;
    char * name;
    void (*func)(word w);
    byte param;
} commands[] = {
        {0,       0177777, "halt",      do_halt,     NO_PARAM},   //0xFFFF
        {0010000, 0170000, "mov",       do_mov,      HAS_SS | HAS_DD},
        {0110000, 0170000, "movb",      do_mov,      HAS_SS | HAS_DD},
        {0060000, 0170000, "add",       do_add,      HAS_SS | HAS_DD},
        {0077000, 0177000, "sob",       do_sob,      HAS_NN},  //SOB
        {0005000, 0177700, "clr",       do_clear,    HAS_DD},
        {0001400, 0177400, "beq",       do_beq,      HAS_XX},
        {0,       0,       "unknown",   do_unknown,  HAS_NN}   //MUST BE THE LAST; последняя команда: функция пробегает весь массив
        // если нет совпадений - выполняется она
};

void run (adr pc0) {
    pc = pc0;   //используем 7 регистр для адресов
    int i;
    while (1) {
        word w = w_read(pc);
        if (t) {
            printf("%06o:%06o   ", pc, w);
        }
        pc += 2;
        for (i = 0; i < 64*1024; i ++) {
            struct Command cmd = commands[i];
            if ((w & cmd.mask) == cmd.opcode) { //проходим весь массив команд
                //printf("%s\n", cmd.name);
                // аргументы
                if(cmd.param & HAS_NN) {
                    nn = get_nn(w);     //разобраться с типом возвращаемого значения
                }
                if(cmd.param & HAS_SS) {
                    nn = get_ss(w);     //написать функцию
                }
                printf("COMMAND: ");    //необязательно
                cmd.func(w);
                //reg_check();
                //printf("\n");   //просто для удобства делает пустую строчку между комндами
                break;  //выходим из сравнения с массивом; если нет совпадений - есть последняя команда unknown
            }
        }
    }
}

void load_file(FILE* f) {    //доделать до полноценной работы с файлами
    unsigned int address;
    unsigned int n;
    unsigned int x;
    unsigned int i = 0;

    //char* filename = NULL;
    //scanf("%ms", &filename);    //%ms - размер считываемого слова неизвестен
    //FILE *f; //= stdin;
    //f = fopen(filename, "r");

    if (f == NULL) {
          perror("file");
          exit(1);
    }
    while (fscanf(f, "%x%x", &address, &n) == 2) {
        for(i = 0; i < n; i ++) {
            fscanf(f, "%x", &x);
            b_write((adr)(address + i), (byte)x);
        }
    }
}

void mem_dump(adr start, word n) {
    assert(n % 2 == 0);
    int i = 0;
    word w;

    for (i = 0; i < n; i += 2)
    {
        w = w_read((adr)(start + i));
        printf("%06o : %06o\n", start + i, w);
    }
}

void f_mem_dump(adr start, word n, FILE* f) {
    assert(n % 2 == 0);
    int i = 0;
    word w;

    for (i = 0; i < 01000 ; i += 2) {       //печать данных, вносимых ранее, чем 1000 адрес
        w = w_read((adr)(i));
        if (w != 0) {
            fprintf(f, "%06o : %06o\n", i, w);
        }
    }

    fprintf(f, "---------------\n");

    for (i = 0; ; i += 2) {
        w = w_read((adr)(start + i));
        fprintf(f, "%06o : %06o\n", start + i, w);
        //fprintf(f, "%o %o\n", src_mode(w), src_reg(w));
        if (w == 000000)
            break;
    }
}

void testmem(){
    byte b0, b1;
    word w;
    b0 = 0x0a;
    b1 = 0x0b;
    b_write(2, b0);
    b_write(3, b1);
    w = w_read(2);
    printf("%04x = %02x%02x\n", w, b1, b0);

    w = 0x0c0d;
    w_write(4, w);
    b0 = b_read(4);
    b1 = b_read(5);
    printf("%04x = %02x%02x\n", w, b1, b0);

    assert(b1 == 0x0c);
    assert(b0 == 0x0d);
}

int main(int argc, char **argv) {

    if (strcmp(argv[1], "-t") == 0) //режим трассировки включен
        t = 1;
    if (strcmp(argv[1], "-q") == 0) //режим трассировки выключен
        t = 0;

//    //printf("---start testmem---\n");
//    testmem();
//    printf("---end testmem---\n");

    FILE* f = fopen(argv[2], "r");
    load_file(f);
    fclose(f);

    f = fopen(argv[3], "w");
    f_mem_dump(01000, 0100, f);
    fclose(f);

    run(01000);
    return 0;
}