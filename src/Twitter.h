//
// Twitter関連
//
#pragma once

#if (TARGET_OS_IPHONE)

#import "KonahenViewController.h"
#import <string>

extern void twitterInit(KonahenViewController *ptr);
extern BOOL isUseTwitter();

extern void setTweetText(const std::string& text);
extern void setTweetImage();

extern void DispTweetBtn();
extern void HideTweetBtn();

#else

#define setTweetText(text)
#define setTweetImage()
#define DispTweetBtn()
#define HideTweetBtn()

#endif
