/* main.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include "client.h"

int main(int argc, char *argv[]) {
    struct hostent *server;
    char classe[100];

    char *hostname = (argc >= 2) ? argv[1] : "localhost";
    server = gethostbyname(hostname);
    if (!server) { fprintf(stderr, "Hote introuvable\n"); exit(1); }

    while (1) {
        printf("\nEntrez le nom de la classe (ou 'quit') : ");
        fgets(classe, sizeof(classe), stdin);
        supprimer_retour(classe);

        if (strcmp(classe, "quit") == 0) {
            printf("Au revoir.\n");
            return 0;
        }

        /* Vérifier que la classe existe avant d'afficher le menu */
        if (!classe_existe(server, classe)) {
            printf("ERR: Classe '%s' introuvable sur le serveur.\n", classe);
            continue;
        }

        menu_classe(server, classe);
    }
    return 0;
}