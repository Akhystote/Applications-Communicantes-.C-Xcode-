CC=gcc
FLAGS= -Wall -std=c89
LIBS= -lm # utile si vous utilisez <math.h>
EXEC= helloworld

%.o: %.c
	$(CC) $(FLAGS) -c $<

all: $(EXEC)

helloworld: helloworld.o
	$(CC) $(LIBS) $(FLAGS) -o $@ $^

clean:
	rm *.o 
