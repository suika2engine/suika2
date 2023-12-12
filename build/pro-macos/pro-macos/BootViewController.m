#import "BootViewController.h"

#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

@interface BootViewController ()

@end

@implementation BootViewController

- (void)viewDidLoad {
    [super viewDidLoad];
}

- (void)viewDidAppear {
}

- (IBAction)createProjectButton:(id)sender {
    
}

- (IBAction)openExistingProjectButton:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    if (@available(macOS 11.0, *)) {
        [panel setAllowedContentTypes:@[[UTType typeWithFilenameExtension:@"suika2project" conformingToType:UTTypeData]]];
    } else {
        [panel setAllowedFileTypes:@[@"suika2project"]];
    }
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    
    NSString *dir = [[[panel URL] URLByDeletingLastPathComponent] path];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    [fileManager changeCurrentDirectoryPath:dir];
}

@end
