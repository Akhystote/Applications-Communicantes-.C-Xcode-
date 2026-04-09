/* OrangeViewController.h */
#import <UIKit/UIKit.h>
#import "BlueViewController.h"

@interface OrangeViewController : UIViewController <BlueViewControllerDelegate>

@property (weak, nonatomic) IBOutlet UIButton    *boutonValider;
@property (weak, nonatomic) IBOutlet UITextField *champDate;
@property (weak, nonatomic) IBOutlet UITextField *champClasse;

@end
