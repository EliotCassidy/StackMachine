// Ce fichier traduit un code assembleur passé en ligne de commande en un code machine

// TODO : Error handling (look for the line : index of the current line), Flags
// TODO : Clean string before doing checks
// TODO : Free all


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256

typedef struct {
    char *operation;
    char code[3];
} Dict;

typedef struct {
    char *flag;
    char *operation;
    char *data;
} Instruction;


char* get_code(const char *operation);
void convert_line(const char *input, Instruction*);

// Fonctions à réimplementer que j'ai pomper d'internet
void to_hex(int number, char *hex) {
    sprintf(hex, "%04X", number);
}

Dict operations_codes[] = {
    {"pop", "00"}, {"ipop", "01"}, {"push", "02"}, {"ipush", "03"},
    {"push#", "04"}, {"jmp", "05"}, {"jnz", "06"}, {"call", "07"},
    {"ret", "08"}, {"read", "09"},{"write", "0A"}, {"op", "0B"},
    {"rnd", "0C"}, {"dup", "0D"}, {"halt", "63"}
};

int main(int argc, char** argv)
{

    // if (argc != 2) {
    //     exit(1); // Wrong format
    // }

    // FILE *input, *output;


    // input = fopen(argv[1], "r");
    // if (input == NULL) {
    //     exit(1);
    // }

    // // Idea now is to implement a list of lists where every line is an element of the list
    

    
    // fclose(input);
    // fclose(output);

    char *text0 = "a:  op c";

    Instruction line = {NULL, NULL, NULL};
    convert_line(text0, &line);
    printf("%s,%s,%s,\n", line.flag, line.operation, line.data);

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
            line->flag = malloc((i+2) * sizeof(char)); // We add the /0
            strncpy(line->flag, input, i);
            line->flag[i+1] = '\0';
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
    line->operation = malloc((i-tick+2) * sizeof(char)); // Add /0
    strncpy(line->operation, input+tick, i-tick);
    line->operation[i-tick+1] = '\0';

    if (input[i] == ' ' && (input[i+1] != ' ' && input[i+1] != '\0')) // To take care if space after operation
    {
        int n = strlen(input);
        line->data = malloc((n - i + 3) * sizeof(char));
        strncpy(line->data, input+i+1, n-i-1);
        line->data[n-i+2] = '\0';
    }

}