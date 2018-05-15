//
// Created by User on 07.05.2018.
//

#include "all.h"

extern byte mem[64*1024];
extern word reg[8];
extern int t;
extern int full;

extern s_byte bb;
extern s_word ww;
extern byte nn;

extern struct status PSW;

word get_nn(word w) {
    return (word) (w & 077);     //??
}

word get_ss(word w) {
    return (word) (w & 077);     //???
}

byte get_offset(word w) {
    return (byte) (w & 0377);
}

byte b_read (adr a) {
    return mem[a];
}

void b_write (adr a, byte val) {
    if (a < 8) {
        if (PSW.N) {
            reg[a] = 0xFF;
            reg[a] = ((reg[a] << 8) | val);
            //printf("\nIF\n");
            //reg[a] = reg[a] | val;
        }
        else
            reg[a] = val;
    }
    else
        mem[a] = val;
    if (a == out + 2) {
        if (t)
            printf ("   SYMBOL ");
        printf("%c", val);
        if (t)
            printf("\n");
    }
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
    if (a == out + 2) {
        if (t)
            printf ("   SYMBOL ");
        printf("%c", val);
        if (t)
            printf("\n");
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
    printf("\n------TEST BEGIN------\n");
    printf("Reg check:\n");
    printf("R0=%06o,   R2=%06o,   R4=%06o,   SP=%06o\n", reg[0], reg[2], reg[4], reg[6]);
    printf("R1=%06o,   R3=%06o,   R5=%06o,   PC=%06o\n", reg[1], reg[3], reg[5], pc);
    printf("N=%06o,    Z=%06o     V=%06o     C=%06o\n", PSW.N, PSW.Z, PSW.V, PSW.C );
    printf("-------TEST END-------\n");
}

void smart_reg_check(int a, int b) {
    printf("   R%d=%06o ", a, reg[a]);
    printf("R%d=%06o\n", b, reg[b]);
}

void flag_check() {
    printf("---FLAG TEST BEGIN---\n");
    printf("N=%06o,    Z=%06o     V=%06o    C=%06o\n", PSW.N, PSW.Z, PSW.V, PSW.C );
    printf("----FLAG TEST END----\n");
}

void set_zero (int a) {
    if (a == 0)
        PSW.Z = 1;
    else
        PSW.Z = 0;
}

void set_negative (int a) {
    if (a < 0)
        PSW.N = 1;
    else
        PSW.N = 0;
}

void do_halt(word w) {
    if (t)
        printf("HALT");
    reg_check();
    printf("------------------THE END (halt)------------------\n");
    exit(0);
}

void do_add(word w) {
    if (t)
        printf("ADD   ");

    int res;
    int res1;

    struct Data source = take(w, src);
    struct Data dest = take(w, dst);
    //printf("%o %o\n",source, dest);

    ww.u_w = dest.w;
    res1 = ww.s_w;

    ww.u_w = source.w;
    res = ww.s_w;

    res = res + res1;

    dest.w = (word) res;
    w_write(dest.mem_adr, dest.w);

    set_zero(res);
    set_negative(res);      //как сделать carry???
    if (ww.s_w < 0 && res1 < 0 && res > 0)
        PSW.V = 1;
    else if (ww.s_w > 0 && res1 > 0 && res < 0)
        PSW.V = 1;
    else
        PSW.V = 0;
    //flag_check;
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
    }

    set_zero(ww.s_w);
    set_negative (ww.s_w);
    PSW.V = 0;
    //flag_check();
}

void do_sob(word w) {
    if (t)
        printf("SOB   ");

    adr reg_adr = (adr)src_reg(w);
    word nn = get_nn(w);

    reg[reg_adr] --;

    if (reg[reg_adr] > 0) {
        pc = (word) (pc - 2*nn);
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

    PSW.N = 0;
    PSW.Z = 1;
    PSW.V = 0;
    PSW.C = 0;
    //flag_check();
    if (t)
        printf("\n");
}

void do_br(word w) {
    if (t)
        printf("BR to ");

    bb.u_b = get_offset(w);
    pc += 2 * bb.s_b;

    if (t)
        printf("%06o\n", pc);
}

void do_beq(word w) {
    if (t)
        printf("BEQ to ");

    if (PSW.Z) {
        bb.u_b = get_offset(w);
        pc += 2 * bb.s_b;
    }
    if (t)
        printf("%06o\n", pc);
}

void do_tstb(word w) {
    struct Data dest;

    if (B(w)) {
        if (t)
            printf("TSTb ");

        dest = take(w, dst);

        bb.u_b = (byte)dest.w;

        set_zero(bb.s_b);
        set_negative(bb.s_b);
        PSW.C = 0;
        PSW.V = 0;
    }
    else {
        if (t)
            printf("TST ");

        dest = take(w, dst);

        ww.u_w = dest.w;

        set_zero(ww.s_w);
        set_negative(ww.s_w);
        PSW.C = 0;
        PSW.V = 0;
    }
    if (t)
        printf("\n");
    //flag_check();
}

void do_bpl(word w) {
    if (t)
        printf("BPL\n");

    //flag_check();

    if (PSW.N == 0 && B(w) == 1) {
        bb.u_b = get_offset(w);
        printf("%06o", bb.u_b);
        pc += (2 * bb.s_b);
    }
    /*else if (N == 1 && com.B == 1) {
        if(t) {
            printf("BPL ");
        }
        return 0;
    }*/
}

void do_jsr(word w) {
    if (t)
        printf("JSR ");

    struct Data dest = take(w, dst);

    w_write(reg[6], reg[src_reg(w)]);
    reg[6]-= 2;
    reg[src_reg(w)] = pc;
    pc = dest.mem_adr;
    if (t)
        printf("\n");
}

void do_rts(word w) {
    if (t) {
        printf("RTS\n");
    }

    pc = reg[dst_reg(w)];
    //printf("R6 = %06o\n", w_read(reg[6]));
    reg[6] += 2;
    //printf("R6 = %06o\n", w_read(reg[6]));
    //printf("%06o\n", pc);
    reg[dst_reg(w)] = w_read(reg[6]);
    reg[dst_reg(w)] += 2;
    //printf("%06o\n", pc);
}

void do_ror(word w) {
    word flag;
    byte c = PSW.C;
    struct Data dest = take(w, dst);

    if (B(w) == 0) {
        if (t)
            printf("ROR\n");
        PSW.C = (byte) (dest.w & 1);

        //assert(PSW.C & 1 == PSW.C);
        c = (c << 15);

        dest.w = ((dest.w) >> 1) | c;
        ww.u_w = dest.w;
        w_write(dest.mem_adr, dest.w);

        set_negative(ww.s_w);
        set_zero(ww.s_w);
    }
    if (B(w)) {
        if (t)
            printf("RORb\n");
        PSW.C = (byte) (dest.w & 1);

        assert(PSW.C & 1 == PSW.C);
        c = (c << 7);

        dest.w = (((byte)dest.w) >> 1) | c;
        bb.u_b = (byte)dest.w;

        //printf("\nDEST.W = %03o\n", (byte)dest.w);

        b_write(dest.mem_adr, (byte)dest.w);

        set_zero(bb.s_b);
        set_negative(bb.s_b);
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
        {0000400, 0177400, "br",        do_br,       HAS_XX},
        {0001400, 0177400, "beq",       do_beq,      HAS_XX},
        {0105700, 0177700, "tstb",      do_tstb,     HAS_XX},
        {0100000, 0177400, "bpl",       do_bpl,      HAS_XX},
        {0004000, 0177000, "jsr",       do_jsr,      HAS_DD},
        {0000200, 0177770, "rts",       do_rts,      NO_PARAM},
        {0006000, 0077700, "ror",       do_ror,      HAS_DD},
        {0,       0,       "unknown",   do_unknown,  HAS_NN}   //MUST BE THE LAST; последняя команда: функция пробегает весь массив
        // если нет совпадений - выполняется она
};

void run (adr pc0) {
    pc = pc0;   //используем 7 регистр для адресов
    int i;
    int length = sizeof(commands)/ sizeof(commands[0]);
    //printf("%d\n", length);
    while (1) {
        word w = w_read(pc);
        if (t) {
            printf("%06o:%06o   ", pc, w);
        }
        pc += 2;
        for (i = 0; i <= length; i ++) {
            struct Command cmd = commands[i];
            if ((w & cmd.mask) == cmd.opcode) { //проходим весь массив команд
                //printf("%s\n", cmd.name);
                // аргументы
                /*if(cmd.param & HAS_NN) {
                    nn = get_nn(w);     //разобраться с типом возвращаемого значения
                }
                if(cmd.param & HAS_SS) {
                    nn = get_ss(w);     //написать функцию
                }*/
                if (t)
                    printf("COMMAND: ");    //необязательно
                cmd.func(w);
                if (full)
                    reg_check();
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
    //scanf("%ms", &filename);    %ms - размер считываемого слова неизвестен
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

void testmem() {
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