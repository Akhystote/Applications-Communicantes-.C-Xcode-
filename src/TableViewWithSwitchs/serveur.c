/* serveur.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <signal.h>

#define PORT            5001
#define BUF_SIZE        4096
#define DOSSIER_STATUTS "statuts_classes"

int sockfd;

/* ===================== UTILITAIRES ===================== */

void closeSocket(int s) {
    printf("\nFermeture du serveur.\n");
    close(sockfd);
    exit(0);
}

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

void creer_dossier_statuts(void) {
    mkdir(DOSSIER_STATUTS, 0755);
}

void nom_fichier_statuts(const char *classe, char *out) {
    sprintf(out, "%s/%s_statuts.csv", DOSSIER_STATUTS, classe);
}

/* Saute la première ligne (en-tête) du CSV */
void sauter_entete_csv(FILE *fp) {
    char line[256];
    fgets(line, sizeof(line), fp);
}

int parser_ligne_csv(char *line, char *out) {
    char tmp[256];
    char *ptr = tmp;
    char *etudid, *nom, *prenom, *tp;
    char *code_nip, *etat, *civilite, *nom_usuel;

    strncpy(tmp, line, 255);
    tmp[255] = '\0';

    etudid    = mon_strsep(&ptr, ';');
    code_nip  = mon_strsep(&ptr, ';');
    etat      = mon_strsep(&ptr, ';');
    civilite  = mon_strsep(&ptr, ';');
    nom       = mon_strsep(&ptr, ';');
    nom_usuel = mon_strsep(&ptr, ';');
    prenom    = mon_strsep(&ptr, ';');
    tp        = mon_strsep(&ptr, '\n');

    if (!etudid || !nom || !prenom || !tp)
        return 0;

    supprimer_retour(tp);
    sprintf(out, "%-6s | %-25s %-20s | %s\n", etudid, nom, prenom, tp);
    return 1;
}

/* ===================== METIER ===================== */

int trouver_etudiant(const char *etudid, const char *classe,
                     char *nom, char *prenom) {
    char filename[128];
    char line[256];
    FILE *fp;

    sprintf(filename, "%s.csv", classe);
    fp = fopen(filename, "r");
    if (!fp) return 0;

    sauter_entete_csv(fp);

    while (fgets(line, sizeof(line), fp)) {
        char tmp[256];
        char *ptr = tmp;
        char *id, *n, *p;
        char *code_nip, *etat, *civilite, *nom_usuel;

        strncpy(tmp, line, 255);

        id = mon_strsep(&ptr, ';');
        if (!id) continue;

        if (strcmp(id, etudid) == 0) {
            code_nip  = mon_strsep(&ptr, ';');
            etat      = mon_strsep(&ptr, ';');
            civilite  = mon_strsep(&ptr, ';');
            n         = mon_strsep(&ptr, ';');
            nom_usuel = mon_strsep(&ptr, ';');
            p         = mon_strsep(&ptr, ';');

            if (n) { strncpy(nom,    n, 49); nom[49]    = '\0'; }
            if (p) { strncpy(prenom, p, 49); prenom[49] = '\0'; }
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int est_enregistre(const char *etudid, const char *classe) {
    char statuts_file[128];
    char line[256];
    FILE *fp;

    nom_fichier_statuts(classe, statuts_file);
    fp = fopen(statuts_file, "r");
    if (!fp) return 0;

    while (fgets(line, sizeof(line), fp)) {
        char tmp[256];
        char *ptr = tmp;
        char *id, *cl;
        char *nom, *prenom;

        strncpy(tmp, line, 255);

        id     = mon_strsep(&ptr, ';');
        nom    = mon_strsep(&ptr, ';');
        prenom = mon_strsep(&ptr, ';');
        cl     = mon_strsep(&ptr, ';');

        if (id && cl && strcmp(id, etudid) == 0 && strcmp(cl, classe) == 0) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int ajouter_statut(const char *etudid, const char *nom,
                   const char *prenom, const char *classe,
                   const char *statut) {
    char statuts_file[128];
    FILE *fp;

    nom_fichier_statuts(classe, statuts_file);
    fp = fopen(statuts_file, "a");
    if (!fp) return 0;
    fprintf(fp, "%s;%s;%s;%s;%s\n", etudid, nom, prenom, classe, statut);
    fclose(fp);
    return 1;
}

int modifier_statut(const char *etudid, const char *classe,
                    const char *nouveau_statut) {
    char statuts_file[128];
    char lines[200][256];
    char line[256];
    int count, found, i;
    FILE *fp;

    count = 0;
    found = 0;

    nom_fichier_statuts(classe, statuts_file);
    fp = fopen(statuts_file, "r");
    if (!fp) return 0;

    while (fgets(line, sizeof(line), fp) && count < 200) {
        char tmp[256];
        char *ptr = tmp;
        char *id, *nom, *prenom, *cl;

        strncpy(tmp, line, 255);

        id     = mon_strsep(&ptr, ';');
        nom    = mon_strsep(&ptr, ';');
        prenom = mon_strsep(&ptr, ';');
        cl     = mon_strsep(&ptr, ';');

        if (id && cl && strcmp(id, etudid) == 0 && strcmp(cl, classe) == 0) {
            sprintf(lines[count], "%s;%s;%s;%s;%s\n",
                    id,
                    nom    ? nom    : "",
                    prenom ? prenom : "",
                    cl,
                    nouveau_statut);
            found = 1;
        } else {
            strncpy(lines[count], line, 255);
        }
        count++;
    }
    fclose(fp);

    if (!found) return 0;

    fp = fopen(statuts_file, "w");
    if (!fp) return 0;
    for (i = 0; i < count; i++)
        fputs(lines[i], fp);
    fclose(fp);
    return 1;
}

/* ===================== TRAITEMENTS SOCKET ===================== */

void traiter_list(int sock, char *classname) {
    char filename[128];
    char response[BUF_SIZE];
    char line[256];
    char entry[200];
    FILE *fp;

    supprimer_retour(classname);
    sprintf(filename, "%s.csv", classname);
    fp = fopen(filename, "r");

    if (!fp) {
        sprintf(response, "ERR: Fichier '%s' introuvable.\n", filename);
        write(sock, response, strlen(response));
        return;
    }

    sprintf(response,
            "=== Classe : %s ===\n%-6s | %-25s %-20s | %s\n%s\n",
            classname, "ID", "NOM", "PRENOM", "GROUPE",
            "--------------------------------------------------------------");

    sauter_entete_csv(fp);

    while (fgets(line, sizeof(line), fp)) {
        if (parser_ligne_csv(line, entry))
            sprintf(response + strlen(response), "%s", entry);
    }
    fclose(fp);
    write(sock, response, strlen(response));
}

void traiter_etudiants(int sock, char *classname) {
    char filename[128];
    char response[BUF_SIZE];
    char line[256];
    FILE *fp;

    supprimer_retour(classname);
    sprintf(filename, "%s.csv", classname);
    fp = fopen(filename, "r");

    if (!fp) {
        write(sock, "ERR: Fichier introuvable.\n", 26);
        return;
    }

    bzero(response, sizeof(response));
    sauter_entete_csv(fp);

    while (fgets(line, sizeof(line), fp)) {
        char tmp[256];
        char *ptr = tmp;
        char *etudid, *nom, *prenom;
        char *code_nip, *etat, *civilite, *nom_usuel;
        char entry[200];

        strncpy(tmp, line, 255);

        etudid    = mon_strsep(&ptr, ';');
        code_nip  = mon_strsep(&ptr, ';');
        etat      = mon_strsep(&ptr, ';');
        civilite  = mon_strsep(&ptr, ';');
        nom       = mon_strsep(&ptr, ';');
        nom_usuel = mon_strsep(&ptr, ';');
        prenom    = mon_strsep(&ptr, ';');

        if (etudid && nom && prenom) {
            supprimer_retour(prenom);
            sprintf(entry, "%s;%s;%s\n", etudid, nom, prenom);
            sprintf(response + strlen(response), "%s", entry);
        }
    }
    fclose(fp);

    sprintf(response + strlen(response), "END\n");
    write(sock, response, strlen(response));
}

void traiter_absent(int sock, char *etudid, char *classe) {
    char nom[50] = "", prenom[50] = "";
    char response[256];

    if (!trouver_etudiant(etudid, classe, nom, prenom)) {
        sprintf(response, "ERR: Etudiant %s non trouve dans %s.\n",
                etudid, classe);
        write(sock, response, strlen(response));
        return;
    }

    if (est_enregistre(etudid, classe)) {
        if (!modifier_statut(etudid, classe, "ABSENT")) {
            write(sock, "ERR: Impossible de modifier le statut.\n", 39);
            return;
        }
    } else {
        if (!ajouter_statut(etudid, nom, prenom, classe, "ABSENT")) {
            write(sock, "ERR: Impossible d'enregistrer.\n", 31);
            return;
        }
    }

    sprintf(response, "OK: %s %s marque ABSENT.\n", nom, prenom);
    write(sock, response, strlen(response));
}

void traiter_present(int sock, char *etudid, char *classe) {
    char nom[50] = "", prenom[50] = "";
    char response[256];

    if (!trouver_etudiant(etudid, classe, nom, prenom)) {
        sprintf(response, "ERR: Etudiant %s non trouve dans %s.\n",
                etudid, classe);
        write(sock, response, strlen(response));
        return;
    }

    if (est_enregistre(etudid, classe)) {
        if (!modifier_statut(etudid, classe, "PRESENT")) {
            write(sock, "ERR: Impossible de modifier le statut.\n", 39);
            return;
        }
    } else {
        if (!ajouter_statut(etudid, nom, prenom, classe, "PRESENT")) {
            write(sock, "ERR: Impossible d'enregistrer.\n", 31);
            return;
        }
    }

    sprintf(response, "OK: %s %s marque PRESENT.\n", nom, prenom);
    write(sock, response, strlen(response));
}

void traiter_statuts(int sock, char *classe) {
    char statuts_file[128];
    char response[BUF_SIZE];
    char line[256];
    int nb;
    FILE *fp;

    nb = 0;
    nom_fichier_statuts(classe, statuts_file);

    sprintf(response,
            "=== Statuts : %s ===\n%-6s | %-25s %-20s | %s\n%s\n",
            classe, "ID", "NOM", "PRENOM", "STATUT",
            "------------------------------------------------------");

    fp = fopen(statuts_file, "r");
    if (fp) {
        while (fgets(line, sizeof(line), fp)) {
            char tmp[256];
            char *ptr = tmp;
            char *id, *nom, *prenom, *cl, *statut;
            char entry[200];

            strncpy(tmp, line, 255);

            id     = mon_strsep(&ptr, ';');
            nom    = mon_strsep(&ptr, ';');
            prenom = mon_strsep(&ptr, ';');
            cl     = mon_strsep(&ptr, ';');
            statut = mon_strsep(&ptr, '\n');

            if (id && cl && strcmp(cl, classe) == 0) {
                if (statut) supprimer_retour(statut);
                sprintf(entry, "%-6s | %-25s %-20s | %s\n",
                        id,
                        nom    ? nom    : "",
                        prenom ? prenom : "",
                        statut ? statut : "");
                sprintf(response + strlen(response), "%s", entry);
                nb++;
            }
        }
        fclose(fp);
    }

    if (nb == 0)
        sprintf(response + strlen(response), "Aucun statut enregistre.\n");

    write(sock, response, strlen(response));
}

/* ===================== DISPATCH + MAIN ===================== */

void doprocessing(int sock) {
    char buffer[256];
    char *cmd, *arg1, *arg2;

    bzero(buffer, sizeof(buffer));
    if (read(sock, buffer, sizeof(buffer) - 1) <= 0)
        return;

    printf("Commande recue : %s\n", buffer);

    cmd  = strtok(buffer, " ");
    arg1 = strtok(NULL,   " ");
    arg2 = strtok(NULL,   " \r\n");

    if (cmd && arg1) {
        if (strcmp(cmd, "LIST") == 0) {
            traiter_list(sock, arg1);
        } else if (strcmp(cmd, "ETUDIANTS") == 0) {
            traiter_etudiants(sock, arg1);
        } else if (strcmp(cmd, "STATUTS") == 0) {
            traiter_statuts(sock, arg1);
        } else if (arg2 && strcmp(cmd, "ABSENT") == 0) {
            traiter_absent(sock, arg1, arg2);
        } else if (arg2 && strcmp(cmd, "PRESENT") == 0) {
            traiter_present(sock, arg1, arg2);
        } else {
            write(sock, "ERR: Commande inconnue.\n", 24);
        }
    }
}

int main(void) {
    int newsockfd;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;
    pid_t pid;

    signal(SIGINT, closeSocket);

    creer_dossier_statuts();

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

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

        if (pid == 0) {
            close(sockfd);
            doprocessing(newsockfd);
            close(newsockfd);
            exit(0);
        } else {
            close(newsockfd);
        }
    }
    return 0;
}