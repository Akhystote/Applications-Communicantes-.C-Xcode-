# Compilateur et options
CC    = gcc
FLAGS = -Wall -std=gnu99
LIBS  = -lm

.PHONY: all clean

# Compilation de tous les exécutables
all: serveur client

# Compilation du serveur
serveur:
	$(CC) $(FLAGS) $(LIBS) -o serveur serveur.c

# Compilation du client en ligne de commande
client:
	$(CC) $(FLAGS) $(LIBS) -o client Main.c client.c

# Suppression des exécutables
clean:
	rm -f serveur client
