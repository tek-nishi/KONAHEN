//
// iAD関連
//
#import "iad.h"

#ifdef VER_LITE

ADBannerView *createBanner(UIInterfaceOrientation orientation)
{
  ADBannerView *view = [[[ADBannerView alloc] initWithFrame:CGRectMake(0,0,0,0)] autorelease];
  view.currentContentSizeIdentifier = UIInterfaceOrientationIsPortrait(orientation) ? ADBannerContentSizeIdentifierPortrait : ADBannerContentSizeIdentifierLandscape;

  return view;
}

CGSize getBannerSize(UIInterfaceOrientation orientation)
{
  NSString *identifier = UIInterfaceOrientationIsPortrait(orientation) ? ADBannerContentSizeIdentifierPortrait : ADBannerContentSizeIdentifierLandscape;
  return [ADBannerView sizeFromBannerContentSizeIdentifier:identifier];
}

void fixupAdView(ADBannerView *view, UIInterfaceOrientation orientation)
{
  view.currentContentSizeIdentifier = UIInterfaceOrientationIsPortrait(orientation) ? ADBannerContentSizeIdentifierPortrait : ADBannerContentSizeIdentifierLandscape;
}

// GET FULL
void getFullVersion()
{
  [[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"http://itunes.apple.com/jp/app/konahen-quan-fang-wei-xing/id529756904?mt=8"]];
}

#endif
