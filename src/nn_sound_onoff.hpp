
#pragma once

//
// Sound ON/OFF
//

#include <iostream>
#include <string>
#include "co_audio.hpp"
#include "co_graph.hpp"
#include "co_task.hpp"
#include "nn_gameenv.hpp"
#include "nn_widgets_map.hpp"


namespace ngs {

class SoundOnOff : public TaskProc {
	GameEnv& env_;
	bool active_;

	picojson::object& params_;

	WidgetsMap::WidgetPtr on_icon_;
	WidgetsMap::WidgetPtr off_icon_;

	EaseArray<Vec2<float> > ease_in_;
	EaseArray<Vec2<float> > ease_out_;
	EaseArray<Vec2<float> > *easing_;
	Vec2<float> vec_;
	bool ease_;
	float time_;

	u_int type_;
	bool mute_;
	float col_;

	bool finish_;

public:
	SoundOnOff(GameEnv& env, const u_int type) :
		env_(env),
		active_(true),
		params_(env.params->value().get<picojson::object>()["title"].get<picojson::object>()),
		ease_(true),
		time_(),
		type_(type),
		mute_(env_.sound->mute(type)),
		col_(1.0f),
		finish_()
	{
		DOUT << "SoundOnOff()" << std::endl;

		const char *tbl[] = {
			"bgm_onoff",
			"se_onoff",
		};

		picojson::object& param = params_[tbl[type]].get<picojson::object>();
		const WidgetsMap widgets(param["widgets"].get<picojson::object>(), env.texMng, *env.path, env.size, env.y_bottom);
		on_icon_ = widgets.get("on");
		off_icon_ = widgets.get("off");

		TouchMenu *tm = static_cast<TouchMenu *>(env_.touchMenu.get());
		using namespace std::tr1;
		tm->add(on_icon_, bind(&SoundOnOff::iconFunc, this,
													 placeholders::_1, placeholders::_2, placeholders::_3));
		
		EasingArayVec2(QUART_OUT, param["ease_in"].get<picojson::array>(), ease_in_);
		EasingArayVec2(QUART_IN, param["ease_out"].get<picojson::array>(), ease_out_);
		easing_ = &ease_in_;
	}

	~SoundOnOff() {
		DOUT << "~SoundOnOff()" << std::endl;
	}

	
	bool active() const { return active_; }
	
	void step(const float delta_time)
	{
		if (ease_)
		{
			time_ += delta_time;
			ease_ = easing_->ease(vec_, time_);
			if (!ease_ && finish_) active_ = false;
		}
	}
	
	void draw()
	{
		WidgetsMap::WidgetPtr icon = mute_ ? off_icon_ : on_icon_;
		
		icon->setCol(col_, col_, col_, 1.0f * vec_.y);
		Vec2<float> pos = icon->posOrig();
		icon->setPos(pos.x + vec_.x, pos.y);
		icon->draw();
	}

	void msg(const int msg)
	{
		switch (msg)
		{
		case MSG_TITLE_INTRO_SKIP:
			{
				time_ = 1000.0f;
				// イントロがスキップされたら表示演出もスキップ
			}
			break;
	
		case MSG_TITLE_CREDITS_PUSH:
		case MSG_TITLE_GAMESTART:
		case MSG_TITLE_END:
			if (!finish_)
			{
				ease_ = true;
				time_ = 0.0f;
				easing_ = &ease_out_;
				finish_ = true;
			}
			break;
		}
	}
	
	void iconFunc(const int type, const Vec2<float>& pos, Widget& widget)
	{
		if (finish_) return;
		
		switch (type)
		{
		case TouchMenu::CANCEL_IN:
			{
				mute_ = !mute_;
				env_.sound->mute(type_, mute_);
				// col_ = mute_ ? 0.4f : 1.0f;
				col_ = 1.0f;
			}
			break;

		case TouchMenu::TOUCH:
		case TouchMenu::MOVE_IN_EDGE:
			{
				col_ = 0.6f;
			}
			break;

		case TouchMenu::MOVE_OUT_EDGE:
		case TouchMenu::CANCEL_OUT:
			{
				// col_ = mute_ ? 0.4f : 1.0f;
				col_ = 1.0f;
			}
			break;
		}
	}
	
};

}
