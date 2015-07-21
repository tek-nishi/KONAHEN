
#import <GameKit/GameKit.h>
#import "KonahenAppDelegate.h"
#import "KonahenViewController.h"
#import "GameCenter.h"
#import "Twitter.h"
#import "co_defines.hpp"


@implementation KonahenAppDelegate

@synthesize window;
@synthesize viewController;


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	[UIApplication sharedApplication].idleTimerDisabled = YES;

	StoreViewController(viewController);
	twitterInit(viewController);
	LoginGameCenter();
	
	window.rootViewController = self.viewController;
  [window makeKeyAndVisible];

	return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application
{
	// HOMEをダブルクリックした時など、アクティブから非アクティブへ移行しようとするタイミングで呼ばれる
	// ロックとかSMSとか着信とかバックグラウンドに行く時とか
	NSLOG(@"applicationWillResignActive:");
	[self.viewController stopAnimation];
	[self.viewController suspendAudio];
	[UIApplication sharedApplication].idleTimerDisabled = NO;
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
	// アプリケーションがアクティブになった時に呼ばれる
	NSLOG(@"applicationDidBecomeActive:");
	[self.viewController startAnimation];
	[self.viewController processAudio];
	[UIApplication sharedApplication].idleTimerDisabled = YES;
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	// バックグラウンドで動作しない設定のアプリが終了しようとするタイミングで呼ばれる
	NSLOG(@"applicationWillTerminate:");
	[self.viewController stopAnimation];
	[UIApplication sharedApplication].idleTimerDisabled = NO;
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
	// アプリがバックグラウンドになった時に呼ばれる。
	// データ保存や不要なタイマーを無効にしておいてね
	NSLOG(@"applicationDidEnterBackground:");
	[self.viewController writeSettings];
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
	// バックグランドからアクティブになる時、呼ばれる
	NSLOG(@"applicationWillEnterForeground:");
}


- (void)dealloc
{
	[UIApplication sharedApplication].idleTimerDisabled = NO;

	[viewController release];
	[window release];
    
	[super dealloc];
}

@end
