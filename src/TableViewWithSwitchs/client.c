/* client.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "client.h"

/* ===================== UTILITAIRES ===================== */

void supprimer_retour(char *str) {
    int i = 0;
    while (str[i] != '\0' && str[i] != '\r' && str[i] != '\n') {
        i++;
    }
    str[i] = '\0';
}

char *mon_strsep(char **str, char delim) {
    char *debut;
    char *ptr;

    if (*str == NULL) return NULL;

    debut = *str;
    ptr   = *str;

    while (*ptr != '\0' && *ptr != delim) {
        ptr++;
    }

    if (*ptr == delim) {
        *ptr = '\0';
        *str = ptr + 1;
    } else {
        *str = NULL;
    }

    return debut;
}

/* ===================== COMMUNICATION ===================== */

void envoyer_commande(struct hostent *server, const char *commande) {
    int sockfd, n;
    struct sockaddr_in serv_addr;
    char buffer[BUF_SIZE];

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); return; }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect"); close(sockfd); return;
    }

    write(sockfd, commande, strlen(commande));

    printf("\n");
    bzero(buffer, sizeof(buffer));
    while ((n = read(sockfd, buffer, sizeof(buffer) - 1)) > 0) {
        printf("%s", buffer);
        bzero(buffer, sizeof(buffer));
    }

    close(sockfd);
}

/* Vérifie si la classe existe sur le serveur.
   Retourne 1 si elle existe, 0 sinon. */
int classe_existe(struct hostent *server, const char *classe) {
    int sockfd, n, total;
    struct sockaddr_in serv_addr;
    char buffer[BUF_SIZE];
    char commande[150];

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); return 0; }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect"); close(sockfd); return 0;
    }

    sprintf(commande, "ETUDIANTS %s", classe);
    write(sockfd, commande, strlen(commande));

    bzero(buffer, sizeof(buffer));
    total = 0;
    while ((n = read(sockfd, buffer + total,
                     sizeof(buffer) - 1 - total)) > 0) {
        total += n;
        buffer[total] = '\0';
        if (strstr(buffer, "END\n") != NULL) break;
        if (strstr(buffer, "ERR")   != NULL) break;
    }
    close(sockfd);

    if (strncmp(buffer, "ERR", 3) == 0)
        return 0;
    return 1;
}

int recuperer_etudiants(struct hostent *server, const char *classe,
                        Etudiant etudiants[], int max) {
    int sockfd, n, nb, total;
    struct sockaddr_in serv_addr;
    char buffer[BUF_SIZE];
    char commande[150];
    char *debut_ligne, *fin_ligne;
    char *id, *nom, *prenom, *ptr_ligne;

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); return 0; }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect"); close(sockfd); return 0;
    }

    sprintf(commande, "ETUDIANTS %s", classe);
    write(sockfd, commande, strlen(commande));

    bzero(buffer, sizeof(buffer));
    total = 0;
    while ((n = read(sockfd, buffer + total,
                     sizeof(buffer) - 1 - total)) > 0) {
        total += n;
        buffer[total] = '\0';
        if (strstr(buffer, "END\n") != NULL) break;
    }
    close(sockfd);

    if (total <= 0) return 0;

    nb          = 0;
    debut_ligne = buffer;

    while (*debut_ligne != '\0' && nb < max) {

        fin_ligne = debut_ligne;
        while (*fin_ligne != '\0' && *fin_ligne != '\n') {
            fin_ligne++;
        }

        if (*fin_ligne == '\n') {
            *fin_ligne = '\0';
        }

        supprimer_retour(debut_ligne);

        if (strcmp(debut_ligne, "END") == 0) break;

        ptr_ligne = debut_ligne;
        id     = mon_strsep(&ptr_ligne, ';');
        nom    = mon_strsep(&ptr_ligne, ';');
        prenom = mon_strsep(&ptr_ligne, ';');

        if (id && nom && prenom && strcmp(id, "") != 0) {
            strncpy(etudiants[nb].etudid,  id,     19);
            strncpy(etudiants[nb].nom,     nom,    49);
            strncpy(etudiants[nb].prenom,  prenom, 49);
            etudiants[nb].etudid[19] = '\0';
            etudiants[nb].nom[49]    = '\0';
            etudiants[nb].prenom[49] = '\0';
            nb++;
        }

        debut_ligne = fin_ligne + 1;
    }

    return nb;
}

int recuperer_dates(struct hostent *server, const char *classe,
                    char dates[][20], int max) {
    int sockfd, n, nb, total;
    struct sockaddr_in serv_addr;
    char buffer[BUF_SIZE];
    char commande[150];
    char *debut_ligne, *fin_ligne;

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); return 0; }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect"); close(sockfd); return 0;
    }

    sprintf(commande, "DATES %s", classe);
    write(sockfd, commande, strlen(commande));

    bzero(buffer, sizeof(buffer));
    total = 0;
    while ((n = read(sockfd, buffer + total,
                     sizeof(buffer) - 1 - total)) > 0) {
        total += n;
        buffer[total] = '\0';
        if (strstr(buffer, "END\n") != NULL) break;
    }
    close(sockfd);

    if (total <= 0) return 0;

    nb          = 0;
    debut_ligne = buffer;

    while (*debut_ligne != '\0' && nb < max) {

        fin_ligne = debut_ligne;
        while (*fin_ligne != '\0' && *fin_ligne != '\n') {
            fin_ligne++;
        }

        if (*fin_ligne == '\n') {
            *fin_ligne = '\0';
        }

        supprimer_retour(debut_ligne);

        if (strcmp(debut_ligne, "END")  == 0) break;
        if (strcmp(debut_ligne, "VIDE") == 0) break;

        strncpy(dates[nb], debut_ligne, 19);
        dates[nb][19] = '\0';
        nb++;

        debut_ligne = fin_ligne + 1;
    }

    return nb;
}

void envoyer_appel(struct hostent *server, const char *classe,
                   const char *date, const char *appel_data) {
    char commande[2048];
    sprintf(commande, "APPEL %s %s %s", classe, date, appel_data);
    envoyer_commande(server, commande);
}

/* ===================== ACTIONS MENU ===================== */

void lire_date(char *date) {
    printf("Entrez la date de l'appel (ex: 2026-03-28) : ");
    fgets(date, 20, stdin);
    supprimer_retour(date);
}

int choisir_date(struct hostent *server, const char *classe,
                 char *date_choisie) {
    char dates[MAX_DATES][20];
    int nb, i, choix;
    char saisie[10];

    nb = recuperer_dates(server, classe, dates, MAX_DATES);

    if (nb == 0) {
        printf("Aucun appel enregistre pour la classe %s.\n", classe);
        return 0;
    }

    printf("\n=== Appels disponibles pour %s ===\n", classe);
    i = 0;
    while (i < nb) {
        printf("%d. %s\n", i + 1, dates[i]);
        i++;
    }

    printf("Votre choix (1-%d) : ", nb);
    fgets(saisie, sizeof(saisie), stdin);
    choix = atoi(saisie);

    if (choix < 1 || choix > nb) {
        printf("Choix invalide.\n");
        return 0;
    }

    strncpy(date_choisie, dates[choix - 1], 19);
    date_choisie[19] = '\0';
    return 1;
}

void action_faire_appel(struct hostent *server, const char *classe) {
    Etudiant etudiants[MAX_ETUDIANTS];
    char statuts[MAX_ETUDIANTS];
    char appel_data[2048];
    char entree[30];
    char date[20];
    int nb, i;
    char saisie[10];

    nb = recuperer_etudiants(server, classe, etudiants, MAX_ETUDIANTS);

    if (nb == 0) {
        printf("Aucun etudiant trouve pour la classe %s.\n", classe);
        return;
    }

    lire_date(date);

    printf("\n=== Appel : %s — %s (%d etudiants) ===\n", classe, date, nb);
    printf("Pour chaque etudiant : A = Absent  |  P = Present\n");
    printf("--------------------------------------------------\n");

    i = 0;
    while (i < nb) {
        printf("[%d/%d] %s %s : ", i + 1, nb,
               etudiants[i].nom, etudiants[i].prenom);

        fgets(saisie, sizeof(saisie), stdin);
        supprimer_retour(saisie);

        if (strcmp(saisie, "A") == 0 || strcmp(saisie, "a") == 0) {
            statuts[i] = 'A';
            i++;
        } else if (strcmp(saisie, "P") == 0 || strcmp(saisie, "p") == 0) {
            statuts[i] = 'P';
            i++;
        } else {
            printf("Saisie invalide, entrez A ou P.\n");
        }
    }

    bzero(appel_data, sizeof(appel_data));
    i = 0;
    while (i < nb) {
        sprintf(entree, "%s:%c", etudiants[i].etudid, statuts[i]);
        sprintf(appel_data + strlen(appel_data), "%s", entree);
        if (i < nb - 1)
            sprintf(appel_data + strlen(appel_data), ";");
        i++;
    }

    envoyer_appel(server, classe, date, appel_data);
}

void action_afficher_statuts(struct hostent *server, const char *classe) {
    char commande[150];
    char date[20];

    if (!choisir_date(server, classe, date))
        return;

    sprintf(commande, "STATUTS %s %s", classe, date);
    envoyer_commande(server, commande);
}

/* Permet de corriger un statut déjà enregistré */
void action_modifier_statut(struct hostent *server, const char *classe) {
    Etudiant etudiants[MAX_ETUDIANTS];
    char date[20];
    char commande[150];
    char saisie[10];
    char nouveau_statut[20];
    int nb, i, choix;

    if (!choisir_date(server, classe, date))
        return;

    nb = recuperer_etudiants(server, classe, etudiants, MAX_ETUDIANTS);
    if (nb == 0) {
        printf("Aucun etudiant trouve.\n");
        return;
    }

    printf("\n=== Choisir un etudiant — %s %s ===\n", classe, date);
    i = 0;
    while (i < nb) {
        printf("%d. %s %s\n", i + 1, etudiants[i].nom, etudiants[i].prenom);
        i++;
    }

    printf("Votre choix (1-%d) : ", nb);
    fgets(saisie, sizeof(saisie), stdin);
    choix = atoi(saisie);

    if (choix < 1 || choix > nb) {
        printf("Choix invalide.\n");
        return;
    }
    choix--;

    printf("Nouveau statut pour %s %s (A/P) : ",
           etudiants[choix].nom, etudiants[choix].prenom);
    fgets(saisie, sizeof(saisie), stdin);
    supprimer_retour(saisie);

    if (strcmp(saisie, "A") == 0 || strcmp(saisie, "a") == 0) {
        strncpy(nouveau_statut, "ABSENT", 19);
    } else if (strcmp(saisie, "P") == 0 || strcmp(saisie, "p") == 0) {
        strncpy(nouveau_statut, "PRESENT", 19);
    } else {
        printf("Saisie invalide.\n");
        return;
    }

    sprintf(commande, "MODIFIER %s %s %s %s",
            classe, date, etudiants[choix].etudid, nouveau_statut);
    envoyer_commande(server, commande);
}

/* ===================== MENU CLASSE ===================== */

void afficher_menu(const char *classe) {
    printf("\n--- Menu : %s ---\n", classe);
    printf("1. Faire l'appel (A/P pour chaque etudiant)\n");
    printf("2. Voir les statuts (absents/presents)\n");
    printf("3. Modifier un statut\n");
    printf("4. Changer de classe\n");
    printf("5. Quitter\n");
    printf("Votre choix : ");
}

int lire_choix(void) {
    char input[10];
    fgets(input, sizeof(input), stdin);
    return atoi(input);
}

void menu_classe(struct hostent *server, const char *classe) {
    int quitter_classe = 0;
    int choix;

    while (!quitter_classe) {
        afficher_menu(classe);
        choix = lire_choix();

        if (choix == 1) {
            action_faire_appel(server, classe);
        } else if (choix == 2) {
            action_afficher_statuts(server, classe);
        } else if (choix == 3) {
            action_modifier_statut(server, classe);
        } else if (choix == 4) {
            quitter_classe = 1;
        } else if (choix == 5) {
            printf("Au revoir.\n");
            exit(0);
        } else {
            printf("Choix invalide, reessayez.\n");
        }
    }
}