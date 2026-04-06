#type compilateur
#CC = gcc

#options de compilation
#CFLAGS = -Wall -Wextra -Werror

#règle par défaut création de tous les exec
#all: serveur_multi

#serveur_multi: serveur_multi.c Client.c
	#$(CC) $(CFLAGS) serveur_multi.c Client.c -o serveur_multi

# Nettoyage : supprime tous les exécutables générés
#clean:
	#rm -f serveur_multi

# Déclarer des cibles "logiques"
#.PHONY: all clean


all: serveur client

serveur: serveur.c
	gcc -Wall -Wextra -o serveur serveur.c

client: main.c client.c
	gcc -Wall -Wextra -o client main.c client.c

clean:
	rm -f serveur client
