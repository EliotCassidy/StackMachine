// Ce fichier traduit un code assembleur passé en ligne de commande en un code machine

// TODO : Free all VALGRIND
// Put the compilation not in main but seperate function
// Verify that first char is char and that it is A-Z a-z 0-9 "_" only
// Accept empty lines
// accpet comments
// Only 3 octets for the data !!!
// verify that no file is created if error
// Only print out error line


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 5000
#define MEMORY 5000

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


char* get_code(const char *operation);
void convert_line(const char *input, Instruction *line);
int isFlag(const char *data, const Flag *flags, const int nb_flags);
int isAllDigits(const char *str);

Dict operations_codes[] = {
    {"pop", "00"}, {"ipop", "01"}, {"push", "02"}, {"ipush", "03"},
    {"push#", "04"}, {"jmp", "05"}, {"jnz", "06"}, {"call", "07"},
    {"ret", "08"}, {"read", "09"},{"write", "0a"}, {"op", "0b"},
    {"rnd", "0c"}, {"dup", "0d"}, {"halt", "63"}
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

    int nb_line = 0;
    char input_line[MAX_LINE];
    Instruction instructions[MAX_LINE];
    int nb_flags = 0;
    Flag flags[MAX_LINE];


    while (fgets(input_line, MAX_LINE, input)) {
        Instruction instruct = {NULL, NULL, NULL};
        convert_line(input_line, &instruct);
        instructions[nb_line] = instruct;
        if (instruct.flag != NULL)
        {
            flags[nb_flags].name = instruct.flag;
            flags[nb_flags].adress_line = nb_line;
            nb_flags++;
        }
        nb_line++;
    }
    fclose(input);

    FILE *output;
    output = fopen("hexa.txt", "w");

    for (int i=0; i<nb_line; i++)
    {
        char *converted_op = get_code(instructions[i].operation);
        if (strcmp(converted_op, "10") == 0)
        {
            printf("OPERATION ERROR >>> %s is not an operation at line %d\n", instructions[i].operation, i+1);
            remove("hexa.txt");
            exit(1);
        }
        fprintf(output, "%s", converted_op);

        if (instructions[i].data != NULL)
        {
            int flag_adress = isFlag(instructions[i].data, flags, nb_flags);
            if (flag_adress == -1)
            {
                if (isAllDigits(instructions[i].data) == 0)
                {
                    printf("FLAG ERROR >>> %s is not a flag at line %d\n", instructions[i].data, i+1);
                    remove("hexa.txt");
                    exit(1);
                }
                fprintf(output, " %04x\n", atoi(instructions[i].data)); // Convert to 4 Hexa
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

    // FILE *machine_code = open("hexa.txt", "r");
    // int PC = 0;
    // int SP = 0;
    // int *STACK[MEMORY];

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

// FLAG: Operation #VAL
void convert_line(const char *input, Instruction *line)
{

    int i = 0, tick = 0;
    while (input[i] != '\0' && tick == 0)
    {
        if (input[i] == ':') // We only consider the first ':' per sentence
        {
            tick = i;
            if (tick == 0)
            {
                printf("NO FLAG >>> %s\n", input); // Sentence is in the format ': TEXT'
                exit(1);
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
        i = tick;
    }

    // Clear spaces/tabls before operation
    while (input[i] != '\0' && (input[i] == ' ' || input[i] == '\t'))
    {
        i++;
    }


    tick = i;

    // Count operation size
    while (input[i] != '\0' && input[i] != ' ')
    {
        i++;
    }

    if (tick == i)
    {
        printf("NO OPERATION >>> %s\n", input);
        exit(1); // No operation
    }
    line->operation = malloc((i-tick+1) * sizeof(char)); // Add /0
    strncpy(line->operation, input+tick, i-tick);
    line->operation[i-tick] = '\0';


    if (input[i] == ' ' && (input[i+1] != ' ' && input[i+1] != '\0')) // To take care if space after operation
    {
        int n = strlen(input);
        line->data = malloc((n - i) * sizeof(char));
        strncpy(line->data, input+i+1, n-i-1);
        line->data[n-i-2] = '\0';
    }

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