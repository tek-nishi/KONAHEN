// 
// GameCenter関連
//
#pragma once

#if (TARGET_OS_IPHONE) && !defined(VER_LITE)

#import "KonahenViewController.h"

extern void StoreViewController(KonahenViewController *ptr);
extern KonahenViewController *GetViewController();
extern void LoginGameCenter();
extern void SendScoreToGameCenter(int mode, int score, int konahen);
extern void DispGameCenerBtn();
extern void HideGameCenerBtn();

#else

#define StoreViewController(ptr)
#define LoginGameCenter()
#define SendScoreToGameCenter(mode, score, konahen)
#define DispGameCenerBtn()
#define HideGameCenerBtn()

#endif
