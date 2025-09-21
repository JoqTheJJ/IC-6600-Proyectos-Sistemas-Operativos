CC     := gcc
CFLAGS := -std=c11 -Wall -Wextra -Werror=implicit-function-declaration
SRC    := $(wildcard *.c)
EXE    := $(SRC:.c=)

FILE   ?= prueba.txt


all: $(EXE)


%: %.c
	$(CC) $(CFLAGS) -o $@ $<


run-huffman: huffman
	./huffman

run-lector: lector
	LANG=C.UTF-8 ./lector $(FILE)


clean:
	rm -f $(EXE)
