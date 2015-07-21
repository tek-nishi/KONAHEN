// 
// GameCenter関連
//
#import <GameKit/GameKit.h>
#import "GameCenter.h"


#ifndef VER_LITE

static KonahenViewController *topViewController;
// FIXME:汚い…

// アプリ起動時に呼び出す
void StoreViewController(KonahenViewController *ptr)
{
	topViewController = ptr;
}

KonahenViewController *GetViewController()
{
	return topViewController;
}


BOOL isGameCenterAPIAvailable()
{
	BOOL localPlayerClassAvailable = (NSClassFromString(@"GKLocalPlayer")) != nil;
  // GKLocalPlayerクラスが存在するかどうかをチェックする

	NSString *reqSysVer = @"4.1";
	NSString *currSysVer = [[UIDevice currentDevice] systemVersion]; BOOL osVersionSupported = ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending);
  // デバイスはiOS 4.1以降で動作していなければならない

	return (localPlayerClassAvailable && osVersionSupported);
}

// GameCenterへログイン
void LoginGameCenter()
{
	if (!isGameCenterAPIAvailable()) return;

	NSLog(@"Login GameCenter");
	GKLocalPlayer *localPlayer = [GKLocalPlayer localPlayer];
	[localPlayer authenticateWithCompletionHandler:^(NSError *error) {
#if 1
			if (error != nil)
			{
				NSLog(@"Login error %@", error);
			}
			
			if (localPlayer.isAuthenticated)
			{
				NSLog(@"Login OK!");
			}
#else
			if (error != nil)
			{
				NSLog(@"Login error %@", error);
			}
			else
			{
				NSLog(@"Login OK!");
			}
#endif
			topViewController.gameCenter.enabled = YES;
		}];
}

// スコア送信
void SendScoreToGameCenter(int mode, int score, int konahen)
{
	if (!isGameCenterAPIAvailable()) return;

	NSLog(@"Sent score to GameCenter");

	{
		NSString *score_id[] = {
			@"NGS0003KONAHEN.normal",
			@"NGS0003KONAHEN.advanced",
			@"NGS0003KONAHEN.survival",
		};
	
		GKScore *scoreReporter = [[[GKScore alloc] initWithCategory:score_id[mode]] autorelease];
		NSInteger value = score;
		scoreReporter.value = value;
		[scoreReporter reportScoreWithCompletionHandler:^(NSError *error) {
				if (error != nil)
				{
					NSLog(@"Sending score error %@", error);
				}
				else
				{
					NSLog(@"Sending score OK!");
				}
			}];

		topViewController.category = score_id[mode];
	}

	{
		NSString *konahen_id[] = {
			@"NGS0003KONAHEN.normal.konahen",
			@"NGS0003KONAHEN.advanced.konahen",
			@"NGS0003KONAHEN.survival.konahen",
		};
	
		GKScore *scoreReporter = [[[GKScore alloc] initWithCategory:konahen_id[mode]] autorelease];
		NSInteger value = konahen;
		scoreReporter.value = value;
		[scoreReporter reportScoreWithCompletionHandler:^(NSError *error) {
				if (error != nil)
				{
					NSLog(@"Sending score error %@", error);
				}
				else
				{
					NSLog(@"Sending score OK!");
				}
			}];
	}
}

// GameCenterのボタンを表示する
void DispGameCenerBtn()
{
	if (!isGameCenterAPIAvailable()) return;
	topViewController.gameCenter.hidden = NO;
}

// GameCenterのボタンを消す
void HideGameCenerBtn()
{
	if (!isGameCenterAPIAvailable()) return;
	topViewController.gameCenter.hidden = YES;
}

#endif
