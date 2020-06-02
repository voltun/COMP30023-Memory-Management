CC=gcc
CFLAGS=-Wall -Wextra -std=gnu99
OBJ=scheduler.o utilities.o memory.o process_scheduling.o
EXE=scheduler

$(EXE): $(OBJ)
	$(CC) -o $(EXE) $(OBJ) $(CFLAGS)

scheduler.o: src/scheduler.c
	$(CC) -c -o $@ $< $(CFLAGS)

utilities.o: src/utilities.c include/utilities.h
	$(CC) -c -o $@ $< $(CFLAGS)

memory.o: src/memory.c include/memory.h
	$(CC) -c -o $@ $< $(CFLAGS)

process_scheduling.o: src/process_scheduling.c include/process_scheduling.h
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJ)