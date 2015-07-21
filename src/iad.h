//
// iAD関連
//
#pragma once

#if (TARGET_OS_IPHONE)
#import <iAD/iAD.h>
#endif

#if (TARGET_OS_IPHONE) && defined(VER_LITE)

extern ADBannerView *createBanner(UIInterfaceOrientation orientation);
extern CGSize getBannerSize(UIInterfaceOrientation orientation);
extern void fixupAdView(ADBannerView *view, UIInterfaceOrientation orientation);
extern void getFullVersion();

#else

#define getFullVersion()

#endif
