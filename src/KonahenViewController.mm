
#import <AudioToolbox/AudioToolbox.h>
#import <Twitter/Twitter.h>
#import "KonahenViewController.h"
#import "Twitter.h"
#import <tr1/memory>
#import <tr1/functional>
#import <string>
#import "co_defines.hpp"
#import "co_glex.hpp"
#import "nn_app.hpp"
#import "nn_capture.hpp"

using namespace ngs;


@interface KonahenViewController ()
@property (nonatomic, retain) EAGLContext *context;
@property (nonatomic, assign) CADisplayLink *displayLink;
@end


static App *app;
// FIXME:atexit()で delete する必要があるので、メンバ変数では保持できない


static NSString *GetDocumentPath()
{
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	return [paths objectAtIndex:0];
}


// CGDataProviderRefから呼ばれるコールバック
static void bufferFree(void *info, const void *data, size_t size)
{
	NSLOG(@"bufferFree()");
	std::free(const_cast<void *>(data));
	// FIXME:これは痛い…
}

// スクリーンショット
static UIImage *createScreenShot(const int width, const int height)
{
	NSInteger myDataLength = width * height * 4;
	GLubyte *buffer = (GLubyte *)std::malloc(myDataLength);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	GLubyte *buffer2 = (GLubyte *)std::malloc(myDataLength);
	for (int y = 0; y < height; ++y)
	{
		std::memcpy(&buffer2[((height - 1) - y) * width * 4], &buffer[y * 4 * width], sizeof(GLubyte) * width * 4);
	}
	free(buffer);
	// FIXME:上下をひっくり返す。この処理が勿体ない。

	CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, buffer2, myDataLength, bufferFree);
	CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
	CGImageRef imageRef = CGImageCreate(width, height, 8, 32, 4 * width, colorSpaceRef, kCGBitmapByteOrderDefault, provider, NULL, NO, kCGRenderingIntentDefault);
	UIImage *image = [UIImage imageWithCGImage:imageRef];

	CGImageRelease(imageRef);
	CGColorSpaceRelease(colorSpaceRef);
	CGDataProviderRelease(provider);

	return image;
}


#ifdef _DEBUG

std::string capturePath;

// tmpディレクトリにキャプチャー用のディレクトリを生成
static void getCapturePath()
{ 
	NSString *dirName = NSTemporaryDirectory();

	NSMutableString *saveFileDirPath = [[[NSMutableString alloc] init] autorelease];
	[saveFileDirPath appendString:dirName];
	[saveFileDirPath appendString:@"capture/"];

	NSFileManager *fileManager = [NSFileManager defaultManager];
	BOOL isYES = YES;
	BOOL isExist = [fileManager fileExistsAtPath:saveFileDirPath isDirectory:&isYES];

	NSLOG(@"Capture path:%@", saveFileDirPath);
	if (!isExist)
	{ 
		[fileManager changeCurrentDirectoryPath:dirName];
		[fileManager createDirectoryAtPath:@"capture" withIntermediateDirectories:YES attributes:nil error:nil];
		NSLOG(@"Create Capture path:%@", saveFileDirPath);
	}

	capturePath = [saveFileDirPath UTF8String];
}
#endif


static void mainFin()
{
	delete app;
}

static void createTouchInfo(std::vector<TouchInfo>& info, NSSet *const touches, UIView *const view, const float scale)
{
	for (UITouch *touch in [touches allObjects])
	{
		CGPoint pos = [touch locationInView:view];
		CGPoint l_pos = [touch previousLocationInView:view];
		TouchInfo obj = {
			Vec2<float>(pos.x * scale, pos.y * scale),
			Vec2<float>(l_pos.x * scale, l_pos.y * scale)
		};
		info.push_back(obj);
	}
}


// AudioSettionカテゴリ設定
static void SetupAudioCategory()
{
	UInt32 cat = kAudioSessionCategory_AmbientSound;
	AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(UInt32), &cat);
}


@implementation KonahenViewController

@synthesize gameCenter;
@synthesize twitter;
@synthesize capture;
@synthesize category;
@synthesize animating, context, displayLink;
@synthesize tweetText, tweetImage, doCapture;
@synthesize adBanner;


-(void)suspendAudio
{
	NSLOG(@"suspendAudio");

	AudioSessionSetActive(NO);
	alcMakeContextCurrent(0);
	alcSuspendContext(app->audioContext());
}

-(void)processAudio
{
	NSLOG(@"processAudio");
		
	SetupAudioCategory();
	AudioSessionSetActive(YES);
	if (app)
	{
		alcMakeContextCurrent(app->audioContext());
		alcProcessContext(app->audioContext());
	}
	// iOS5.1だとAudioSessionInitialize()のコールバックでこの設定するとエラーになる
}


- (void)setupBannerLayout:(UIInterfaceOrientation)interfaceOrientation
{
#ifdef VER_LITE
	CGRect view_bounds = self.view.bounds;
	CGSize banner_size = getBannerSize(interfaceOrientation);
  
  CGRect ad_bounds = adBanner.frame;
  ad_bounds.origin.y = view_bounds.size.height - banner_size.height;
  adBanner.frame = ad_bounds;

	fixupAdView(self.adBanner, interfaceOrientation);

  // バナーのサイズを変更したらアプリ内のオフセットを変更
  ad_bounds = adBanner.frame;
  app->y_bottom(ad_bounds.size.height * (scale_ / ckp_scale_));
#endif
}


- (void)awakeFromNib
{
	NSLOG(@"awakeFromNib");

	// 他のアプリの音の再生を許可
	AudioSessionInitialize(NULL, NULL, NULL, NULL);
	SetupAudioCategory();

	// OpenGL初期化
	EAGLContext *aContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
	if (!aContext)
	{
		NSLOG(@"Failed to create ES context");
	}
	else if (![EAGLContext setCurrentContext:aContext])
	{
		NSLOG(@"Failed to set ES context current");
	}
  
	self.context = aContext;
	[aContext release];

	[(EAGLView *)self.view setContext:context];
	[(EAGLView *)self.view setFramebuffer];
    
	animating = FALSE;
	animationFrameInterval = 1;
	self.displayLink = nil;

	if ([UIView instancesRespondToSelector:@selector(contentScaleFactor)])
	{
		scale_ = [UIScreen mainScreen].scale;
		self.view.contentScaleFactor = scale_;
		// TIPS:Retina対応
	}
	else
	{
		scale_ = 1.0;
	}
	bool retina = scale_ > 1.0f;

	std::string path = [[[NSBundle mainBundle] resourcePath] UTF8String] + std::string("/");
	std::string savePath = [GetDocumentPath() UTF8String] + std::string("/");
	std::string lang = [NSLocalizedString(@"langFile", 0) UTF8String];
	DOUT << lang << std::endl;

	initGlexFunc();

	CGRect bounds = self.view.bounds;
	NSLOG(@"Display:%d x %d", (int)(bounds.size.width * scale_), (int)(bounds.size.height * scale_));	
	ckp_scale_ = 1.0;
	if (bounds.size.width * scale_ < 640)
	{
		ckp_scale_ = 0.5;
		// iPhone3G
	}
	else if (bounds.size.width * scale_ > 1024)
	{
		ckp_scale_ = 2.0;
		// iPad Retina
	}
	int width = bounds.size.width * scale_;
	int height = bounds.size.height * scale_;
	app = new App(width, height, ckp_scale_, path, savePath, lang);
	app->retina(retina);
	app->reset();
	app->resize(width, height);
	atexit(mainFin);

	glHint(GL_FOG_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	// ↑たいてい無視されている模様:P

	glDisable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0f);
	// 最初に設定してしまえばいいもの

#ifdef VER_LITE
	{
    // iAdバナー生成
    UIInterfaceOrientation interfaceOrientation = self.interfaceOrientation;
		ADBannerView *adView = createBanner(interfaceOrientation);
		self.adBanner = adView;

    // 初期設定
		adView.autoresizingMask = UIViewAutoresizingNone;
		adView.hidden = YES;
		adView.delegate = self;

		[self.view addSubview:adView];
    [self setupBannerLayout:interfaceOrientation];
	}
#endif
	
#ifdef _DEBUG
	self.capture.hidden = NO;
	getCapturePath();
#endif
}

- (void)viewDidLoad
{
	NSLOG(@"viewDidLoad");
	AudioSessionSetActive(YES);	
}


- (void)dealloc
{
	if (program)
	{
		glDeleteProgram(program);
		program = 0;
	}
    
	// Tear down context.
	if ([EAGLContext currentContext] == context)
	[EAGLContext setCurrentContext:nil];

	self.adBanner.delegate = nil;

	[context release];
	self.gameCenter = nil;
	self.twitter = nil;
	self.capture = nil;
	self.category = nil;
	self.tweetText = nil;
	self.tweetImage = nil;
	self.adBanner = nil;
    
	[super dealloc];
}


- (void)writeSettings
{
	app->writeSettings();
}


- (void)viewWillAppear:(BOOL)animated
{
	NSLOG(@"viewWillAppear");
	[self startAnimation];
#ifdef VER_LITE
	[self willAnimateRotationToInterfaceOrientation:self.interfaceOrientation duration:0];
#endif
    
	[super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
	NSLOG(@"viewWillDisappear");
	[self stopAnimation];
    
	[super viewWillDisappear:animated];
}

- (void)viewDidUnload
{
	[super viewDidUnload];
	
	if (program)
	{
		glDeleteProgram(program);
		program = 0;
	}

	// Tear down context.
	if ([EAGLContext currentContext] == context)
	[EAGLContext setCurrentContext:nil];
	self.context = nil;	
}


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	return YES;
}

-(void)willAnimateRotationToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation duration:(NSTimeInterval)duration
{
	NSLOG(@"willAnimateRotationToInterfaceOrientation:%d", interfaceOrientation);

  // iAdバナーのレイアウト変更
  [self setupBannerLayout:interfaceOrientation];

	// [super willAnimateRotationToInterfaceOrientation:interfaceOrientation duration:duration];
	// FIXME:呼ぶの？呼ばないの？
}

#if 0
-(void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
}
#endif


// タップ開始イベント
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	std::vector<TouchInfo> info;
	createTouchInfo(info, touches, self.view, scale_);
	app->touch().touchStart(info);
}

// タップ中イベント
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	if (frame_count == mouse_frame) return;
	// 同一フレームで二回以上処理しない
	
	std::vector<TouchInfo> info;
	createTouchInfo(info, touches, self.view, scale_);
	app->touch().touchMove(info);
	if (mouse_frame == frame_count) NSLOG(@"Same:touchesMoved()");
	mouse_frame = frame_count;
}

// タップ終了イベント
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	std::vector<TouchInfo> info;
	createTouchInfo(info, touches, self.view, scale_);
	app->touch().touchEnd(info);
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	std::vector<TouchInfo> info;
	createTouchInfo(info, touches, self.view, scale_);
	app->touch().touchEnd(info);
}


- (NSInteger)animationFrameInterval
{
	return animationFrameInterval;
}

- (void)setAnimationFrameInterval:(NSInteger)frameInterval
{
	/*
		Frame interval defines how many display frames must pass between each time the display link fires.
		The display link will only fire 30 times a second when the frame internal is two on a display that refreshes 60 times a second. The default frame interval setting of one will fire 60 times a second when the display refreshes at 60 times a second. A frame interval setting of less than one results in undefined behavior.
	*/
	if (frameInterval >= 1)
	{
		animationFrameInterval = frameInterval;
        
		if (animating)
		{
			[self stopAnimation];
			[self startAnimation];
		}
	}
}

- (void)startAnimation
{
	if (!animating)
	{
		CADisplayLink *aDisplayLink;
		if ([UIScreen instancesRespondToSelector:@selector(displayLinkWithTarget:selector:)])
		{
			aDisplayLink = [[UIScreen mainScreen] displayLinkWithTarget:self selector:@selector(drawFrame)];
		}
		else
		{
			aDisplayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawFrame)];
		}

		// CADisplayLink *aDisplayLink = [[UIScreen mainScreen] displayLinkWithTarget:self selector:@selector(drawFrame)];
		[aDisplayLink setFrameInterval:animationFrameInterval];
		[aDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
		self.displayLink = aDisplayLink;
        
		animating = TRUE;
	}
}

- (void)stopAnimation
{
	if (animating)
	{
		[self.displayLink invalidate];
		self.displayLink = nil;
		animating = FALSE;
	}
}

- (void)drawFrame
{
	[(EAGLView *)self.view setFramebuffer];

  // iOS6のiPadを横にした状態でアプリを起動すると
  // willAnimateRotationToInterfaceOrientation が呼ばれない
  // そのため、画面サイズの調整はここで行う必要がある
	CGRect bounds = self.view.bounds;
	app->resize(bounds.size.width * scale_, bounds.size.height * scale_);

	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT);
	app->update(1.0 / 60.0);
	++frame_count;

	if (self.doCapture)
	{
		self.doCapture = NO;
		[self screenShot];
		// FIXME:ここで実行しないとちゃんとキャプチャできない(苦肉の策)
	}

#ifdef _DEBUG
	const GLenum gl_err = glGetError();
	if (gl_err != GL_NO_ERROR)
	{
		const GLubyte *str = gluErrorString(gl_err);
		DOUT << "OpenGL Error:" << str << std::endl;
	}
	const ALenum al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		const char *str = AudioErrorString(al_err);
		DOUT << "OpenAL Error:" << str << std::endl;
	}

	if (execCapture)
	{
		ExecCapture(capturePath, app->windowWidth(), app->windowHeight());
	}
#endif
    
	[(EAGLView *)self.view presentFramebuffer];
}

- (void)didReceiveMemoryWarning
{
	// Releases the view if it doesn't have a superview.
	[super didReceiveMemoryWarning];
    
	// Release any cached data, images, etc. that aren't in use.
}


#ifndef VER_LITE

//リーダーボードを立ち上げる
-(IBAction)showBord
{
	if (self.gameCenter.isHidden) return;
	// タップした状態でhiddenになってもコールバックが呼ばれてしまうための措置
	
	GKLeaderboardViewController *leaderboardController = [[[GKLeaderboardViewController alloc] init] autorelease];
	if (leaderboardController != nil)
	{
		if (self.category) leaderboardController.category = self.category;
		leaderboardController.leaderboardDelegate = self;
		[self presentModalViewController: leaderboardController animated: YES];
	}
}

//リーダーボードで完了を押した時に呼ばれる
- (void)leaderboardViewControllerDidFinish:(GKLeaderboardViewController *)viewController
{
	self.category = viewController.category;
	[self dismissModalViewControllerAnimated:YES];
}

#endif


// プレイ結果をツイートする
-(IBAction)tweet
{
	if (self.twitter.isHidden) return;
	// タップした状態でhiddenになってもコールバックが呼ばれてしまうための措置

	if (isUseTwitter())
	{
		TWTweetComposeViewController *viewController = [[[TWTweetComposeViewController alloc] init] autorelease];

		[viewController setInitialText:tweetText];
		[viewController addImage:tweetImage];
		[viewController addURL:[NSURL URLWithString:@"http://goo.gl/2Zgn8"]];
	
		[self presentModalViewController:viewController animated:YES];
	}
}

// スクリーンショット
-(void)screenShot
{
	UIImage *image = createScreenShot(app->windowWidth(), app->windowHeight());
	// スクリーンショット

	CGSize size = image.size;
	CGFloat w = (size.width > size.height) ? 480.0 : 320.0;
	CGFloat h = size.height * (w / size.width);

	UIImage *cap_image = image;
	if (size.width > w)
	{
		UIGraphicsBeginImageContext(CGSizeMake(w, h));
		CGContextSetInterpolationQuality(UIGraphicsGetCurrentContext(), kCGInterpolationHigh);
		// 縮小のクオリティを再設定
		[image drawInRect:CGRectMake(0, 0, w, h)];
		cap_image = UIGraphicsGetImageFromCurrentImageContext();
		UIGraphicsEndImageContext();
		// 大きな場合は縮小する
	}
	self.tweetImage = cap_image;
}


#ifdef VER_LITE

// iAd読み込み完了
- (void)bannerViewDidLoadAd:(ADBannerView *)banner
{
  DOUT << "bannerViewDidLoadAd:" << std::endl;
	banner.hidden = NO;

  // バナーのレイアウトを再設定
  // iOS6のiPadでは、横画面でアプリを起動すると
  // willAnimateRotationToInterfaceOrientation:　が呼び出されない
  [self setupBannerLayout:self.interfaceOrientation];
}
 
- (void)bannerView:(ADBannerView *)banner didFailToReceiveAdWithError:(NSError *)error
{
  DOUT << "bannerView:didFailToReceiveAdWithError:" << std::endl;
	banner.hidden = YES;
}

#endif


// 画面キャプチャー
- (IBAction)btnCapture
{
#ifdef _DEBUG
	execCapture = !execCapture;
	app->forceFrame(execCapture);
	if (execCapture) StartCapture();
#endif
}


@end
