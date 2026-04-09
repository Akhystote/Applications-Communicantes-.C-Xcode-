/* OrangeViewController.m */
#import "OrangeViewController.h"
#import "BlueViewController.h"
#import "client.h"

/* IP et port du serveur — à adapter selon l'environnement */
#define SERVEUR_HOST "127.0.0.1"
#define SERVEUR_PORT  5001

@interface OrangeViewController ()
@end

@implementation OrangeViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.title = @"Gestion des présences";

    /* Date du jour par défaut */
    NSDateFormatter *fmt = [[NSDateFormatter alloc] init];
    [fmt setDateFormat:@"yyyy-MM-dd"];
    self.champDate.text = [fmt stringFromDate:[NSDate date]];

    /* Fermer le clavier en tapant ailleurs */
    UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc]
                                   initWithTarget:self
                                   action:@selector(fermerClavier)];
    [self.view addGestureRecognizer:tap];
}

- (void)fermerClavier {
    [self.view endEditing:YES];
}

- (IBAction)validerClasse:(id)sender {
    NSString *classe = [self.champClasse.text
                        stringByTrimmingCharactersInSet:
                        [NSCharacterSet whitespaceCharacterSet]];
    NSString *date   = [self.champDate.text
                        stringByTrimmingCharactersInSet:
                        [NSCharacterSet whitespaceCharacterSet]];

    if (classe.length == 0 || date.length == 0) {
        UIAlertController *alert =
            [UIAlertController alertControllerWithTitle:@"Champs manquants"
                                               message:@"Veuillez saisir la classe et la date."
                                        preferredStyle:UIAlertControllerStyleAlert];
        [alert addAction:[UIAlertAction actionWithTitle:@"OK"
                                                  style:UIAlertActionStyleDefault
                                                handler:nil]];
        [self presentViewController:alert animated:YES completion:nil];
        return;
    }

    [self performSegueWithIdentifier:@"gotoBlueID" sender:self];
}

#pragma mark - Navigation

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    if ([segue.identifier isEqualToString:@"gotoBlueID"]) {
        BlueViewController *blueVC = segue.destinationViewController;
        // Transmet les données
        blueVC.classe   = self.champClasse.text;
        blueVC.date     = self.champDate.text;
        // info serveur
        blueVC.hostname = @SERVEUR_HOST;
        blueVC.port     = SERVEUR_PORT;
        blueVC.delegate = self;
    }
}

#pragma mark - BlueViewControllerDelegate

/* Appelé par le BlueViewController quand il a terminé */
- (void)blueViewControllerDidFinish:(BlueViewController *)controller {
    [self dismissViewControllerAnimated:YES completion:nil];
}

@end
