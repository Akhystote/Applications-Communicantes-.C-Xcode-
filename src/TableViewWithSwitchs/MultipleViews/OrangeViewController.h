// ViewController.h
// Contrôleur principal

#import <UIKit/UIKit.h>

@interface OrangeViewController : UIViewController

// Champ texte pour saisir la classe
@property (weak, nonatomic) IBOutlet UITextField *champClasse;

// Action du bouton Valider
- (IBAction)validerClasse:(id)sender;

@end

