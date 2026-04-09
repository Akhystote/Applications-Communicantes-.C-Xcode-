/* ViewController.m */
#import "ViewController.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define SERVER_HOST "192.168.x.x"  // ← Remplace par l'IP de ta machine Kali
#define SERVER_PORT 5001

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Cache le clavier quand on tape ailleurs
    UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc]
        initWithTarget:self action:@selector(dismissKeyboard)];
    [self.view addGestureRecognizer:tap];
}

- (void)dismissKeyboard {
    [self.view endEditing:YES];
}

- (IBAction)envoyerClasse:(id)sender {
    NSString *classe = self.classeTextField.text;
    if (classe.length == 0) {
        self.resultatTextView.text = @"Entrez une classe !";
        return;
    }

    // Connexion dans un thread secondaire (obligatoire sur iOS)
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSString *resultat = [self connecterEtEnvoyer:classe];

        // Mise à jour de l'UI dans le thread principal
        dispatch_async(dispatch_get_main_queue(), ^{
            self.resultatTextView.text = resultat;
        });
    });
}

- (NSString *)connecterEtEnvoyer:(NSString *)classe {
    int sockfd;
    struct sockaddr_in addr;
    struct hostent *server;

    // Créer le socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return @"Erreur: impossible de créer le socket";

    // Résoudre le nom du serveur
    server = gethostbyname(SERVER_HOST);
    if (!server) return @"Erreur: serveur introuvable";

    // Configurer l'adresse
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    memcpy(&addr.sin_addr, server->h_addr, server->h_length);

    // Connexion
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sockfd);
        return @"Erreur: connexion impossible\nVérifiez que le serveur est lancé";
    }

    // Envoyer la classe demandée
    const char *msg = [classe UTF8String];
    write(sockfd, msg, strlen(msg));

    // Recevoir la réponse
    char buf[4096];
    int total = 0, n;
    memset(buf, 0, sizeof(buf));
    while ((n = (int)read(sockfd, buf + total, sizeof(buf) - total - 1)) > 0) {
        total += n;
    }
    buf[total] = '\0';
    close(sockfd);

    // Formatter la réponse
    NSString *reponse = [NSString stringWithUTF8String:buf];
    NSArray *lignes = [reponse componentsSeparatedByString:@"\n"];

    if ([lignes[0] hasPrefix:@"ERREUR"]) {
        return lignes[0];
    }

    int nb = [lignes[0] intValue];
    NSMutableString *affichage = [[NSMutableString alloc] init];
    [affichage appendFormat:@"=== %@ (%d étudiants) ===\n", classe, nb];

    for (int i = 1; i <= nb && i < lignes.count; i++) {
        [affichage appendFormat:@"%2d. %@\n", i, lignes[i]];
    }

    return affichage;
}

@end
