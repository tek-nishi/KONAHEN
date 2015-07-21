
#pragma once

//
// メニュー入力を一括して扱う
// Widgetをそのまま転用。shared_ptrで保持されている必要がある
// FIXME:ウィジットより先にコールバックのクラスが無くなると未定義になる
//

#include <iostream>
#include <list>
#include <algorithm>
#include "co_task.hpp"
#include "co_misc.hpp"
#include "co_touch.hpp"
#include "nn_widget.hpp"


namespace ngs {

class TouchObj
{
public:
	typedef std::tr1::function<void (const int, const Vec2<float>&, Widget&)> TouchFunc;
	
private:
	std::tr1::weak_ptr<Widget> widget_;
	TouchFunc func_;

public:
	TouchObj(std::tr1::shared_ptr<Widget> widget, TouchFunc func) :
		widget_(widget),
		func_(func)
	{}

	bool active() const
	{
		return !widget_.expired();
	}

	bool within(const Vec2<float>& pos, const float margin = 0.0f)
	{
		std::tr1::shared_ptr<Widget> widget = widget_.lock();
		const Vec2<float>& wpos = widget->dispPos();
		const Vec2<float>& wsize = widget->size();
		const Vec2<float>& wofs = widget->ofsSize();
		Box<float> box = {
			Vec2<float>(wpos.x - margin - (wofs.x / 2.0), wpos.y - margin - (wofs.y / 2.0)),
			Vec2<float>(wpos.x + wsize.x + margin + (wofs.x / 2.0), wpos.y + wsize.y + margin + (wofs.y / 2.0))
		};
		return crossPointBox(pos, box);
	}

	void callback(const int type, const Vec2<float>& pos)
	{
		func_(type, pos,  *(widget_.lock()));
	}
	
};


class TouchMenu : public TaskProc, TouchCallBack
{
public:
	enum {
		TOUCH,																					// 枠内でタッチ操作
		MOVE_IN_EDGE,																		// 枠外→枠内
		MOVE_IN,																				// 枠内でドラッグ
		MOVE_OUT_EDGE,																	// 枠内→枠外
		MOVE_OUT,																				// 枠外でドラッグ
		CANCEL_IN,																			// 枠内で離された
		CANCEL_OUT,																			// 枠外で離された
	};
	
private:
	Touch& touch_;
	bool active_;

	const float touch_margin_;
	
	std::list<TouchObj> objs_;												// ポインタ同士の比較をするので list を使用
	TouchObj *touched_;

public:
	TouchMenu(Touch& touch, const float margin) :
		touch_(touch),
		active_(true),
		touch_margin_(margin),
		touched_()
	{
		DOUT << "TouchMenu()" << std::endl;
		touch_.resistCallBack(this);
	}

	~TouchMenu()
	{
		DOUT << "~TouchMenu()" << std::endl;
		touch_.removeCallBack(this);
	}

	bool active() const { return active_; }

	void step(const float delta_time)
	{
		for(std::list<TouchObj>::iterator it = objs_.begin(); it != objs_.end(); /* none */)
		{
			if (!it->active())
			{
				if (touched_ == &(*it)) touched_ = 0;				// listならpush_back()などでポインタの位置は変わらない
				it = objs_.erase(it);
				DOUT << "TouchMenu removed." << std::endl;

				continue;
			}
			++it;
		}
	}

	void draw() {}
	void msg(const int msg) {}

	void touchStart(const Touch& touch, const std::vector<TouchInfo>& info)
	{
		Vec2<float> pos = touch.pos2local(info[0].pos);
		for(std::list<TouchObj>::iterator it = objs_.begin(); it != objs_.end(); ++it)
		{
			if (it->active())
			{
				if (it->within(pos))
				{
					touched_ = &(*it);
					it->callback(TOUCH, pos);
					break;
				}
			}
		}
	}

	void touchMove(const Touch& touch, const std::vector<TouchInfo>& info)
	{
		if (touched_)
		{
			if (!touched_->active())
			{
				touched_ = 0;
			}
			else
			{
				Vec2<float> pos = touch.pos2local(info[0].pos);
				Vec2<float> lpos = touch.pos2local(info[0].l_pos);

				bool within = touched_->within(pos, touch_margin_);
				bool lwithin = touched_->within(lpos, touch_margin_);
				touched_->callback(within ? (lwithin ? MOVE_IN : MOVE_IN_EDGE) : (lwithin ? MOVE_OUT_EDGE : MOVE_OUT), pos);
			}
		}
	}
	
	void touchEnd(const Touch& touch, const std::vector<TouchInfo>& info)
	{
		if (touched_)
		{
			if (touched_->active())
			{
				Vec2<float> pos = touch.pos2local(info[0].pos);
				touched_->callback(touched_->within(pos, touch_margin_) ? CANCEL_IN : CANCEL_OUT, pos);				
			}
			touched_ = 0;
		}
	}


	void add(std::tr1::shared_ptr<Widget> widget, TouchObj::TouchFunc func)
	{
		TouchObj obj(widget, func);
		objs_.push_back(obj);
	}
};

}
