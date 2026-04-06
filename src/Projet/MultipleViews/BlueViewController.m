/* BlueViewController.m */
#import "BlueViewController.h"

@interface BlueViewController ()

/* Liste des élèves reçue du serveur — passée par valeur, pas de variable globale */
@property (nonatomic, assign) ListeEleves liste;

/* Spinner de chargement */
@property (nonatomic, strong) UIActivityIndicatorView *spinner;

@end

@implementation BlueViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    self.title = [NSString stringWithFormat:@"%@ — %@",
                  self.classe, self.date];

    /* Bouton retour */
    self.navigationItem.leftBarButtonItem =
        [[UIBarButtonItem alloc] initWithTitle:@"Retour"
                                         style:UIBarButtonItemStylePlain
                                        target:self
                                        action:@selector(retour)];

    /* Spinner dans la barre de navigation */
    self.spinner = [[UIActivityIndicatorView alloc]
                    initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleMedium];
    self.navigationItem.rightBarButtonItem =
        [[UIBarButtonItem alloc] initWithCustomView:self.spinner];

    /* Enregistrement de la cellule */
    [self.tableView registerClass:[UITableViewCell class]
           forCellReuseIdentifier:@"celluleID"];

    /* Chargement de la liste depuis le serveur */
    [self chargerListeDepuisServeur];
}

#pragma mark - Réseau (arrière-plan)

- (void)chargerListeDepuisServeur {
    [self.spinner startAnimating];

    /* Copies locales pour le bloc (évite de capturer self inutilement) */
    NSString *hostname = self.hostname;
    NSString *classe   = self.classe;
    NSString *date     = self.date;
    int       port     = self.port;

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{

        int sockfd = connecter_serveur([hostname UTF8String], port);
        if (sockfd < 0) {
            dispatch_async(dispatch_get_main_queue(), ^{
                [self.spinner stopAnimating];
                [self afficherErreur:@"Impossible de se connecter au serveur."];
            });
            return;
        }

        ListeEleves liste;
        liste.nb_eleves = 0;
        recuperer_liste(sockfd, [classe UTF8String], [date UTF8String], &liste);
        close(sockfd);

        dispatch_async(dispatch_get_main_queue(), ^{
            self.liste = liste;
            [self.spinner stopAnimating];
            [self.tableView reloadData];
        });
    });
}

- (void)sauvegarderListeSurServeur {
    NSString   *hostname = self.hostname;
    NSString   *classe   = self.classe;
    NSString   *date     = self.date;
    int         port     = self.port;
    ListeEleves liste    = self.liste;

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        int sockfd = connecter_serveur([hostname UTF8String], port);
        if (sockfd < 0) return;
        envoyer_appel(sockfd, [classe UTF8String], [date UTF8String],
                      (ListeEleves *)&liste);
        close(sockfd);
    });
}

#pragma mark - UITableViewDataSource

- (NSInteger)tableView:(UITableView *)tableView
 numberOfRowsInSection:(NSInteger)section {
    return self.liste.nb_eleves;
}

- (UITableViewCell *)tableView:(UITableView *)tableView
         cellForRowAtIndexPath:(NSIndexPath *)indexPath {

    UITableViewCell *cell =
        [tableView dequeueReusableCellWithIdentifier:@"celluleID"
                                        forIndexPath:indexPath];

    /* Nom de l'élève */
    cell.textLabel.text = [NSString stringWithUTF8String:
                           self.liste.eleves[indexPath.row].nom];
    cell.selectionStyle = UITableViewCellSelectionStyleNone;

    /* UISwitch — réutilisation si la cellule a déjà un switch (pattern du prof) */
    UISwitch *sw;
    if ([cell.accessoryView isKindOfClass:[UISwitch class]]) {
        sw = (UISwitch *)cell.accessoryView;
    } else {
        sw = [[UISwitch alloc] init];
        [sw addTarget:self
               action:@selector(switchChange:)
     forControlEvents:UIControlEventValueChanged];
        cell.accessoryView = sw;
    }

    sw.tag = indexPath.row;
    sw.on  = (self.liste.eleves[indexPath.row].present == 1);

    return cell;
}

#pragma mark - Action switch

- (void)switchChange:(UISwitch *)sender {
    NSInteger index = sender.tag;

    /* Mise à jour locale de la liste */
    self.liste.eleves[index].present = sender.isOn ? 1 : 0;

    printf("L'étudiant %s est %s\n",
           self.liste.eleves[index].nom,
           sender.isOn ? "PRESENT" : "ABSENT");

    /* Envoi immédiat au serveur */
    [self sauvegarderListeSurServeur];
}

#pragma mark - Retour

- (void)retour {
    /* On sauvegarde une dernière fois avant de revenir */
    [self sauvegarderListeSurServeur];
    [self.delegate blueViewControllerDidFinish:self];
    [self dismissViewControllerAnimated:YES completion:nil];
}

#pragma mark - Erreur

- (void)afficherErreur:(NSString *)message {
    UIAlertController *alert =
        [UIAlertController alertControllerWithTitle:@"Erreur"
                                           message:message
                                    preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"OK"
                                              style:UIAlertActionStyleDefault
                                            handler:nil]];
    [self presentViewController:alert animated:YES completion:nil];
}

@end