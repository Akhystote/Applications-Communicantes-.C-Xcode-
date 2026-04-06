/* client.h */
#ifndef CLIENT_H
#define CLIENT_H

#define PORT       5001
#define MAX_ELEVES 200
#define MAX_NOM    100

//toto

/* Structure représentant un élève avec son statut de présence */
typedef struct {
    char nom[MAX_NOM]; /* "NOM PRENOM" */
    int  present;      /* 1 = présent, 0 = absent */
} Eleve;

/* Structure pour stocker la liste complète d'une classe */
typedef struct {
    Eleve eleves[MAX_ELEVES];
    int   nb_eleves;
} ListeEleves;

/* Connexion au serveur — retourne le sockfd, -1 si erreur */
int connecter_serveur(const char *hostname, int port);

/*
 * Récupérer la liste des élèves d'une classe pour une date donnée
 * Envoie : GET_LISTE|classe|date\n
 * Reçoit : NOM PRENOM;P\n ou NOM PRENOM;A\n ... FIN\n
 * Stocke le résultat dans liste (passé par pointeur)
 */
void recuperer_liste(int sockfd, const char *classe,
                     const char *date, ListeEleves *liste);

/*
 * Envoyer la liste complète des présences vers le serveur
 * Envoie : ENVOYER_APPEL|classe|date\n puis NOM;P\n ou NOM;A\n ... FIN\n
 * Retourne 1 si OK, 0 si erreur
 */
int envoyer_appel(int sockfd, const char *classe,
                  const char *date, ListeEleves *liste);

#endif
