/* client.h */
#ifndef CLIENT_H
#define CLIENT_H

#include <netdb.h>

#define PORT          5001
#define BUF_SIZE      4096
#define MAX_ETUDIANTS 100
#define MAX_DATES      50

typedef struct {
    char etudid[20];
    char nom[50];
    char prenom[50];
} Etudiant;

/* Utilitaires */
void  supprimer_retour(char *str);
char *mon_strsep(char **str, char delim);

/* Communication */
void envoyer_commande(struct hostent *server, const char *commande);
int  classe_existe(struct hostent *server, const char *classe);
int  recuperer_etudiants(struct hostent *server, const char *classe,
                         Etudiant etudiants[], int max);
int  recuperer_dates(struct hostent *server, const char *classe,
                     char dates[][20], int max);
void envoyer_appel(struct hostent *server, const char *classe,
                   const char *date, const char *appel_data);

/* Actions menu */
void lire_date(char *date);
int  choisir_date(struct hostent *server, const char *classe,
                  char *date_choisie);
void action_faire_appel(struct hostent *server, const char *classe);
void action_afficher_statuts(struct hostent *server, const char *classe);
void action_modifier_statut(struct hostent *server, const char *classe);

/* Menu */
void afficher_menu(const char *classe);
int  lire_choix(void);
void menu_classe(struct hostent *server, const char *classe);

#endif