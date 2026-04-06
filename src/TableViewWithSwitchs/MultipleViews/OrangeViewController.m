// ViewController.m
// Gestion interface et appel du client C

#import "OrangeViewController.h"
#import "Client.h"// important
#import "BlueViewController.h"

@implementation OrangeViewController

- (void)viewDidLoad {
    [super viewDidLoad];
}

// Action appelée quand on appuie sur Valider
- (IBAction)validerClasse:(id)sender {
    NSString *classe = self.champClasse.text;
    if (classe.length == 0) return;

    char *resultat = lancerClient("127.0.0.1", 5001, [classe UTF8String]);
    if (resultat == NULL) return;

    NSString *liste = [NSString stringWithUTF8String:resultat];

    // On découpe la NSString en NSArray (séparateur \n)
    NSArray *etudiants = [liste componentsSeparatedByString:@"\n"];

    UIStoryboard *storyboard = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
    BlueViewController *vc = [storyboard instantiateViewControllerWithIdentifier:@"BlueViewController"];

    // On passe le tableau et on initialise les présents (tous à YES)
    vc.receivedEtudiants = etudiants;
    vc.presents = [NSMutableArray array];
    for (int i = 0; i < etudiants.count; i++)
        [vc.presents addObject:@(YES)];

    [self presentViewController:vc animated:YES completion:nil];
}


@end
