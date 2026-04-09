/* Main.c */
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Affiche la liste avec les statuts de présence */
static void afficher_liste(ListeEleves *liste) {
    printf("\n--- Liste des élèves ---\n");
    for (int i = 0; i < liste->nb_eleves; i++) {
        printf("  %2d. %-30s [%s]\n",
               i + 1,
               liste->eleves[i].nom,
               liste->eleves[i].present ? "PRESENT" : "ABSENT");
    }
    printf("------------------------\n");
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Utilisation : %s hostname port\n", argv[0]);
        exit(1);
    }

    const char *hostname = argv[1];
    int port = atoi(argv[2]);
    char classe[100];
    char date[20];
    char commande[256];
    ListeEleves liste;

    printf("================\n");
    printf("   Appel\n");
    printf("================\n");

    while (1) {
        /* Saisie de la classe */
        printf("\nEntrez le nom de la classe (ex: RT1FA) ou 'q' pour quitter : ");
        scanf("%99s", classe);

        if (strcmp(classe, "q") == 0 || strcmp(classe, "Q") == 0) {
            printf("Au revoir.\n");
            break;
        }

        /* Saisie de la date */
        printf("Entrez la date (ex: 2026-03-30) : ");
        scanf("%19s", date);

        /* Connexion + demande de liste */
        int sockfd = connecter_serveur(hostname, port);
        if (sockfd < 0) {
            printf("Impossible de se connecter au serveur.\n");
            continue;
        }

        liste.nb_eleves = 0;
        recuperer_liste(sockfd, classe, date, &liste);
        close(sockfd);

        if (liste.nb_eleves == 0) {
            printf("Aucun élève trouvé pour la classe %s.\n", classe);
            continue;
        }

        afficher_liste(&liste);

        /* Boucle de gestion des présences */
        while (1) {
            printf("\nCommandes :\n");
            printf("  p <num>  = marquer PRESENT\n");
            printf("  a <num>  = marquer ABSENT\n");
            printf("  t        = appel rapide (a/p pour chaque élève)\n");
            printf("  l        = réafficher la liste\n");
            printf("  s        = sauvegarder et changer de classe\n");
            printf("  q        = quitter\n");
            printf("> ");

            scanf("%255s", commande);

            if (strcmp(commande, "q") == 0 || strcmp(commande, "Q") == 0) {
                /* Sauvegarder avant de quitter */
                sockfd = connecter_serveur(hostname, port);
                if (sockfd >= 0) {
                    envoyer_appel(sockfd, classe, date, &liste);
                    close(sockfd);
                }
                printf("Au revoir.\n");
                return 0;
            }

            if (strcmp(commande, "s") == 0 || strcmp(commande, "S") == 0) {
                /* Sauvegarder et changer de classe */
                sockfd = connecter_serveur(hostname, port);
                if (sockfd >= 0) {
                    envoyer_appel(sockfd, classe, date, &liste);
                    close(sockfd);
                }
                break;
            }

            if (strcmp(commande, "l") == 0 || strcmp(commande, "L") == 0) {
                /* Recharger depuis le serveur */
                sockfd = connecter_serveur(hostname, port);
                if (sockfd >= 0) {
                    recuperer_liste(sockfd, classe, date, &liste);
                    close(sockfd);
                }
                afficher_liste(&liste);
                continue;
            }

            if (commande[0] == 't' || commande[0] == 'T') {
                /* Mode appel rapide : les élèves défilent un par un */
                printf("\n=== Appel rapide : %s — %s ===\n", classe, date);
                printf("Pour chaque élève : a = Absent  |  p = Présent\n");
                printf("--------------------------------------------------\n");

                int i = 0;
                while (i < liste.nb_eleves) {
                    char saisie[10];
                    printf("[%d/%d] %s : ",
                           i + 1, liste.nb_eleves,
                           liste.eleves[i].nom);
                    scanf("%9s", saisie);

                    if (saisie[0] == 'p' || saisie[0] == 'P') {
                        liste.eleves[i].present = 1;
                        i++;
                    } else if (saisie[0] == 'a' || saisie[0] == 'A') {
                        liste.eleves[i].present = 0;
                        i++;
                    } else {
                        printf("Saisie invalide, entrez a ou p.\n");
                    }
                }

                /* Envoyer la liste complète au serveur */
                sockfd = connecter_serveur(hostname, port);
                if (sockfd < 0) {
                    printf("Erreur de connexion.\n");
                    continue;
                }

                int ok = envoyer_appel(sockfd, classe, date, &liste);
                close(sockfd);

                if (ok)
                    printf("Appel enregistré.\n");
                else
                    printf("Erreur du serveur.\n");

                afficher_liste(&liste);
                continue;
            }

            if (commande[0] == 'p' || commande[0] == 'P' ||
                commande[0] == 'a' || commande[0] == 'A') {
                int num;
                scanf("%d", &num);

                if (num < 1 || num > liste.nb_eleves) {
                    printf("Numéro invalide (1-%d).\n", liste.nb_eleves);
                    continue;
                }

                int statut = (commande[0] == 'p' || commande[0] == 'P') ? 1 : 0;
                liste.eleves[num - 1].present = statut;

                /* Pousser la liste mise à jour */
                sockfd = connecter_serveur(hostname, port);
                if (sockfd < 0) {
                    printf("Erreur de connexion.\n");
                    continue;
                }

                int ok = envoyer_appel(sockfd, classe, date, &liste);
                close(sockfd);

                if (ok) {
                    printf("-> %s marqué %s\n",
                           liste.eleves[num - 1].nom,
                           statut ? "PRESENT" : "ABSENT");
                } else {
                    printf("Erreur du serveur.\n");
                }
                continue;
            }

            printf("Commande incorrecte, veuillez recommencer.\n");
        }
    }

    return 0;
}
