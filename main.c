#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;

byte mem[64*1024];  //вся память
word reg[8];    //регистры

#define pc reg[7]   //7 регистр используем как прог. каунтер

#define NO_PARAM 0
#define HAS_SS 1
#define HAS_DD (1<<1)
#define HAS_XX (1<<2)
#define HAS_NN (1<<3)

byte nn;

word get_nn(word w) {
    return w & 077;
}

word get_ss(word w) {
    return w & 077;
}

#define LO(x) ((x) & 0xFF)
#define HI(x) (((x) >> 8) & 0xFF)

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

void do_halt() {
    printf("THE END\n");
    exit(0);
}

void do_add() {
    printf("ADD\n");
}

void do_mov() {
    printf("MOV\n");
}

void do_sob(){
    w_read(nn);
}

void do_unknown() {
    printf("UNKNOWN\n");
}


struct Command {
    word opcode;
    word mask;
    char * name;
    void (*func)();
    byte param;
} commands[] = {
        {0,       0177777, "halt",      do_halt,     NO_PARAM},   //0xFFFF
        {0010000, 0170000, "mov",       do_mov,      HAS_SS | HAS_DD},
        {0060000, 0170000, "add",       do_add,      HAS_SS | HAS_DD},
        {0077000, 0177000, "sob",       do_sob,      HAS_NN},  //SOB
        {0,       0,       "unknown",   do_unknown,  HAS_NN}   //MUST BE THE LAST; последняя команда: функция пробегает весь массив
        // если нет совпадений - выполняется она
};

void run (adr pc0) {
    pc = pc0;   //используем 7 регистр для адресов
    int i;
    while (1) {
        word w = w_read(pc);
        printf("%06o:%06o\n", pc, w);
        pc += 2;
        for (i = 0; i< 64*1024 ;i ++) {
            struct Command cmd = commands[i];
            if ((w & cmd.mask) == cmd.opcode) { //проходим весь массив команд
                printf("%s ", cmd.name);
                // аргументы
                if(cmd.param & HAS_NN) {
                    nn = get_nn(w);     //разобраться с типом возвращаемого значения
                }
                if(cmd.param & HAS_SS) {
                    nn = get_ss(w);     //написать функцию
                }

                cmd.func();
                break;  //выходим из сравнения с массивом; если нет совпадений - есть последняя команда unknown
            }
        }

    }
}

void load_file() {    //доделать до полноценной работы с файлами
    unsigned int address;
    unsigned int n;
    unsigned int x;
    unsigned int i = 0;

    char* filename = NULL;
    scanf("%ms", &filename);    //%ms - размер считываемого слова неизвестен
    FILE *f; //= stdin;
    f = fopen(filename, "r");

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
    fclose(f);
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

int main() {
    printf("---start testmem---\n");
    testmem();
    printf("---end testmem---\n");
    load_file();
    run(01000);
    return 0;
}