/* This file is part of DoIP. DoIP is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * DoIP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with DoIP.  If not, see <http://www.gnu.org/licenses/>. */
 
 #import "MWFavoritesTableViewController.h"
#import "MWEndpoint.h"
#import "MWMethod.h"
#import "MWClient.h"

static UIImage* favoriteImage;
static UIImage* favoriteAllImage;

@implementation MWFavoritesTableViewController
@synthesize favorites = _favorites;

#ifdef TARGET_IPAD
	@synthesize popOverController = _popOverController;
#endif

+ (UIImage*) favoriteImage {
	return favoriteImage;
}

+ (void) initialize {
	NSString* pngPath = [[NSBundle mainBundle] pathForResource:@"Favorite" ofType:@"png"];
	favoriteImage = [[UIImage imageWithContentsOfFile:pngPath] retain];
	
	pngPath = [[NSBundle mainBundle] pathForResource:@"FavoriteAll" ofType:@"png"];
	favoriteAllImage = [[UIImage imageWithContentsOfFile:pngPath] retain];
}

- (NSString*) persistentPath {
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES); 
	NSString *documentsDirectoryPath = [paths objectAtIndex:0];
	NSString *databaseFile = [documentsDirectoryPath stringByAppendingPathComponent:@"favorites.plist"];
	//[databaseFile autorelease];
	return databaseFile;
}
	
- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
}

- (void) persist {
	if(![NSKeyedArchiver archiveRootObject:self.favorites toFile:[self persistentPath]]) {
		NSLog(@"Archival failed..");
	}
}

- (void) addFavorite:(MWFavorite *)fav {
	[_favorites addObject:fav];
	[self.tableView reloadData];
	[self persist];
}

// Ensure that the view controller supports rotation and that the split view can therefore show in both portrait and landscape.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return YES;
}

- (void)viewWillAppear:(BOOL)animated {
}

- (void)viewDidLoad {
	[self.navigationItem setRightBarButtonItem:[self editButtonItem] animated:NO];
	
	self.favorites = [NSKeyedUnarchiver unarchiveObjectWithFile:[self persistentPath]];
	if(!self.favorites) {
		self.favorites = [[[NSMutableArray alloc] init] autorelease];
	}
	[self.tableView setSeparatorColor:[UIColor colorWithRed:1.0f green:1.0f blue:1.0f alpha:0.2f]];
	[super viewDidLoad];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

// Customize the number of rows in the table view.
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [_favorites count];
}


// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
    }
    
	MWFavorite* fav = [_favorites objectAtIndex:indexPath.row];
	cell.textLabel.text = [fav friendlyName];
	cell.textLabel.textColor = [UIColor whiteColor];
	if([fav specificDevice]) {
		cell.imageView.image = favoriteImage;
	}
	else {
		cell.imageView.image = favoriteAllImage;
	}
	
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    if(indexPath.section==0) {
		MWFavorite* fav = [_favorites objectAtIndex:indexPath.row];
		MWClient* client = [MWClient sharedInstance];

		bool success = false;
		for(MWEndpoint* ep in [client resolvedEndpoints]) {
			if(!fav.specificDevice || [ep.endpointIdentifier isEqualToString:fav.specificDevice]) {
				success = [ep executeFavorite:fav] || success;
				NSLog(@"Run favorite on ep=%@", ep.endpointIdentifier);
			}
		}
		
		if(!success) {
			NSString* msg;
			if(fav.specificDevice) {
				msg = @"The favorite action could not be executed, because the device for which it was created cannot be found.";
			}
			else {
				msg = @"The favorite action could not be executed, because there are no devices available that support the command.";
			}
			UIAlertView* av = [[UIAlertView alloc] initWithTitle:@"No supported devices found" message:msg delegate:nil cancelButtonTitle:nil otherButtonTitles:@"OK",nil];
			[av show];
			[av autorelease];			
		}
		
		[tableView deselectRowAtIndexPath:indexPath animated:YES];
	}
}

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source
		[_favorites removeObjectAtIndex:indexPath.row];
        [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:UITableViewRowAnimationRight];
		[self persist];
    }   
    else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}

- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
	NSObject* to = [[_favorites objectAtIndex:toIndexPath.row] retain];
	[_favorites replaceObjectAtIndex:toIndexPath.row withObject:[_favorites objectAtIndex:fromIndexPath.row]];
	[_favorites replaceObjectAtIndex:fromIndexPath.row withObject:to];
	[to release];
	[self persist];
}

- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}

#ifdef TARGET_IPAD
- (void)splitViewController: (UISplitViewController*)svc willHideViewController:(UIViewController *)aViewController withBarButtonItem:(UIBarButtonItem*)barButtonItem forPopoverController: (UIPopoverController*)pc {
    UINavigationBar* navigationBar = [[self navigationController] navigationBar];
	barButtonItem.title = @"Remote";
    [navigationBar.topItem setLeftBarButtonItem:barButtonItem animated:YES];
    self.popOverController = pc;
}


// Called when the view is shown again in the split view, invalidating the button and popover controller.
- (void)splitViewController: (UISplitViewController*)svc willShowViewController:(UIViewController *)aViewController invalidatingBarButtonItem:(UIBarButtonItem *)barButtonItem {
    UINavigationBar* navigationBar = [[self navigationController] navigationBar];
	[navigationBar.topItem setLeftBarButtonItem:nil animated:YES];
    self.popOverController = nil;
}
#endif

- (void)dealloc {
	[_favorites release];
    [super dealloc];
}

@end

