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

byte b_read (adr a) {
    return mem[a];
}

void b_write (adr a, byte val) {
    mem[a] = val;
}

void w_write (adr a, word val) {
    assert(a % 2 == 0);

    mem[a] = (byte)(val & 0xFF);
    mem[a + 1] = (byte)((val >> 8) & 0xFF); //в следующий байт записываем вторую часть слова (для этого двигаем его)
}

word w_read (adr a) {
    assert(a % 2 == 0);
    word w = (word)(mem[a + 1]);
    w = w << 8; //сдвигаем, чтобы записать первый байт на место нулей
    w = w | (word)(mem[a]);
    return w;
}

void reg_check() {
    printf("Reg check:\n");
    printf("R0=%06o,   R2=%06o,   R4=%06o,   R6=%06o\n", reg[0], reg[2], reg[4], reg[6]);
    printf("R1=%06o,   R3=%06o,   R5=%06o,   PC=%06o\n", reg[1], reg[3], reg[5], pc);
}

word take_src(word w) {
    word mem_adr;
    int i = src_reg(w);

    switch  (src_mode(w)) {
        case 0:
            if (t)
                printf("R%o, ", i);
            return reg[i];
        case 1:
            if (t)
                printf( "@R%o, ", i);
            mem_adr = reg[i];
            return w_read(mem_adr);
        case 2:
            if (t)
                printf("(R%o)+, ", i);
            mem_adr = reg[i];
            reg[i] += 2;
            return w_read(mem_adr);
        case 3:
            if (t)
                printf("@(R%o)+, ", i);
            mem_adr = w_read(reg[i]);
            reg[i] += 2;
            return w_read(mem_adr);
        case 4:
            if (t)
                printf("-(R%o), ", i);
            reg[i] -= 2;
            mem_adr = reg[i];
            return w_read(mem_adr);
        case 5:
            if (t)
                printf("-@(R%o), ", i);
            reg[i] -= 2;
            mem_adr = w_read(reg[i]);
            return w_read(mem_adr);
        default:        //mode 6, 7 ???
            printf("Ha, loh (take_src) \n");
            break;
    }
}

word take_dst(word w) {
    word mem_adr;
    int i = dst_reg(w);

    switch (dst_mode(w)) {
        case 0:
            return reg[i];
        case 1:
            mem_adr = reg[i];
            return w_read(mem_adr);
        case 2:
            mem_adr = reg[i];
            reg[i] += 2;
            return w_read(mem_adr);
        case 3:
            mem_adr = w_read(reg[i]);
            reg[i] += 2;
            return w_read(mem_adr);
        case 4:
            reg[i] -= 2;
            mem_adr = reg[i];
            return w_read(mem_adr);
        case 5:
            reg[i] -= 2;
            mem_adr = w_read(reg[i]);
            return w_read(mem_adr);
        default:        //mode 6, 7 ???
            return 0;
    }
}

void dst_push(word w, word result) {
    word mem_adr;
    int i = dst_reg(w);

    switch (dst_mode(w)) {
        case 0:
            if (t)
                printf("R%o\n", i);
            reg[i] = result;
            break;
        case 1:
            if (t)
                printf("@R%o\n", i);
            mem_adr = reg[i];
            w_write(mem_adr, result);
            break;
        case 2:
            if (t)
                printf("(R%o)+\n", i);
            mem_adr = reg[i];
            reg[i] += 2;
            w_write(mem_adr, result);
            break;
        case 3:
            if (t)
                printf("@(R%o)+\n", i);
            mem_adr = w_read(reg[i]);
            reg[i] += 2;
            w_write(mem_adr, result);
            break;
        case 4:
            if (t)
                printf("-(R%o)\n", i);
            reg[i] -= 2;
            mem_adr = reg[i];
            w_write(mem_adr, result);
            break;
        case 5:
            if (t)
                printf("@-(R%o)\n", i);
            reg[i] -= 2;
            mem_adr = w_read(reg[i]);
            w_write(mem_adr, result);
            break;
        default:        //mode 6, 7 ???
            printf("ha, loh!");
    }
}

void do_halt(word w) {
    printf("\n---TEST BEGIN---\n");
    if (t) {
        reg_check();
        printf("---TEST END---\n");
        printf("THE END (halt)\n");
    }
    exit(0);
}

void do_add(word w) {
    //reg[0] = 1;
    printf("ADD ");

    word source = take_src(w);
    word dest = take_dst(w);
            //printf("%o %o\n",source, dest);
    dest = dest + source;
    dst_push(w, dest);
    //printf("%o %o\n", reg[0], reg[1]);
}

void do_mov(word w) {
    printf("MOV ");

    word source = take_src(w);
    dst_push(w, source);
}

void do_sob(word w) {
    printf("SOB \n");

    adr reg_adr = src_reg(w);
    word nn = get_nn(w);

    reg[reg_adr] --;
    if (reg[reg_adr] > 0)
        pc = pc - 2*nn;
}

void do_clear(word w) {
    printf("CLR ");

    take_dst(w);
    dst_push(w, 0);
}

void do_unknown(word w) {
    printf("UNKNOWN\n");
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
        {0060000, 0170000, "add",       do_add,      HAS_SS | HAS_DD},
        {0077000, 0177000, "sob",       do_sob,      HAS_NN},  //SOB
        {0005000, 0177700, "clr",       do_clear,    HAS_DD},
        {0,       0,       "unknown",   do_unknown,  HAS_NN}   //MUST BE THE LAST; последняя команда: функция пробегает весь массив
        // если нет совпадений - выполняется она
};

void run (adr pc0) {
    pc = pc0;   //используем 7 регистр для адресов
    int i;
    while (1) {
        word w = w_read(pc);
        printf("%06o:%06o   ", pc, w);
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
                cmd.func(w);
                printf("\n");   //просто для удобства делает пустую строчку между комндами
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

    for (i = 0; ; i += 2)
    {
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

    //mem_dump(1000)

    f = fopen(argv[3], "w");
    f_mem_dump(01000, 0100, f);
    fclose(f);

    run(01000);
    return 0;
}