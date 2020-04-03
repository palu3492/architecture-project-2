#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

// Define constant
#define NUMMEMORY 20
#define NUMREGS 8

typedef struct state_struct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int num_memory;
} statetype;

// Define functions
void handleCommandLineInputs(char **inputFileName, int argc, char *argv[]);

void simulate(int opCode, int regA, int regB, int val, statetype *stateptr);

int convert_num(int num);

void print_state(statetype *stateptr);

void print_stats(int n_instrs);

// simulate any legal UST-3400 machine-code program
int main(int argc, char *argv[]) {

    // The simulator should begin by initializing all memory, registers, and the program counter to 0
    statetype my_state;

    my_state.pc = 0;

    int i;
    for (i = 0; i < NUMMEMORY; i++) {
        my_state.mem[i] = 0;
    }

    for (i = 0; i < NUMREGS; i++) {
        my_state.reg[i] = 0;
    }

    my_state.num_memory = 0;

    // take the machine-code filename as a command line argument using -i
    char *inputFileName = NULL;
    handleCommandLineInputs(&inputFileName, argc, argv);

    // Read input file that holds machine code
    // load into "memory"?
    // sets num_memory to number of memory addresses used
    FILE *file;

    if (inputFileName != NULL) {
        printf("FILE: %s\n", inputFileName); // prof's sim prints this
        file = fopen(inputFileName, "r");
        if (file != NULL) {
            int val;
            char line[100];
            while (fgets(line, 100, file) != NULL) {
                val = atoi(line);
                my_state.mem[my_state.num_memory] = val;
                my_state.num_memory++;
            }
            fclose(file);
        } else {
            printf("Could not open the file\n");
            exit(-1);
        }
    } else {
        printf("Machine code file needed\n");
        exit(-1);
    }

    // The simulator will then simulate the program until the program executes a halt
    // uses my_state.pc to iterate?
    int num;
    int instructionCount = 0;
    do {
        num = my_state.mem[my_state.pc];

        int opCode = num & 29360128;
        opCode >>= 22;

        int regA = num & 0x380000;
        regA >>= 19;

        int regB = num & 458752;
        regB >>= 16;

        int val = num & 65535;
        val = convert_num(val); // sign extend

        // The simulator should call print_state() before executing each instruction
        print_state(&my_state);
        simulate(opCode, regA, regB, val, &my_state);
        instructionCount++;

        // Halt
        if (opCode == 6) {
            printf("machine halted\n"); // prof's sim prints this
            break;
        }

    } while (1);

    // The simulator should call print_state() nce just before exiting the program
    // print_state(&my_state);
    // Prof's sim does not call this at end of program?

    // The final output is print_stats()
    print_stats(instructionCount);

    return 0;
}

// Function to handle command line inputs
// -i (input) file name of machine code program to be simulated
void handleCommandLineInputs(char **inputFileName, int argc, char *argv[]) {
    int option;
    opterr = 0;
    // Iterate over the supplied command line options
    // If -i then store supplied file name
    while ((option = getopt(argc, argv, "i:")) != -1) {
        switch (option) {
            case 'i':
                *inputFileName = (char *) malloc(strlen(optarg) + 1);
                memcpy(*inputFileName, optarg, strlen(optarg) + 1);
                break;
            case '?':
                printf("unknown option: %c\n", optopt);
                exit(-1);
                break;
        }
    }
}

// Simulate the code
void simulate(int opCode, int regA, int regB, int val, statetype *stateptr) {
    stateptr->pc += 1;
    if (opCode == 0) {
        // add
        stateptr->reg[val] = stateptr->reg[regA] + stateptr->reg[regB];
    } else if (opCode == 1) {
        // nand
        stateptr->reg[val] = ~(stateptr->reg[regA] & stateptr->reg[regB]);
    } else if (opCode == 2) {
        // lw
        int index = stateptr->reg[regB] + val;
        stateptr->reg[regA] = stateptr->mem[index];
    } else if (opCode == 3) {
        // sw
        int index = stateptr->reg[regB] + val;
        stateptr->mem[index] = stateptr->reg[regA];
    } else if (opCode == 4) {
        // beq
        if (stateptr->reg[regA] == stateptr->reg[regB]) {
            stateptr->pc = stateptr->pc + val;
        }
    } else if (opCode == 5) {
        // jalr
        stateptr->reg[regA] = stateptr->pc; // already pc += 1
        stateptr->pc = stateptr->reg[regB];
    } else if (opCode == 6 || opCode == 7) {
        // halt or noop
    } else {
        printf("Unrecognized operation\n");
        stateptr->pc -= 1;
        exit(-1);
    }
}

// Convert a 16-bit number into a signed 32-bit integer
int convert_num(int num) {
    if (num & (1 << 15)) {
        num -= (1 << 16);
    }
    return (num);
}

// This function prints the current state of the machine (program counter, registers, memory)
// print_state(&my_state);
void print_state(statetype *stateptr) {
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", stateptr->pc);
    printf("\tmemory:\n");
    for (i = 0; i < stateptr->num_memory; i++) {
        printf("\t\tmem[%d]=%d\n", i, stateptr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i = 0; i < NUMREGS; i++) {
        printf("\t\treg[%d]=%d\n", i, stateptr->reg[i]);
    }
    printf("end state\n");
}

void print_stats(int n_instrs) {
    printf("INSTRUCTIONS: %d\n", n_instrs); // total executed instructions
}
