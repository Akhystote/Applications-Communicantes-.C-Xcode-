/* ViewController.h */
#import <UIKit/UIKit.h>

@interface ViewController : UIViewController

@property (weak, nonatomic) IBOutlet UITextField *classeTextField;
@property (weak, nonatomic) IBOutlet UITextView  *resultatTextView;

- (IBAction)envoyerClasse:(id)sender;

@end
