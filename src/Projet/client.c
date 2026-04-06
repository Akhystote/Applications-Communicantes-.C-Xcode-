/* client.c */
#include "client.h"
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/* Connexion au serveur — retourne le sockfd, -1 si erreur */
int connecter_serveur(const char *hostname, int port) {
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Erreur d'ouverture du socket");
        return -1;
    }

    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "Erreur: hote introuvable\n");
        close(sockfd);
        return -1;
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0) {
        perror("Erreur de connexion");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

/*
 * Récupérer la liste des élèves d'une classe pour une date donnée
 * Envoie : GET_LISTE|classe|date\n
 * Reçoit : NOM PRENOM;P\n ou NOM PRENOM;A\n ... FIN\n
 */
void recuperer_liste(int sockfd, const char *classe,
                     const char *date, ListeEleves *liste) {
    char buffer[256];
    FILE *socket_f;

    liste->nb_eleves = 0;

    /* Envoi de la commande */
    snprintf(buffer, sizeof(buffer), "GET_LISTE|%s|%s\n", classe, date);
    write(sockfd, buffer, strlen(buffer));

    /* Lecture ligne par ligne via fdopen */
    socket_f = fdopen(sockfd, "r");
    if (socket_f == NULL) {
        perror("Erreur fdopen");
        return;
    }

    char ligne[512];
    while (fgets(ligne, sizeof(ligne), socket_f) != NULL) {
        ligne[strcspn(ligne, "\r\n")] = '\0';

        if (strcmp(ligne, "FIN") == 0)
            break;

        if (strncmp(ligne, "ERREUR|", 7) == 0) {
            printf("Erreur serveur : %s\n", ligne + 7);
            break;
        }

        /* Parse "NOM PRENOM;P" ou "NOM PRENOM;A" */
        char *sep = strrchr(ligne, ';');
        if (sep != NULL && liste->nb_eleves < MAX_ELEVES) {
            char statut = *(sep + 1);
            *sep = '\0';

            Eleve *e = &liste->eleves[liste->nb_eleves];
            strncpy(e->nom, ligne, MAX_NOM - 1);
            e->nom[MAX_NOM - 1] = '\0';
            e->present = (statut == 'P') ? 1 : 0;
            liste->nb_eleves++;
        }
    }
}

/*
 * Envoyer la liste complète des présences vers le serveur
 * Envoie : ENVOYER_APPEL|classe|date\n puis NOM;P\n ou NOM;A\n ... FIN\n
 * Retourne 1 si OK, 0 si erreur
 */
int envoyer_appel(int sockfd, const char *classe,
                  const char *date, ListeEleves *liste) {
    char buffer[512];
    int i;

    /* Envoi de l'entête */
    snprintf(buffer, sizeof(buffer), "ENVOYER_APPEL|%s|%s\n", classe, date);
    write(sockfd, buffer, strlen(buffer));

    /* Envoi de chaque élève */
    for (i = 0; i < liste->nb_eleves; i++) {
        snprintf(buffer, sizeof(buffer), "%s;%c\n",
                 liste->eleves[i].nom,
                 liste->eleves[i].present ? 'P' : 'A');
        write(sockfd, buffer, strlen(buffer));
    }

    /* Fin de transmission */
    write(sockfd, "FIN\n", 4);

    /* Lecture de la réponse */
    bzero(buffer, sizeof(buffer));
    read(sockfd, buffer, sizeof(buffer) - 1);

    if (strncmp(buffer, "OK|", 3) == 0) {
        printf("Serveur : %s", buffer);
        return 1;
    }

    printf("Erreur serveur : %s", buffer);
    return 0;
}