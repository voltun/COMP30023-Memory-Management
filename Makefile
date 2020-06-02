CC=gcc
CFLAGS=-Wall -Wextra -std=gnu99
OBJ=scheduler.o
EXE=scheduler

$(EXE): $(OBJ)
	$(CC) -o $(EXE) $(OBJ) $(CFLAGS)

scheduler.o: src/scheduler.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJ)