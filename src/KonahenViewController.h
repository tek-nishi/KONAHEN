//
//  KonahenViewController.h
//  Konahen
//
//  Created by nishi on 11/09/21.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//
#pragma once

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <GameKit/GameKit.h>

#import <OpenGLES/EAGL.h>

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#import "EAGLView.h"
#import "iad.h"


@interface KonahenViewController : UIViewController<GKLeaderboardViewControllerDelegate, ADBannerViewDelegate>
{
  IBOutlet UIButton *gameCenter;
  IBOutlet UIButton *twitter;
  IBOutlet UIButton *capture;
  NSString *category;                               // Leaderboard表示カテゴリ

  EAGLContext *context;
  GLuint program;
    
  BOOL animating;
  NSInteger animationFrameInterval;
  CADisplayLink *displayLink;

  float scale_;
  float ckp_scale_;

  unsigned int frame_count;
  unsigned int mouse_frame;

  BOOL execCapture;

  NSString *tweetText;
  UIImage  *tweetImage;
  BOOL doCapture;

  ADBannerView *adBanner;
}
@property (nonatomic, retain) UIButton *gameCenter;
@property (nonatomic, retain) UIButton *twitter;
@property (nonatomic, retain) UIButton *capture;
@property (nonatomic, copy) NSString *category;
@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;
@property (nonatomic, retain) NSString *tweetText;
@property (nonatomic, retain) UIImage *tweetImage;
@property (nonatomic, assign) BOOL doCapture;
@property (nonatomic, retain) ADBannerView *adBanner;

-(void)suspendAudio;
-(void)processAudio;

- (void)writeSettings;

- (void)startAnimation;
- (void)stopAnimation;

#ifndef VER_LITE
- (IBAction)showBord;
- (void)leaderboardViewControllerDidFinish:(GKLeaderboardViewController *)viewController;
#endif

- (IBAction)tweet;
-(void)screenShot;

- (IBAction)btnCapture;

@end
