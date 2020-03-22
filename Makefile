CC=gcc

all: multicycle_sim.o
	$(CC) multicycle_sim.o -o multicycle_sim
	
sim: multicycle_sim.c
	$(CC) multicycle_sim.c -c multicycle_sim.o
	
clean:
	rm multicycle_sim.o multicycle_sim
