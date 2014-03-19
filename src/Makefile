CC	= mpicc.openmpi

INCLUDE = -I include/
LIB = -I/usr/include/SDL -lSDL

EXEFILES = fish



.PHONY: all clean exe

all: clean $(EXEFILES)

fish_init.o: fish_init.c
	$(CC) -g -Wall $(INCLUDE) -c fish_init.c

fish_utils.o: fish_utils.c
	$(CC) -g -Wall $(INCLUDE) -c fish_utils.c

simulation.o: simulation.c
	$(CC) -g -Wall $(INCLUDE) -c simulation.c

main.o: main.c
	$(CC) -g -Wall $(INCLUDE) -c main.c

communication.o: communication.c
	$(CC) -g -Wall $(INCLUDE) -c communication.c

affichage.o:  affichage.c
	$(CC) -g -Wall $(INCLUDE) -c affichage.c

sdl.o: sdl.c
	$(CC) -g -Wall $(INCLUDE) -c sdl.c $(LIB)

run: fish
	mpirun.openmpi -np 3 fish ocean_ex/ocean_200.oce 50

fish: fish_init.o fish_utils.o simulation.o main.o communication.o affichage.o sdl.o
	$(CC) -g -Wall $(INCLUDE) *.o -o fish $(LIB)


clean: 
	rm -f $(EXEFILES) *.o *~ PI*


