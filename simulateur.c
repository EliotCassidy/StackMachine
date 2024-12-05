// Ce fichier traduit un code assembleur passé en ligne de commande en un code machine

// TODO : Error handling (look for the line : index of the current line), Flags
// TODO : Clean string before doing checks


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
    int adress;
} Instruction;


char* get_code(const char *operation);
void clean_line(char *sentence, int *flag);

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

int main(int argc, char** argv) {

    // if (argc != 2) {
    //     return 1; // Wrong format
    // }

    // FILE *input, *output;


    // input = fopen(argv[1], "r");
    // if (input == NULL) {
    //     return 1;
    // }

    // // Idea now is to implement a list of lists where every line is an element of the list
    

    
    // fclose(input);
    // fclose(output);

    char *text0 = "flag: blabla";
    char *text1 = "flag:blabla"; // Should add elements
    char *text2 = "flag:    blabla";
    char *text3 = "flag:    blabla";
    char *text4 = "flag:            blabla";
    char *text5 = "flag:            blabla";
    char *text6 = "flag:             blabla";
    char *text7 = ": blabla"; // Should return Error
    char *text8 = "     blabla ";
    int *flag = 0;
    clean_line(text0, *flag);
    printf("%s", text0);

}

char* get_code(const char *operation) {
    for (int i = 0; i < sizeof(operations_codes) / sizeof(Dict); i++) {
        if (strcmp(operations_codes[i].operation, operation) == 0)
            return operations_codes[i].code;
    }
    return "10"; // Error handling
}


void clean_line(char *sentence, int *flag) {
    int i = 0;
    while (sentence[i] != '\0') {
        if (sentence[i] == ':') {
            *flag = 1;
            while (sentence[i+1]==' ' || sentence[i+1] =='\t') {
                for (int j = i + 1; sentence[j] != '\0'; j++) {
                    sentence[j] = sentence[j + 1];
                }
            }
            // Add ONE space after ':'
            for (int j = i + 1; sentence[j] != '\0'; j++) {
                sentence[j] = ' ';
                sentence[j+1] = sentence[j];
            }            
        }
        i++;
    }

}
