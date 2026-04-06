//
//  BlueViewController.h
//  MultipleViews
//
//  Created by etudiant on 27/02/2026.
//

#import <UIKit/UIKit.h>

@interface BlueViewController : UIViewController<UITableViewDelegate,UITableViewDataSource>

// Zone qui affiche la liste des étudiants
@property (weak, nonatomic) IBOutlet UITextView *zoneTexte;

// Remplace NSString *liste
@property (copy, nonatomic) NSArray *receivedEtudiants;
@property (strong, nonatomic) NSMutableArray *presents;

@property (nonatomic, weak) IBOutlet UITableView *tableView;


@end
