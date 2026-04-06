#import "BlueViewController.h"

@implementation BlueViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    [self.tableView registerClass:[UITableViewCell class]
          forCellReuseIdentifier:@"celluleID"];
}

- (NSInteger)tableView:(UITableView *)tableView
 numberOfRowsInSection:(NSInteger)section {
    return self.receivedEtudiants.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView
         cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell =
        [tableView dequeueReusableCellWithIdentifier:@"celluleID"
                                       forIndexPath:indexPath];

    // Nom de l'étudiant
    cell.textLabel.text = self.receivedEtudiants[indexPath.row];

    // Réutilise le switch si la cellule est recyclée, sinon en crée un
    UISwitch *switchControl;
    if ([cell.accessoryView isKindOfClass:[UISwitch class]]) {
        switchControl = (UISwitch *)cell.accessoryView;
    } else {
        switchControl = [[UISwitch alloc] init];
        [switchControl addTarget:self
                         action:@selector(switchChanged:)
               forControlEvents:UIControlEventValueChanged];
        cell.accessoryView = switchControl;
    }

    // Toujours mettre à jour tag et état
    switchControl.tag = indexPath.row;
    switchControl.on = [self.presents[indexPath.row] boolValue];

    return cell;
}

// Appelée quand on bascule un switch
- (void)switchChanged:(UISwitch *)sender {
    NSInteger index = sender.tag;
    self.presents[index] = @(sender.isOn);
    printf("L'étudiant %s est %s\n",
        [self.receivedEtudiants[index] UTF8String],
        [self.presents[index] isEqual:@(YES)] ? "présent" : "absent");
}

@end

