// 
// Twitter関連
//
#import <string>
#import <cstdlib>
#import <Twitter/Twitter.h>
#import "Twitter.h"
#import "co_defines.hpp"


static KonahenViewController *topViewController;
static BOOL useTwitter = NO;
static bool TweetReady = false;
// FIXME:汚い…


// 初期化(iOSが古いのもチェック)
void twitterInit(KonahenViewController *ptr)
{
	topViewController = ptr;
	
	Class clazz = NSClassFromString(@"TWTweetComposeViewController");
	if (clazz)
	{
		NSLOG(@"Tweet OK!");
		useTwitter = YES;
	}
}

BOOL isUseTwitter()
{
	return useTwitter && [TWTweetComposeViewController canSendTweet];
}

// ツイートするテキストを設定
void setTweetText(const std::string& text)
{
	if (!isUseTwitter()) return;

	NSString *str = [[[NSString alloc] initWithCString:text.c_str() encoding:NSUTF8StringEncoding] autorelease];
	topViewController.tweetText = str;

	TweetReady = true;
}

// 添付画像を設定
void setTweetImage()
{
	if (!isUseTwitter()) return;

	topViewController.doCapture = YES;
}


// ボタンを表示する
void DispTweetBtn()
{
	if (!isUseTwitter() || !TweetReady) return;
	
	topViewController.twitter.hidden = NO;
	topViewController.twitter.enabled = YES;
	NSLOG(@"DispTweetBtn()");
}

// ボタンを消す
void HideTweetBtn()
{
	if (!isUseTwitter()) return;
	
	topViewController.twitter.hidden = YES;
	topViewController.twitter.enabled = NO;
	NSLOG(@"HideTweetBtn()");
}


#if 0

void twitterExec(const char *text)
{
	if (!useTwitter && ![TWTweetComposeViewController canSendTweet]) return;
	
	TWTweetComposeViewController *viewController = [[[TWTweetComposeViewController alloc] init] autorelease];

	NSString *str = [[[NSString alloc] initWithCString:text encoding:NSUTF8StringEncoding] autorelease];
	[viewController setInitialText:str];
	// [viewController addImage:image];
	// [viewController addURL:[NSURL URLWithString:@"http://itunes.apple.com/jp/app/konahen-lite-quan-fang-wei/id529753100?mt=8"]];

#if 0
	viewController.completionHandler = ^(TWTweetComposeViewControllerResult res) {
		if (res == TWTweetComposeViewControllerResultDone) {
			NSLOG(@"done");
			// 自分でdismissする
		} else if (res == TWTweetComposeViewControllerResultCancelled) {
			NSLOG(@"cancel");
			// 自分でdismissする
		}
	};
#endif

	[topViewController presentModalViewController:viewController animated:YES];
}

#endif
