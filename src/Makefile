# Compilateur et options
CC    = gcc
FLAGS = -Wall -std=gnu99
LIBS  = -lm
 
# Chemins des sources
SERVEUR_SRC = src/serveur.c
CLIENT_SRC  = clientConsole/console.c clientConsole/client.c
 
.PHONY: all clean
 
# Compilation de tous les exécutables
all: serveur client
 
# Compilation du serveur
serveur:
	$(CC) $(FLAGS) $(LIBS) -o serveur $(SERVEUR_SRC)
 
# Compilation du client console
client:
	$(CC) $(FLAGS) $(LIBS) -o client $(CLIENT_SRC)
 
# Suppression des exécutables
clean:
	rm -f serveur client