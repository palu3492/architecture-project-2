#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUMMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 8 /* number of machine registers */

#define ADD 0
#define NAND 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION 0x1c00000

typedef struct statestruct {
	int pc;
	int mem[NUMMEMORY];
	int reg[NUMREGS];
	int nummemory;
} statetype;

int field0(int instruction){
	return( (instruction>>19) & 0x7);
}

int field1(int instruction){
	return( (instruction>>16) & 0x7);
}

int field2(int instruction){
	return(instruction & 0xFFFF);
}

int opcode(int instruction){
	return(instruction>>22);
}

void printstate(statetype *stateptr){
	int i;
	printf("\n@@@\nstate:\n");
	printf("\tpc %d\n", stateptr->pc);
	printf("\tmemory:\n");
	for(i = 0; i < stateptr->nummemory; i++){
		printf("\t\tmem[%d]=%d\n", i, stateptr->mem[i]);
	}	
	printf("\tregisters:\n");
	for(i = 0; i < NUMREGS; i++){
		printf("\t\treg[%d]=%d\n", i, stateptr->reg[i]);
	}
	printf("end state\n");
}

int signextend(int num){
	// convert a 16-bit number into a 32-bit integer
	if (num & (1<<15) ) {
		num -= (1<<16);
	}
	return num;
}

void print_stats(int n_instrs, int n_cycles){
	printf("INSTRUCTIONS: %d\n", n_instrs); // total executed instructions
	// Print the total number of cycles executed
    printf("CYCLES: %d\n", n_cycles); // total executed cycles
}

void run(statetype* state){

	// Reused variables;
	int instr = 0;
	int regA = 0;
	int regB = 0;
	int offset = 0;
	int branchtarget = 0;
	int aluresult = 0;
    // Totals
	int total_instrs = 0;
	int total_cycles = 0;

	// Primary loop
	while(1){
		total_instrs++;

		printstate(state);

		// Instruction Fetch
		instr = state->mem[state->pc]; // mem holds decimal version of line

		/* check for halt */
		if (opcode(instr) == HALT) {
			printf("machine halted\n");
            // HALT requires 1 cycle
            total_cycles += 1;
			break;
		}

		// Increment the PC
		state->pc = state->pc+1;

		// Set reg A and B
		regA = state->reg[field0(instr)]; // field 0 is register number, state-reg gets value from that register
		regB = state->reg[field1(instr)];

		// Set sign extended offset
		offset = signextend(field2(instr));

		// Branch target gets set regardless of instruction
		// Only used for branching
		branchtarget = state->pc + offset;

		/**
		 *
		 * Action depends on instruction
		 *
		 **/
		// ADD
		if(opcode(instr) == ADD){
			// Add
			aluresult = regA + regB;
			// Save result
			state->reg[field2(instr)] = aluresult;
			// ADD requires 2 cycles
            total_cycles += 2;
		}
		// NAND
		else if(opcode(instr) == NAND){
			// NAND
			aluresult = ~(regA & regB);
			// Save result
			state->reg[field2(instr)] = aluresult;
            // NAND requires 2 cycles
            total_cycles += 2;
		}
		// LW or SW
		else if(opcode(instr) == LW || opcode(instr) == SW){
			// Calculate memory address
			aluresult = regB + offset;
			if(opcode(instr) == LW){
				// Load
				state->reg[field0(instr)] = state->mem[aluresult];
			}else if(opcode(instr) == SW){
				// Store
				state->mem[aluresult] = regA;
			}
            // LW and SW each require 2 cycles
            total_cycles += 8;
		}
		// JALR
		else if(opcode(instr) == JALR){
			// Save pc+1 in regA
			state->reg[field0(instr)] = state->pc;
			//Jump to the address in regB;
			state->pc = state->reg[field1(instr)];
            // JALR requires 1 cycle
            total_cycles += 1;
		}
		// BEQ
		else if(opcode(instr) == BEQ){
			// Calculate condition
			aluresult = (regA == regB);

			// ZD
			if(aluresult){
				// branch
				state->pc = branchtarget;
			}
            // BEQ requires 4 cycles
            total_cycles += 4;
		}
        // NOOP
        else if(opcode(instr) == NOOP){
            // NOOP requires 1 cycle
            total_cycles += 1;
        }
	} // While
	print_stats(total_instrs, total_cycles);
}

int main(int argc, char** argv){

	/** Get command line arguments **/
	char* fname;

	if(argc == 1){
		fname = (char*)malloc(sizeof(char)*100);
		printf("Enter the name of the machine code file to simulate: ");
		fgets(fname, 100, stdin);
		fname[strlen(fname)-1] = '\0'; // gobble up the \n with a \0
	}
	else if (argc == 2){

		int strsize = strlen(argv[1]);

		fname = (char*)malloc(strsize);
		fname[0] = '\0';

		strcat(fname, argv[1]);
	}else{
		printf("Please run this program correctly\n");
		exit(-1);
	}

	FILE *fp = fopen(fname, "r");
	if (fp == NULL) {
		printf("Cannot open file '%s' : %s\n", fname, strerror(errno));
		return -1;
	}

	/* count the number of lines by counting newline characters */
	int line_count = 0;
	int c;
	while (EOF != (c=getc(fp))) {
		if ( c == '\n' ){
			line_count++;
		}
	}
	// reset fp to the beginning of the file
	rewind(fp);

	statetype* state = (statetype*)malloc(sizeof(statetype));

	state->pc = 0;
	memset(state->mem, 0, NUMMEMORY*sizeof(int));
	memset(state->reg, 0, NUMREGS*sizeof(int));

	state->nummemory = line_count;

	char line[256];

	int i = 0;
	while (fgets(line, sizeof(line), fp)) {
		/* note that fgets doesn't strip the terminating \n, checking its
		   presence would allow to handle lines longer that sizeof(line) */
		state->mem[i] = atoi(line);
		i++;
	}
	fclose(fp);

	/** Run the simulation **/
	run(state);

	free(state);
	free(fname);

}
