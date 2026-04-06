#type compilateur
CC = gcc

#options de compilation
CFLAGS = -Wall -Wextra -Werror

#fichiers sources grâce wildcard
SRC = $(wildcard *.c)

#on enlève l'extension .c
EXEC = $(SRC:.c=)

#règle par défaut création de tous les exec
all: $(EXEC)

#création d'exec à partir de code en c
%: %.c
	$(CC) $(CFLAGS) $< -o $@

# Nettoyage : supprime tous les exécutables générés
clean:
	rm -f $(EXEC)

# Déclarer des cibles "logiques"
.PHONY: all clean
