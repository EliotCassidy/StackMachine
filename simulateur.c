#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_LINE 5000
#define MEMORY 5000

int nb_line = 0;
int PC = 0;
int SP = 0;
int STACK[MEMORY] = {0};
int seed = 0;

typedef struct {
    char *operation;
    char code[3];
} Dict;

typedef struct {
    char *name;
    int adress_line;
} Flag;

typedef struct {
    char *flag;
    char *operation;
    char *data;
} Instruction;

typedef struct {
    int data;
    void (*func)(int);
} Execute;


char* get_code(const char *operation);
int convert_line(char *input, Instruction *line, FILE *f, Instruction *instructions, int nb_line);
int isFlag(const char *data, const Flag *flags, const int nb_flags);
int isAllDigits(const char *str);
void freeLine(Instruction *line);
void freeAll(Instruction *, int);
int is_integer(const char *str);

void pop(int);
void ipop(int);
void push(int);
void ipush(int);
void pushval(int);
void jmp(int);
void jnz(int);
void call(int);
void ret(int);
void read(int);
void write(int);
void op(int);
void rnd(int);
void dup(int);
void deb(int);
void halt(int);


Dict operations_codes[] = {
    {"pop", "00"}, {"ipop", "01"}, {"push", "02"}, {"ipush", "03"},
    {"push#", "04"}, {"jmp", "05"}, {"jnz", "06"}, {"call", "07"},
    {"ret", "08"}, {"read", "09"},{"write", "0a"}, {"op", "0b"},
    {"rnd", "0c"}, {"dup", "0d"}, {"deb", "0e"}, {"halt", "63"}
};




int main(int argc, char** argv)
{

    if (argc != 2) {
        printf("WRONG FORMAT >>> %d files where given, 1 expected\n", argc-1);
        exit(1);
    }



    FILE *input;
    input = fopen(argv[1], "r");
    if (!input) {
        printf("MEMORY ERROR >>>\n");
        exit(1);
    }

    char input_line[MAX_LINE];
    Instruction instructions[MAX_LINE];
    int nb_flags = 0;
    Flag flags[MAX_LINE];

    // First pass: Parse assembly code into instructions and collect labels
    while (fgets(input_line, MAX_LINE, input)) {
        Instruction instruct = {NULL, NULL, NULL};
        int is_empty = convert_line(input_line, &instruct, input, instructions, nb_line);
        if (is_empty == 0)
        {
            instructions[nb_line] = instruct;
            if (instruct.flag != NULL)
            {
                flags[nb_flags].name = instruct.flag;
                flags[nb_flags].adress_line = nb_line;
                nb_flags++;
            }
            nb_line++;
        }
    }
    fclose(input);

    FILE *output = fopen("hexa.txt", "w");
    // Second pass: Convert instructions to machine code
    for (int i=0; i<nb_line; i++)
    {
        // Convert operation to hex code
        char *converted_op = get_code(instructions[i].operation);
        if (strcmp(converted_op, "10") == 0)
        {
            printf("OPERATION ERROR >>> %s is not an operation at line %d\n", instructions[i].operation, i+1);
            freeAll(instructions, nb_line);
            fclose(output);
            remove("hexa.txt");
            exit(1);
        }
        fprintf(output, "%s", converted_op);

        // Check if operation expects data
        int is_data = strcmp(converted_op, "01") == 0 || strcmp(converted_op, "03") == 0 || strcmp(converted_op, "08") == 0 || strcmp(converted_op, "0d") == 0 || strcmp(converted_op, "0e") == 0 || strcmp(converted_op, "63" ) == 0;
        
        // Process instruction data (if any)
        if (instructions[i].data != NULL)
        {
            if (is_data) {
                printf("DATA ERROR >>> %s does not take an argument at line %d\n", instructions[i].operation, i+1);
                freeAll(instructions, nb_line);
                fclose(output);
                remove("hexa.txt");
                exit(1);
            } 
            int flag_adress = isFlag(instructions[i].data, flags, nb_flags);
            if (flag_adress == -1)
            {
                if (!is_integer(instructions[i].data))
                {
                    printf("FLAG ERROR >>> %s is not a flag at line %d\n", instructions[i].data, i+1);
                    freeAll(instructions, nb_line);
                    fclose(output);
                    remove("hexa.txt");
                    exit(1);
                }

                if (atoi(instructions[i].data) > 32767 || atoi(instructions[i].data) < -32768)
                {
                    printf("WARING >>> operation produces a value that is not representable in 16 bits\n");
                    int signed_number = (short) atoi(instructions[i].data);
                    unsigned short masked_number = (signed_number) & 0xFFFF;
                    fprintf(output, " %04x\n", masked_number);
                }
                else {
                    unsigned short masked_number = (atoi(instructions[i].data) & 0xFFFF);
                    fprintf(output, " %04x\n", masked_number);
                }

            }
            else
            {
                int adress = flag_adress-i-1;
                unsigned short masked_number = (adress & 0xFFFF);
                fprintf(output, " %04x\n", masked_number); // Convert to 4 Hexa
            }
        }
        else
        {
            if (!is_data) {
                printf("DATA ERROR >>> %s takes an argument at line %d\n", instructions[i].operation, i+1);
                freeAll(instructions, nb_line);
                fclose(output);
                remove("hexa.txt");
                exit(1);
            }
            if (i == nb_line-1)
            {
                fprintf(output, " 0000");
            }
            else
            {
                fprintf(output, " 0000\n");
            }
            
        }
    }

    fclose(output);
    freeAll(instructions, nb_line);

    // Third pass: Read the machine code and put in list
    FILE *machine_code = fopen("hexa.txt", "r");
    if (machine_code == NULL)
    {
        printf("Couldn't open file\n");
        exit(1);
    }

    char execute_line[MAX_LINE];
    int nb_execute = 0;
    Execute execute_list[MAX_LINE];

    // Map operation codes to their corresponding functions
    void (*functions[16])(int) = { pop, ipop, push, ipush, pushval, jmp, jnz, call, ret, read, write, op, rnd, dup, deb, halt};

    // Load machine code into executable format
    while (fgets(execute_line, MAX_LINE, machine_code)) {
        Execute execute;
        // Extract operation code (first 2 hex digits)
        char op[3];
        strncpy(op, execute_line, 2);
        op[2] = '\0';
        int value = (int)strtol(op, NULL, 16);
        if (value == 99)
        {
            value = 15;
        }
        if (value > 15)
        {
            printf("op error\n");
            fclose(machine_code);
            exit(1);
        }
        execute.func = functions[value]; 

        // Extract data value (next 4 hex digits)
        char data[5];
        strncpy(data, &execute_line[3], 4);
        data[4] = '\0';
        int val = (int)strtol(data, NULL, 16);
        int signed_val = (int)(short)val;
        execute.data = signed_val;
        execute_list[nb_execute] = execute;
        nb_execute++;
    }
    fclose(machine_code);
    Execute null_execute = {0, NULL};
    execute_list[nb_execute] = null_execute;

    // Execute instructions until program end
    while (execute_list[PC].func != NULL)
    {
        Execute exe = execute_list[PC];
        exe.func(exe.data);
        PC++;
    }
    printf("Program terminated >>> Standard exit 0. Please put halt to stop graciously\n");
    exit(0);
}

int is_integer(const char *str) {
    if (!str || *str == '\0') return 0;  // Empty string check

    char *endptr;
    strtol(str, &endptr, 10);

    // Ensure the entire string was a valid integer
    return *endptr == '\0';
}

char* get_code(const char *operation)
{
    for (int i = 0; i < sizeof(operations_codes) / sizeof(Dict); i++)
    {
        if (strcmp(operations_codes[i].operation, operation) == 0)
        {
            return operations_codes[i].code;
        }
    }
    return "10"; // Error handling
}

// FLAG: Operation #VAL     ; Comment
int convert_line(char *input, Instruction *line, FILE *f, Instruction *instructions, int nb_line)
{
    // Check if comments
    for (int i = 0, n = strlen(input); i < n; i++)
    {
        if (input[i] == ';')
        {
            if (i == strlen(input)-1)
            {
                input[i] = '\0';
            }
            else
            {
                input[i] = '\n';
                input[i+1] = '\0';
            }

        }
    }
    int is_empty = 1;
    // White line/spaces/tabs
    for (int i = 0; i < strlen(input); i++)
    {
        if (input[i] != ' ' && input[i] != '\t' && input[i] != '\0' && input[i] != '\n')
        {
            is_empty = 0;
            break;
        }
    }

    if (is_empty == 1)
    {
        return 1;
    }

    int tick = 0;
    int i = 0;
    while (input[i] != '\0' && tick == 0)
    {
        if (input[i] == ':') // We only consider the first ':' per sentence
        {
            tick = i;
            if (tick == 0)
            {
                printf("NO FLAG >>> %s\n", input); // Sentence is in the format ': TEXT'
                fclose(f);
                exit(1);
            }

            if (isalpha(input[0]) == 0)
            {
                printf("WRONG FORMAT OF FLAG >>> %s\n", input);
                fclose(f);
                exit(1);                
            }
            for (int j = 0; j < i; j++)
            {
                if (isalpha(input[j]) == 0 && isdigit(input[j]) == 0 && input[j] != '_')
                {
                printf("WRONG FORMAT OF FLAG >>> %s\n", input);
                fclose(f);
                exit(1);                                 
                }
            }

            line->flag = malloc((tick+1) * sizeof(char)); // We add the /0
            strncpy(line->flag, input, i);
            line->flag[tick] = '\0';
        }
        i++;
    }


    if (tick != 0)
    {
        i = tick+1;
    }
    else
    {
        i = 0;
    }

    // Clear spaces/tabs before operation
    while (input[i] != '\0' && (input[i] == ' ' || input[i] == '\t'))
    {
        i++;
    }


    tick = i;

    // Count operation size
    while (input[i] != '\0' && (input[i] != ' ' && input[i] != '\n'))
    {
        i++;
    }

    if (tick == i)
    {
        freeLine(line);
        freeAll(instructions, nb_line);
        printf("NO OPERATION >>> %s\n", input);
        fclose(f);
        exit(1);
    }
    line->operation = malloc((i-tick+1) * sizeof(char)); // Add /0
    strncpy(line->operation, input+tick, i-tick);
    line->operation[i-tick] = '\0';


    if (input[i] == ' ' && (input[i+1] != ' ' && input[i+1] != '\0' && input[i+1] != '\n')) // To take care if space after operation
    {
        int j = i+1;
        // Place of space after data before space
        while (input[j] != '\0' && (input[j] != ' ' && input[j] != '\t' && input[j] != '\n'))
        {
            j++;
        }

        // read 1000        blabla ; This should create an error
        for (int k = j; k < strlen(input); k++)
        {
            if (input[k] != '\0' && input[k] != '\t' && input[k] != ' ' && input[k] != '\n')
            {
                freeLine(line);
                freeAll(instructions, nb_line);
                printf("COMPILE ERROR >>>>%s\n", input);
                fclose(f);
                exit(1);
            }
        }
        if (j != i+1)
        {
            j--;
            line->data = malloc((j-i+1) * sizeof(char));
            strncpy(line->data, input+i+1, j-i);
            line->data[j-i] = '\0';
        }

    }
    return 0;
}

int isFlag(const char *data, const Flag *flags, const int nb_flags)
{
    for (int i = 0; i < nb_flags; i++)
    {
        if (strcmp(data, flags[i].name) == 0)
        {
            return flags[i].adress_line;
        }
    }
    return -1;
}

int isAllDigits(const char *str)
{
    while (*str) {
        if (!isdigit((unsigned char)*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}


void halt(int i)
{
    exit(0);
}

void op(int i)
{
    if (i > 15 || ((i == 15 || i == 9) && SP < 0) || ((i != 15 && i != 9) && SP < 2))
    {
        printf("Operation code error\n");
        exit(1);
    }
    switch (i)
    {
        case 0:
            SP--;
            STACK[SP-1] = STACK[SP-1] == STACK[SP] ? 1 : 0;
            break;
        case 1:
            SP--;
            STACK[SP-1] = STACK[SP-1] != STACK[SP] ? 1 : 0;
            break;
        case 2:
            SP--;
            STACK[SP-1] = STACK[SP-1] >= STACK[SP] ? 1 : 0;
            break;
        case 3:
            SP--;
            STACK[SP-1] = STACK[SP-1] <= STACK[SP] ? 1 : 0;
            break;
        case 4:
            SP--;
            STACK[SP-1] = STACK[SP-1] > STACK[SP] ? 1 : 0;
            break;
        case 5:
            SP--;
            STACK[SP-1] = STACK[SP-1] < STACK[SP] ? 1 : 0; 
            break;
        case 6:
            SP--;
            if (abs(STACK[SP-1] | STACK[SP]) > 32767)
            {
                printf("WARING >>> operation produces a value that is not representable in 16 bits\n");
                STACK[SP-1] = (short) (STACK[SP-1] | STACK[SP]);
            }
            else {
                STACK[SP-1] = (STACK[SP-1] | STACK[SP]);
            }
            break;
        case 7:
            SP--;
            if (abs(STACK[SP-1] ^ STACK[SP]) > 32767)
            {
                printf("WARING >>> operation produces a value that is not representable in 16 bits\n");
                STACK[SP-1] = (short) (STACK[SP-1] ^ STACK[SP]);
            }
            else {
                STACK[SP-1] = (STACK[SP-1] ^ STACK[SP]);
            }
            break;
        case 8:
            SP--;
            if (abs(STACK[SP-1] & STACK[SP]) > 32767)
            {
                printf("WARING >>> operation produces a value that is not representable in 16 bits\n");
                STACK[SP-1] = (short) (STACK[SP-1] & STACK[SP]);
            }
            else {
                STACK[SP-1] = (STACK[SP-1] & STACK[SP]);
            }
            break;
        case 9:
            STACK[SP-1] =~ STACK[SP-1];
            break;
        case 10:
            SP--;
            if (abs(STACK[SP-1] + STACK[SP]) > 32767)
            {
                printf("WARING >>> operation produces a value that is not representable in 16 bits\n");
                STACK[SP-1] = (short) (STACK[SP-1] + STACK[SP]);
            }
            else {
                STACK[SP-1] = (STACK[SP-1] + STACK[SP]);
            }
            break;
        case 11:
            SP--;
            if (abs(STACK[SP-1] - STACK[SP]) > 32767)
            {
                printf("WARING >>> operation produces a value that is not representable in 16 bits\n");
                STACK[SP-1] = (short) (STACK[SP-1] - STACK[SP]);
            }
            else {
                STACK[SP-1] = (STACK[SP-1] - STACK[SP]);
            }
            break;
        case 12:
            SP--;
            if (abs(STACK[SP-1] * STACK[SP]) > 32767)
            {
                printf("WARING >>> operation produces a value that is not representable in 16 bits\n");
                STACK[SP-1] = (short) (STACK[SP-1] * STACK[SP]);
            }
            else {
                STACK[SP-1] = (STACK[SP-1] * STACK[SP]);
            }
            break;
        case 13:
            if (STACK[SP] == 0) {
                printf("Division by 0\n");
                exit(1);
            }
            SP--;
            STACK[SP-1] = STACK[SP-1] / STACK[SP];
            break;
        case 14:
            if (STACK[SP] < 0) {
                printf("Modulo by 0\n");
                exit(1);
            }
            SP--;
            STACK[SP-1] = STACK[SP-1] % STACK[SP];
            break;
        case 15:
            STACK[SP-1] = -STACK[SP-1];
            break;
    }
}

void push(int x)
{
    if (SP > MEMORY)
    {
        printf("Overflow\n");
        exit(1);
    }
    if (x >= MEMORY || x < 0)
    {
        printf("MEMORY ERROR >>> Address %d out of bounds\n", x);
        exit(1);
    }
    STACK[SP] = STACK[x];
    SP++;
}

void ipush(int i)
{
    if (SP == 0)
    {
        printf("SP underflow\n");
        exit(1);
    }
    int addr = STACK[SP-1];
    if (addr >= MEMORY || addr < 0)
    {
        printf("MEMORY ERROR >>> Address %d out of bounds\n", addr);
        exit(1);
    }
    STACK[SP-1] = STACK[addr];
}

void pushval(int i)
{
    if (SP >= MEMORY)
    {
        printf("SP overflow\n");
        exit(1);
    }
    if (abs(i) > 32767)
    {
        printf("WARNING >>> push# %d has an argument that is not representable on 16 bits\n", i);
        i = (short) i;
    }
    STACK[SP] = i;
    SP++;
}

void jmp(int adr)
{
    if (PC + adr >= nb_line  || PC + adr < -1) {
        printf(">>>> Jump error\n");
        exit(1);
    }
    PC = PC + adr;
}

void jnz(int adr)
{
    if (SP == 0)
    {
        printf(">>>> Stack underflow\n");
        exit(1);
    }
    SP--;
    int x = STACK[SP];
    if (x != 0)
    {
        if (PC + adr >= nb_line  || PC + adr < -1) {
            printf(">>>> Jump error\n");
            exit(1);
        }
        PC = PC + adr;
    }
}

void call(int adr)
{
    if (SP >= MEMORY)
    {
        printf(">>>> Stack overflow\n");
        exit(1);
    }
    if (PC + adr >= nb_line  || PC + adr < -1) {
        printf(">>>> Jump error\n");
        exit(1);
    }
    STACK[SP] = PC;
    SP++;
    PC = PC + adr;
}

void ret(int i)
{
    if (SP == 0)
    {
        exit(1);
    }
    SP--;
    if (STACK[SP] >= nb_line || STACK[SP] < -1) {
        printf(">>>> Jump error\n");
        exit(1);
    }
    PC = STACK[SP];
}

void read(int x)
{
    int i = 0;
    scanf("%d", &i);
    if (x >= MEMORY || x < -1)
    {
        printf("Stack overflow\n");
        exit(1);
    }
    if (abs(i) > 32767)
    {
        printf("WARNING >>> read has got an argument that is not representable on 16 bits : %d\n", i);
        i = (short) i;
    }
    
    if (x == -1) {
        if (SP >= MEMORY) {
            printf("Stack overflow\n");
            exit(1);
        }
        STACK[SP] = i;
        SP++;
    }
    else {
        STACK[x] = i;
    }
}

void write(int x)
{
    if (x >= MEMORY)
    {
        printf("Memory overflow\n");
        exit(1);
    }
    if (x < -1) {
        printf("Memory underlow\n");
        exit(1);        
    }
    if (x == -1) {
        if (SP <= 0) {
            printf("Stack underflow\n");
            exit(1);
        }
        printf("%d\n", STACK[SP-1]);
    }
    else {
        printf("%d\n", STACK[x]);
    }
}

void rnd(int x)
{
    if (x < 1) {
        printf("RANGE ERROR >>> %d\n", x);
        exit(1);
    }
    srand(time(NULL) + seed);
    seed += 1;
    int r = rand() % x;
    if (abs(r) > 32767)
    {
        printf("WARNING >>> rnd hgenerated a number that is not representable on 16 bits : %d\n", r);
        r = (short) r;
    }
    if (SP >= MEMORY)
    {
        printf("Stack overflow\n");
        exit(1);
    }
    STACK[SP] = r;
    SP++;
}

void dup(int i)
{
    if (SP >= MEMORY)
    {
        printf("Stack overflow\n");
        exit(1);
    }
    STACK[SP] = STACK[SP-1];
    SP++;
}

void pop(int x)
{
    if (SP <= 0)
    {
        printf("Stack underflow\n");
        exit(1);
    }
    if (x >= MEMORY)
    {
        printf("Memory overflow\n");
        exit(1);
    }
    SP--;
    STACK[x] = STACK[SP];
}

void ipop(int i)
{
    if (SP <= 1 || STACK[SP-1]>= MEMORY)
    {
        printf("Memory error\n");
        exit(1);
    }
    STACK[STACK[SP-1]] = STACK[SP-2];
    SP = SP - 2;
}

void deb(int i)
{
    printf("[ ");
    for (int i = 0; i < SP; i++) {
        printf("%d, ", STACK[i]);
    }
    printf("]\n");
}

void freeLine(Instruction *line)
{
    if (line->operation)
    {
        free(line->operation);
    }
    if (line->data)
    {
        free(line->data);
    }
    if (line->flag)
    {
        free(line->flag);
    }
}


void freeAll(Instruction *instructions, int n)
{
    for (int i = 0; i < n; i++)
    {
        freeLine(&instructions[i]);
    }
}