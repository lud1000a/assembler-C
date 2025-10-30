#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 

#define WORD_SIZE 8
#define MIN_BINARY_LINES 128

typedef struct {
    const char* name;
    const char* opcode;
    int num_arguments;
} Instruction;

const Instruction mnemonics_table[] = {
    {"INC",  "00000001", 1}, // Increment
    {"DEC",  "00000010", 1}, // Decrement
    {"NOT",  "00000011", 1}, // Bitwise NOT
    {"JMP",  "00000100", 1}, // Unconditional Jump
    {"ADD",  "00010000", 2}, // Addition
    {"SUB",  "00100000", 2}, // Subtraction
    {"MUL",  "00110000", 2}, // Multiplication
    {"DIV",  "01000000", 2}, // Division
    {"MOD",  "01010000", 2}, // Modulo
    {"AND",  "01100000", 2}, // Bitwise AND
    {"OR" ,  "01110000", 2}, // Bitwise OR
    {"XOR",  "10000000", 2}, // Bitwise XOR
    {"NAND", "10010000", 2}, // Bitwise NAND
    {"NOR",  "10100000", 2}, // Bitwise NOR
    {"XNOR", "10110000", 2}, // Bitwise XNOR
    {"COMP", "11000000", 2}  // Compare
};

const int mnemonics_count = sizeof(mnemonics_table) / sizeof(mnemonics_table[0]);

const Instruction* getInstruction(const char* name) {
    for (int i = 0; i < mnemonics_count; ++i) {
        if (strcmp(mnemonics_table[i].name, name) == 0) {
            return &mnemonics_table[i];
        }
    }
    return NULL;
}

void removeComments(char* line) {
    char* comment_start = strchr(line, ';');
    if (comment_start != NULL) {
        *comment_start = '\0';
    }

    line[strcspn(line, "\r\n")] = 0;
}

void removeSpaces(char* line) {

    int len = strlen(line);
    while (len > 0 && isspace((unsigned char)line[len - 1])) {
        line[--len] = '\0';
    }

    char* start = line;
    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }

    if (start != line) {
        memmove(line, start, strlen(start) + 1);
    }
}

void freeBinaryLines(char** lines, int count) {
    if (lines != NULL) {
        for (int i = 0; i < count; ++i) {
            free(lines[i]); 
        }
        free(lines); 
    }
}

char* padArgument(const char* arg) {
    int argLen = strlen(arg);
    int padLen = WORD_SIZE - argLen;
    
    char* padded = (char*)malloc(WORD_SIZE + 1);
    if (padded == NULL) {
         perror("erro ao alocar memória para padArgument");
         return NULL;
    }
    
    if (padLen > 0) {
        
        memset(padded, '0', padLen);
    
        strcpy(padded + padLen, arg);
    } else {
        
        strcpy(padded, arg);
    }
    
    return padded;
}


int isBinary(const char* arg) {
    if (arg[0] == '\0') return 0;
    for (int i = 0; arg[i] != '\0'; ++i) {
        if (arg[i] != '0' && arg[i] != '1') {
            return 0; 
        }
    }
    return 1; 
}   


int main(int argc, char *argv[]){

    //verificação de argumentos 
     if (argc != 3) {
         return 1; // erro
    }

    printf("Entrada: %s\n", argv[1]);
    printf("Saída: %s\n", argv[2]);
    
    //leitura no arquivo de entrada 
    FILE *inputFile = fopen(argv[1], "r");
    if (inputFile == NULL) {
        perror("erro/arquivo de entrada");
        return 1;
    }
    
    //escrita no arquivo de saída
    FILE *outputFile = fopen(argv[2], "w");
    if (outputFile == NULL) {
        perror("erro/arquivo de saída");
        fclose(inputFile);
        return 1;
    }
    
    char buffer[256];        
    int lineCounter = 0;         
    char **binLines = NULL; 
    int binLinesCount = 0;  

     while (fgets(buffer, sizeof(buffer), inputFile) != NULL){

         lineCounter++;

         removeComments(buffer);
         removeSpaces(buffer);

         if (buffer[0] == '\0') {
             continue;
         }

         const Instruction* instruction = getInstruction(buffer);

         if (instruction == NULL) { 
             fprintf(stderr, "Erro [Linha %d]: Comando desconhecido '%s'.\n", lineCounter, buffer);
             fclose(inputFile);
             fclose(outputFile);
             freeBinaryLines(binLines, binLinesCount);
             return 3;
         }
         
         binLinesCount++;
         binLines = (char**)realloc(binLines, binLinesCount * sizeof(char*));

         if (binLines == NULL) {
             perror("Erro ao realocar binaryLines (opcode)");
             fclose(inputFile); fclose(outputFile); freeBinaryLines(binLines, binLinesCount-1);
             return 10;
         }
         

         binLines[binLinesCount - 1] = strdup(instruction->opcode); 
         if (binLines[binLinesCount - 1] == NULL) {
             perror("Erro no strdup (opcode)");
             fclose(inputFile); fclose(outputFile); freeBinaryLines(binLines, binLinesCount-1);
             return 11;
         }

         
         for (int i = 0; i < instruction->num_arguments; ++i) {
             char argBuffer[256];
             int argFound = 0;
             
            
             while (fgets(argBuffer, sizeof(argBuffer), inputFile) != NULL) {
                 lineCounter++;
                 removeComments(argBuffer);
                 removeSpaces(argBuffer);
                 if (argBuffer[0] != '\0') {
                     argFound = 1;
                     break;
                 }
             } 

             if (!argFound) {
                
                 fprintf(stderr, "Erro: Fim do arquivo alcancado enquanto esperava um argumento para o comando '%s'.\n", buffer);
                 fclose(inputFile); fclose(outputFile); freeBinaryLines(binLines, binLinesCount); 
                 return 4;
             }          

             if (!isBinary(argBuffer) || strlen(argBuffer) > WORD_SIZE) {
                 fprintf(stderr, "Erro [Linha %d]: Argumento invalido '%s' para o comando '%s'.\n", lineCounter, argBuffer, buffer);
                 fprintf(stderr, "Esperado um binário de %d bits.\n", WORD_SIZE);
                 fclose(inputFile); fclose(outputFile); freeBinaryLines(binLines, binLinesCount); 
                 return 5;
             }

             char* paddedArg = padArgument(argBuffer);

             if (paddedArg == NULL) {
            
                  fclose(inputFile); fclose(outputFile); freeBinaryLines(binLines, binLinesCount); 
                  return 12;
             }

             binLinesCount++;
             binLines = (char**)realloc(binLines, binLinesCount * sizeof(char*));
             if (binLines == NULL) {
                 perror("Erro ao realocar binaryLines (argumento)");
                 free(paddedArg); 
                 fclose(inputFile); fclose(outputFile); freeBinaryLines(binLines, binLinesCount-1);
                 return 13;
             }
            
             binLines[binLinesCount - 1] = paddedArg;


         } 
     } 

     for (int i = 0; i < binLinesCount; ++i) {
         fprintf(outputFile, "%s\n", binLines[i]);
     }

     char zeroPad[WORD_SIZE + 1];
     memset(zeroPad, '0', WORD_SIZE);
     zeroPad[WORD_SIZE] = '\0';

     
     for (int i = binLinesCount; i < MIN_BINARY_LINES; ++i) {
         fprintf(outputFile, "%s\n", zeroPad);
     }
     
     printf("Compilação legal. %d linhas de código:.\n", binLinesCount);

     fclose(inputFile);
     fclose(outputFile);
     freeBinaryLines(binLines, binLinesCount);

     return 0; 
}
