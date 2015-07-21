
#import <UIKit/UIKit.h>

@class KonahenViewController;

@interface KonahenAppDelegate : NSObject <UIApplicationDelegate> {
  UIWindow *window;
  KonahenViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet KonahenViewController *viewController;

@end

