
#pragma once

//
// Sound音量変更
//

#include <iostream>
#include <string>
#include "co_audio.hpp"
#include "co_graph.hpp"
#include "co_task.hpp"
#include "nn_gameenv.hpp"
#include "nn_widgets_map.hpp"


namespace ngs {

class SoundVol : public TaskProc {
	GameEnv& env_;
	bool active_;

	picojson::object& params_;

	WidgetsMap::WidgetPtr sound_widget_;
	WidgetsMap::WidgetPtr on_widget_;
	WidgetsMap::WidgetPtr off_widget_;

	float delay_;
	bool inout_;
	bool menu_open_;
	bool bar_disp_;
	float menu_ofs_;

	int ease_mode_;
	float time_cur_, time_ed_;
	Vec3<float> vec_, vec_st_, vec_ed_;
	float bar_alpha_, bar_alpha_st_, bar_alpha_ed_;
	float col_;

	bool draw_;
	bool touched_;

public:
	explicit SoundVol(GameEnv& env) :
		env_(env),
		active_(true),
		params_(env.params->value().get<picojson::object>()["title"].get<picojson::object>()),
		delay_(params_["sound_delay"].get<double>()),
		inout_(),
		menu_open_(),
		bar_disp_(),
		menu_ofs_(params_["sound_menu_ofs"].get<double>()),
		ease_mode_(QUART_OUT),
		time_cur_(),
		time_ed_(1.5),
		vec_(),
		vec_st_(100, 0, 0),
		vec_ed_(0, 0, 1),
		bar_alpha_(),
		bar_alpha_st_(),
		bar_alpha_ed_(),
		col_(1),
		draw_(),
		touched_()
	{
		DOUT << "SoundVol()" << std::endl;
		const WidgetsMap widgets(params_["sound_widgets"].get<picojson::object>(), env.texMng, *env.path, env.size, env.y_bottom);
		sound_widget_ = widgets.get("sound");
		on_widget_ = widgets.get("sound_on");
		off_widget_ = widgets.get("sound_off");

		TouchMenu *tm = static_cast<TouchMenu *>(env_.touchMenu.get());
		{
			using namespace std::tr1;
			tm->add(sound_widget_, bind(&SoundVol::btnSound, this,
																	placeholders::_1, placeholders::_2, placeholders::_3));

			tm->add(off_widget_, bind(&SoundVol::btnBar, this,
																placeholders::_1, placeholders::_2, placeholders::_3));
		}
	}
	~SoundVol() {
		DOUT << "~SoundVol()" << std::endl;
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

		if (inout_)
		{
			Easing easing;
			easing.ease(vec_, time_cur_, vec_st_, vec_ed_, time_ed_, BACK_OUT);
			easing.ease(bar_alpha_, time_cur_, bar_alpha_st_, bar_alpha_ed_, time_ed_, ease_mode_);
			if (time_cur_ >= time_ed_)
			{
				inout_ = false;
				if (!menu_open_) bar_disp_ = false;
			}
		}
		else
		{
			Easing easing;
			easing.ease(vec_, time_cur_, vec_st_, vec_ed_, time_ed_, ease_mode_);
		}

		if (touched_ && (time_cur_ >= time_ed_))
		{
			active_ = false;
		}
	}
	
	void draw()
	{
		if (!draw_) return;

		{
			const Vec2<float>& pos = sound_widget_->posOrig();
			sound_widget_->setPos(pos.x + vec_.x, pos.y + vec_.y);
			sound_widget_->setCol(col_, col_, col_, vec_.z);
			sound_widget_->draw();
		}

		if (bar_disp_)
		{
			const Vec2<float>& size = on_widget_->sizeOrig();
			const float gain = env_.sound->getGain();
			float x_on = size.x * gain;

			{
				const Vec2<float>& pos = off_widget_->posOrig();
				off_widget_->setPos(pos.x + vec_.x, pos.y + vec_.y);
				off_widget_->setCol(1, 1, 1, bar_alpha_ * vec_.z);
				off_widget_->draw();
			}
			
			{
				const Vec2<float>& pos = on_widget_->posOrig();
				on_widget_->setPos(pos.x + vec_.x, pos.y + vec_.y);
				on_widget_->setSize(x_on, size.y);
				on_widget_->setCol(1, 1, 1, bar_alpha_ * vec_.z);
				on_widget_->draw();
			}
		}
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

		case MSG_TITLE_SOUND_VOL_OPEN:
		case MSG_TITLE_SOUND_VOL_CLOSE:
			{
				vec_st_ = vec_;
				vec_ed_.set(menu_open_ ? 0 : menu_ofs_, 0, 1);
				bar_alpha_st_ = bar_alpha_;
				bar_alpha_ed_ = menu_open_ ? 0 : 1;
				time_cur_ = 0;
				time_ed_ = 0.5;
				menu_open_ = !menu_open_;
				if (menu_open_) bar_disp_ = true;
				
				inout_ = true;
			}
			break;
	
		case MSG_TITLE_CREDITS_PUSH:
		case MSG_TITLE_GAMESTART:
		case MSG_TITLE_END:
			if (!touched_)
			{
				ease_mode_ = QUART_IN;
				time_cur_ = 0.0;
				time_ed_ = 0.5;
				vec_st_ = vec_;
				vec_ed_.set(100, 0, 0);
				col_ = 1.0;

				touched_ = true;
			}
			break;
		}
	}
	
	void btnSound(const int type, const Vec2<float>& pos, Widget& widget)
	{
		if (!draw_ || touched_) return;

		switch (type)
		{
		case TouchMenu::CANCEL_IN:
			{
				col_ = 1.0;
				env_.task->sendMsgAll(menu_open_ ? MSG_TITLE_SOUND_VOL_CLOSE : MSG_TITLE_SOUND_VOL_OPEN);
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

	void btnBar(const int type, const Vec2<float>& pos, Widget& widget)
	{
		if (!draw_ || touched_ || !menu_open_) return;

		const Vec2<float>& wpos = widget.dispPos();
		const Vec2<float>& wsize = widget.sizeOrig();

		float gain = (pos.x - wpos.x) / wsize.x;
		if (gain < 0.0f) gain = 0.0f;
		if (gain > 1.0f) gain = 1.0f;
		env_.sound->setGain(gain);
		if ((type == TouchMenu::CANCEL_IN) || (type == TouchMenu::CANCEL_OUT))
		{
			env_.sound->play("touch_in");
		}
	}
	
};

}
