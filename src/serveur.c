/* serveur.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <signal.h>

#define PORT             5001
#define BUF_SIZE         4096
#define DOSSIER_STATUTS  "statuts_classes"
#define MAX_ELEVES       200
#define MAX_NOM          100

/* ===================== UTILITAIRES ===================== */

void gestionnaire_sigchld(int s) {
    (void)s;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void supprimer_retour(char *str) {
    int i = 0;
    while (str[i] != '\0' && str[i] != '\r' && str[i] != '\n')
        i++;
    str[i] = '\0';
}

/* Construit le chemin du fichier CSV de statuts pour une classe+date */
void chemin_fichier_statuts(const char *classe, const char *date, char *out) {
    sprintf(out, "%s/%s_%s.csv", DOSSIER_STATUTS, classe, date);
}

/* Crée le dossier statuts_classes s'il n'existe pas */
void creer_dossier_statuts(void) {
    mkdir(DOSSIER_STATUTS, 0755);
}

/* ===================== METIER ===================== */

/*
 * Lit le fichier classe.csv et renvoie la liste des élèves
 * Format CSV : etudid;code_nip;etat;civilite;nom;nom_usuel;prenom;TP
 * Retourne le nombre d'élèves trouvés, -1 si erreur
 */
int lire_eleves_classe(const char *classe, char noms[][MAX_NOM], int max) {
    char filename[128];
    char line[256];
    FILE *fp;
    int nb = 0;

    sprintf(filename, "%s.csv", classe);
    fp = fopen(filename, "r");
    if (!fp) return -1;

    /* Sauter l'en-tête */
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp) && nb < max) {
        char tmp[256];
        char *ptr = tmp;
        char *nom, *prenom;

        strncpy(tmp, line, 255);
        tmp[255] = '\0';

        /* Parse : etudid;code_nip;etat;civilite;nom;nom_usuel;prenom;TP
         * On saute les 4 premiers champs pour atteindre nom et prenom */
        strsep(&ptr, ";"); /* etudid    — non utilisé */
        strsep(&ptr, ";"); /* code_nip  — non utilisé */
        strsep(&ptr, ";"); /* etat      — non utilisé */
        strsep(&ptr, ";"); /* civilite  — non utilisé */
        nom       = strsep(&ptr, ";");
        strsep(&ptr, ";"); /* nom_usuel — non utilisé */
        prenom    = strsep(&ptr, ";");

        if (!nom || !prenom) continue;

        supprimer_retour(prenom);
        /* Ignorer les lignes vides */
        if (strlen(nom) == 0 && strlen(prenom) == 0) continue;

        snprintf(noms[nb], MAX_NOM - 1, "%s %s", nom, prenom);
        noms[nb][MAX_NOM - 1] = '\0';
        nb++;
    }
    fclose(fp);
    return nb;
}

/*
 * Lit le fichier de statuts pour classe+date
 * Remplit noms[] et statuts[] (1=présent, 0=absent)
 * Retourne le nombre d'élèves, 0 si fichier inexistant
 */
int lire_statuts(const char *classe, const char *date,
                 char noms[][MAX_NOM], int statuts[], int max) {
    char filepath[256];
    char line[256];
    FILE *fp;
    int nb = 0;

    chemin_fichier_statuts(classe, date, filepath);
    fp = fopen(filepath, "r");
    if (!fp) return 0;

    while (fgets(line, sizeof(line), fp) && nb < max) {
        char tmp[256];
        char *ptr = tmp;
        char *nom, *statut_str;

        strncpy(tmp, line, 255);
        tmp[255] = '\0';
        supprimer_retour(tmp);

        nom        = strsep(&ptr, ";");
        statut_str = strsep(&ptr, ";");

        if (!nom || !statut_str) continue;
        if (strlen(nom) == 0) continue;

        strncpy(noms[nb], nom, MAX_NOM - 1);
        noms[nb][MAX_NOM - 1] = '\0';
        statuts[nb] = (strcmp(statut_str, "P") == 0) ? 1 : 0;
        nb++;
    }
    fclose(fp);
    return nb;
}

/*
 * Écrit (ou met à jour) le fichier de statuts pour classe+date
 * noms[] et statuts[] contiennent la liste complète
 * Si le fichier existe déjà, on met à jour uniquement les entrées modifiées
 */
void ecrire_statuts(const char *classe, const char *date,
                    char noms[][MAX_NOM], int statuts[], int nb) {
    char filepath[256];
    char noms_existants[MAX_ELEVES][MAX_NOM];
    int  statuts_existants[MAX_ELEVES];
    int  nb_existants, i, j, trouve;
    FILE *fp;

    chemin_fichier_statuts(classe, date, filepath);

    /* Lire les statuts existants */
    nb_existants = lire_statuts(classe, date,
                                noms_existants, statuts_existants, MAX_ELEVES);

    /* Mettre à jour ou ajouter chaque élève */
    for (i = 0; i < nb; i++) {
        trouve = 0;
        for (j = 0; j < nb_existants; j++) {
            if (strcmp(noms_existants[j], noms[i]) == 0) {
                statuts_existants[j] = statuts[i];
                trouve = 1;
                break;
            }
        }
        if (!trouve && nb_existants < MAX_ELEVES) {
            strncpy(noms_existants[nb_existants], noms[i], MAX_NOM - 1);
            statuts_existants[nb_existants] = statuts[i];
            nb_existants++;
        }
    }

    /* Réécrire le fichier complet */
    fp = fopen(filepath, "w");
    if (!fp) return;

    for (i = 0; i < nb_existants; i++) {
        fprintf(fp, "%s;%c\n",
                noms_existants[i],
                statuts_existants[i] ? 'P' : 'A');
    }
    fclose(fp);
}

/* ===================== TRAITEMENTS SOCKET ===================== */

/*
 * Traite la commande DEMANDER_LISTE|classe|date
 * - Si un fichier de statuts existe pour classe+date : renvoie les statuts enregistrés
 * - Sinon : renvoie la liste des élèves de la classe avec P par défaut
 * Format réponse : "NOM PRENOM;P\n" ou "NOM PRENOM;A\n" ... "FIN\n"
 */
void traiter_demander_liste(int sock, const char *classe, const char *date) {
    char noms[MAX_ELEVES][MAX_NOM];
    int  statuts[MAX_ELEVES];
    int  nb, i;
    char response[256];

    /* Essayer de lire les statuts existants */
    nb = lire_statuts(classe, date, noms, statuts, MAX_ELEVES);

    if (nb == 0) {
        /* Pas de fichier existant : lire depuis la liste de la classe */
        char noms_classe[MAX_ELEVES][MAX_NOM];
        int  nb_classe = lire_eleves_classe(classe, noms_classe, MAX_ELEVES);

        if (nb_classe < 0) {
            snprintf(response, sizeof(response),
                     "ERREUR|Classe '%s' introuvable\n", classe);
            write(sock, response, strlen(response));
            return;
        }

        /* Envoyer chaque élève avec P par défaut */
        for (i = 0; i < nb_classe; i++) {
            snprintf(response, sizeof(response), "%s;P\n", noms_classe[i]);
            write(sock, response, strlen(response));
        }
    } else {
        /* Envoyer les statuts déjà enregistrés */
        for (i = 0; i < nb; i++) {
            snprintf(response, sizeof(response), "%s;%c\n",
                     noms[i], statuts[i] ? 'P' : 'A');
            write(sock, response, strlen(response));
        }
    }

    write(sock, "FIN\n", 4);
}

/*
 * Traite la commande PUSH_APPEL|classe|date
 * Lit ensuite les lignes "NOM PRENOM;P\n" ou "NOM PRENOM;A\n" jusqu'à "FIN\n"
 * Sauvegarde dans statuts_classes/CLASSE_DATE.csv
 */
void traiter_push_appel(FILE *socket_f, int sock,
                        const char *classe, const char *date) {
    char noms[MAX_ELEVES][MAX_NOM];
    int  statuts[MAX_ELEVES];
    int  nb = 0;
    char ligne[256];

    /* Lire les élèves envoyés par le client */
    while (fgets(ligne, sizeof(ligne), socket_f) != NULL) {
        char tmp[256];
        char *ptr = tmp;
        char *nom, *statut_str;

        strncpy(tmp, ligne, 255);
        tmp[255] = '\0';
        supprimer_retour(tmp);

        if (strcmp(tmp, "FIN") == 0) break;

        nom        = strsep(&ptr, ";");
        statut_str = strsep(&ptr, ";");

        if (!nom || !statut_str) continue;
        if (strlen(nom) == 0) continue;

        strncpy(noms[nb], nom, MAX_NOM - 1);
        noms[nb][MAX_NOM - 1] = '\0';
        statuts[nb] = (strcmp(statut_str, "P") == 0) ? 1 : 0;

        if (nb < MAX_ELEVES - 1) nb++;
    }

    /* Sauvegarder */
    ecrire_statuts(classe, date, noms, statuts, nb);

    char response[128];
    snprintf(response, sizeof(response),
             "OK|Appel enregistre pour %s le %s (%d eleves)\n",
             classe, date, nb);
    write(sock, response, strlen(response));
}

/*
 * Dispatch principal : lit la première ligne et route vers le bon handler
 * Protocole :
 *   DEMANDER_LISTE|classe|date\n
 *   PUSH_APPEL|classe|date\n  puis lignes NOM;P/A\n ... FIN\n
 */
void doprocessing(int sock) {
    char premiere_ligne[256];
    char *ptr, *cmd, *classe, *date;
    FILE *socket_f;

    socket_f = fdopen(sock, "r+");
    if (!socket_f) { perror("fdopen"); return; }

    if (fgets(premiere_ligne, sizeof(premiere_ligne), socket_f) == NULL)
        return;

    premiere_ligne[strcspn(premiere_ligne, "\r\n")] = '\0';
    printf("Commande recue : %s\n", premiere_ligne);

    ptr    = premiere_ligne;
    cmd    = strsep(&ptr, "|");
    classe = strsep(&ptr, "|");
    date   = strsep(&ptr, "|");

    if (!cmd || !classe || !date) {
        write(sock, "ERREUR|Format invalide\n", 22);
        return;
    }

    if (strcmp(cmd, "GET_LISTE") == 0) {
        traiter_demander_liste(sock, classe, date);
    } else if (strcmp(cmd, "ENVOYER_APPEL") == 0) {
        traiter_push_appel(socket_f, sock, classe, date);
    } else {
        write(sock, "ERREUR|Commande inconnue\n", 25);
    }

    fflush(socket_f);
}

/* ===================== MAIN ===================== */

int main(void) {
    int sockfd, newsockfd;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;
    struct sigaction sa;
    pid_t pid;

    /* Éviter les processus zombies */
    sa.sa_handler = gestionnaire_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    creer_dossier_statuts();

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    /* Réutilisation du port */
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port        = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind"); exit(1);
    }

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    printf("Serveur en ecoute sur le port %d...\n", PORT);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) { perror("accept"); continue; }

        printf("Connexion acceptee.\n");
        pid = fork();

        if (pid < 0) {
            perror("fork");
            close(newsockfd);
        } else if (pid == 0) {
            /* Processus fils */
            close(sockfd);
            doprocessing(newsockfd);
            close(newsockfd);
            exit(0);
        } else {
            /* Processus père */
            close(newsockfd);
        }
    }

    close(sockfd);
    return 0;
}