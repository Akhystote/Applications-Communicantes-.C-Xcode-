/* BlueViewController.h */
#import <UIKit/UIKit.h>
#import "client.h"

@class BlueViewController;

/* Pattern delegate — exactement comme dans le tutoriel du prof */
@protocol BlueViewControllerDelegate <NSObject>
- (void)blueViewControllerDidFinish:(BlueViewController *)controller;
@end

@interface BlueViewController : UITableViewController

/* Données transmises depuis OrangeViewController via prepareForSegue */
@property (nonatomic, copy)   NSString *classe;
@property (nonatomic, copy)   NSString *date;
@property (nonatomic, copy)   NSString *hostname;
@property (nonatomic, assign) int       port;

/* Delegate — pattern vu dans le tutoriel MultipleViewControllers */
@property (nonatomic, weak) id<BlueViewControllerDelegate> delegate;

@end
