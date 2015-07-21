
#pragma once

//
// Credits表示ボタン
//

#include <iostream>
#include <string>
#include "co_graph.hpp"
#include "co_task.hpp"
#include "nn_widgets_map.hpp"
#include "nn_gameenv.hpp"


namespace ngs {

class TitleCredits : public TaskProc {
	GameEnv& env_;
	bool active_;

	picojson::object& params_;

	WidgetsMap::WidgetPtr widget_;

	float delay_;

	int ease_mode_;
	float time_cur_, time_ed_;
	Vec3<float> vec_, vec_st_, vec_ed_;
	float col_;

	bool exec_credits_;
	bool draw_;
	bool touched_;

public:
	explicit TitleCredits(GameEnv& env) :
		env_(env),
		active_(true),
		params_(env.params->value().get<picojson::object>()["title"].get<picojson::object>()),
		delay_(params_["credits_delay"].get<double>()),
		ease_mode_(QUART_OUT),
		time_cur_(),
		time_ed_(1.5),
		vec_(),
		vec_st_(100, 0, 0),
		vec_ed_(0, 0, 1),
		col_(1),
		exec_credits_(),
		draw_(),
		touched_()
	{
		DOUT << "TitleCredits()" << std::endl;
		const WidgetsMap widgets(params_["credits_widgets"].get<picojson::object>(), env_.texMng, *env_.path, env_.size, env_.y_bottom);
		widget_ = widgets.get("credits");

		TouchMenu *tm = static_cast<TouchMenu *>(env_.touchMenu.get());
		tm->add(widget_, std::tr1::bind(&TitleCredits::btnCredits,
																		this, std::tr1::placeholders::_1, std::tr1::placeholders::_2, std::tr1::placeholders::_3));
	}
	~TitleCredits() {
		DOUT << "~TitleCredits()" << std::endl;
	}

	bool active() const { return active_; }
	
	void step(const float delta_time)
	{
		if (delay_ > 0.0)
		{
			draw_ = (delay_ -= delta_time) <= 0.0;
			if (!draw_) return;
		}
		
		if (time_cur_ < time_ed_) time_cur_ += delta_time;
		
		Easing easing;
		easing.ease(vec_, time_cur_, vec_st_, vec_ed_, time_ed_, ease_mode_);

		if (touched_ && (time_cur_ >= time_ed_))
		{
			active_ = false;
			if (exec_credits_)
			{
				env_.task->sendMsgAll(MSG_TITLE_CREDITS);
			}
		}
	}
	
	void draw()
	{
		if (!draw_) return;

		const Vec2<float>& pos_orig = widget_->posOrig();
		widget_->setPos(pos_orig.x + vec_.x, pos_orig.y + vec_.y);
		widget_->setCol(col_, col_, col_, vec_.z);
		widget_->draw();
	}

	void msg(const int msg)
	{
		switch (msg)
		{
		case MSG_TITLE_INTRO_SKIP:
			{
				draw_ = true;
				delay_ = 0.0;
				time_cur_ = 100;
			}
			break;

		case MSG_TITLE_CREDITS_PUSH:
			if (!touched_)
			{
				ease_mode_ = QUART_IN;
				time_cur_ = 0.0;
				time_ed_ = 0.5;
				vec_st_ = vec_;
//				vec_ed_.set(100, 0, 0);
				vec_ed_.set(0, 0, 0);
				env_.sound->play("touch_just1");
				exec_credits_ = true;
				touched_ = true;
			}
			break;
			
		case MSG_TITLE_GAMESTART:
		case MSG_TITLE_END:
			if (!touched_)
			{
				ease_mode_ = QUART_IN;
				time_cur_ = 0.0;
				time_ed_ = 0.7;
				vec_st_ = vec_;
				vec_ed_.set(100, 0, 0);
				col_ = 1.0;
				
				touched_ = true;
			}
			break;
		}
	}
	
	void btnCredits(const int type, const Vec2<float>& pos, Widget& widget)
	{
		if (!draw_ || touched_) return;

		switch (type)
		{
		case TouchMenu::CANCEL_IN:
			{
				col_ = 1.0;
				env_.task->sendMsgAll(MSG_TITLE_CREDITS_PUSH);
			}
			break;
			
		case TouchMenu::TOUCH:
		case TouchMenu::MOVE_IN_EDGE:
			{
				col_ = 0.5;
			}
			break;

		case TouchMenu::MOVE_OUT_EDGE:
		case TouchMenu::CANCEL_OUT:
			{
				col_ = 1.0;
			}
			break;
		}
	}
	
};

}
