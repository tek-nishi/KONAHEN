
#pragma once

//
// ゲーム開始操作
//

#include <iostream>
#include <string>
#include "co_graph.hpp"
#include "co_task.hpp"
#include "nn_gameenv.hpp"
#include "nn_widgets_map.hpp"
#include "nn_achievement.hpp"


namespace ngs {

class TitleStart : public TaskProc {
	GameEnv& env_;
	bool active_;

	picojson::object& params_;
	std::vector<WidgetsMap::WidgetPtr> widgets_;

	float delay_;
	float alpha_;
	float alpha_st_, alpha_ed_;
	float time_;
	float time_ed_;

	struct BtnScale {
		bool ease;
		float alpha;
		float alpha_st, alpha_ed;
		float time, time_ed;
		int mode;
		BtnScale() :
			ease(),
			alpha(1),
			alpha_st(),
			alpha_ed(),
			time(),
			time_ed(),
			mode()
		{}
	};
	std::vector<BtnScale> btn_scale_;

	
	bool started_;
	int pushed_num_;
	float push_time_;
	float push_time_ed_;
	Vec2<float> push_, push_st_, push_ed_;

	float fin_time_;
	
	bool draw_;
	bool touched_;

public:
	explicit TitleStart(GameEnv& env) :
		env_(env),
		active_(true),
		params_(env.params->value().get<picojson::object>()["title"].get<picojson::object>()),
		delay_(params_["start_delay"].get<double>()),
		alpha_(),
		alpha_st_(),
		alpha_ed_(1),
		time_(),
		time_ed_(1.0),
		started_(),
		pushed_num_(),
		push_time_(),
		push_time_ed_(params_["start_ease_time"].get<double>()),
		push_st_(1, 1),
		push_ed_(1.5, 0),
		fin_time_(),
		draw_(),
		touched_()
	{
		DOUT << "TitleStart()" << std::endl;

		const WidgetsMap widgets(params_["start_widgets"].get<picojson::object>(), env_.texMng, *env_.path, env_.size, env_.y_bottom);
		TouchMenu *tm = static_cast<TouchMenu *>(env_.touchMenu.get());
		BtnScale b;
		
		const bool advanced = isAchievementUnlock("advanced", env_);
		const bool survival = isAchievementUnlock("survival", env_);
		const bool review = isAchievementUnlock("review", env_, true);
#ifdef VER_LITE
		const bool getfull = isAchievementUnlock("getfull", env_, true);
#endif
		
		{
			using namespace std::tr1;

			WidgetsMap::WidgetPtr widget = widgets.get((advanced || survival) ? "normal" : "play");
			widgets_.push_back(widget);
			btn_scale_.push_back(b);
			tm->add(widget, bind(&TitleStart::btnGameStart, this,
													 placeholders::_1,
													 placeholders::_2,
													 placeholders::_3));
		}
		
		if (advanced)
		{
			using namespace std::tr1;

			WidgetsMap::WidgetPtr widget = widgets.get("advanced");
			widgets_.push_back(widget);
			btn_scale_.push_back(b);
			tm->add(widgets.get("advanced"), bind(&TitleStart::btnGameStart, this,
																						placeholders::_1,
																						placeholders::_2,
																						placeholders::_3));
		}

		if (survival)
		{
			using namespace std::tr1;

			WidgetsMap::WidgetPtr widget = widgets.get("survival");
			widgets_.push_back(widget);
			btn_scale_.push_back(b);
			tm->add(widgets.get("survival"), bind(&TitleStart::btnGameStart, this,
																						placeholders::_1,
																						placeholders::_2,
																						placeholders::_3));
		}

		if (review)
		{
			using namespace std::tr1;

			WidgetsMap::WidgetPtr widget = widgets.get("review");

			int num = widgets_.size();
			Vec2<float> pos = widget->pos();
			pos.y += 65 * num;
			widget->setPos(pos);
			// REVIEWボタンは常に一番下
			
			widgets_.push_back(widget);
			btn_scale_.push_back(b);
			tm->add(widgets.get("review"), bind(&TitleStart::btnGameStart, this,
																					placeholders::_1,
																					placeholders::_2,
																					placeholders::_3));
		}

#ifdef VER_LITE
		if (getfull)
		{
			using namespace std::tr1;

			WidgetsMap::WidgetPtr widget = widgets.get("getfull");

			int num = widgets_.size();
			Vec2<float> pos = widget->pos();
			pos.y += 65 * num;
			widget->setPos(pos);
			// GETFULLボタンは常に一番下

			widgets_.push_back(widget);
			btn_scale_.push_back(b);
			tm->add(widgets.get("getfull"), bind(&TitleStart::btnGameStart, this,
																					 placeholders::_1,
																					 placeholders::_2,
																					 placeholders::_3));
		}
#endif

		int num = widgets_.size();
		float ofs = (195.0f - (65.0f * (num - 1))) / 2.0f;
#ifdef VER_LITE
		ofs -= 15.0f;
#endif
		for (int i = 0; i < num; ++i)
		{
			Vec2<float> pos = widgets_[i]->pos();
			pos.y = int(pos.y + ofs);
			// 座標は切り捨てしないと表示がぼやける
			widgets_[i]->setPos(pos);
		}
		// ボタンの数に合わせて位置を調整
	}
	
	~TitleStart() {
		DOUT << "~TitleStart()" << std::endl;
	}

	bool active() const { return active_; }
	
	void step(const float delta_time)
	{
		if (delay_ > 0.0)
		{
			draw_ = (delay_ -= delta_time) <= 0;
			if (!draw_) return;
		}
		
		if (time_ < time_ed_) time_ += delta_time;

		Easing easing;
		easing.ease(alpha_, time_, alpha_st_, alpha_ed_, time_ed_, CUBIC_OUT);
		if (started_)
		{
			push_time_ += delta_time;
			easing.ease(push_, push_time_, push_st_, push_ed_, push_time_ed_, EXPO_IN);
		}

		for (std::vector<BtnScale>::iterator it = btn_scale_.begin(); it != btn_scale_.end(); ++it)
		{
			if (it->ease)
			{
				it->time += delta_time;
				it->ease = (it->time <= it->time_ed);
				easing.ease(it->alpha, it->time, it->alpha_st, it->alpha_ed, it->time_ed, it->mode);
			}
		}

		if (fin_time_ > 0) fin_time_ -= delta_time;
		if (touched_ && (fin_time_ <= 0.0))
		{
			active_ = false;
		}
	}
	
	void draw()
	{
		if (!draw_) return;

		int i = 0;
		for (std::vector<WidgetsMap::WidgetPtr>::iterator it = widgets_.begin(); it != widgets_.end(); ++it, ++i)
		{
			if (started_ && i == pushed_num_)
			{
				(*it)->setScale(push_.x * btn_scale_[i].alpha, push_.x * btn_scale_[i].alpha);
				(*it)->setCol(1.0, 1.0, 1.0, push_.y);
			}
			else
			{
				(*it)->setScale(1 * btn_scale_[i].alpha, 1 * btn_scale_[i].alpha);
				(*it)->setCol(1.0, 1.0, 1.0, alpha_);
			}
			
			(*it)->blend_mode(0);
			(*it)->draw();
		}
	}

	void msg(const int msg)
	{
		switch (msg)
		{
		case MSG_TITLE_INTRO_SKIP:
			{
				delay_ = 0.0;
				draw_ = true;
				time_ = 100;
			}
			break;

		case MSG_TITLE_CREDITS_PUSH:
		case MSG_TITLE_END:
			if (!touched_)
			{
				time_ = 0.0;
				alpha_st_ = alpha_;
				alpha_ed_ = 0;
				fin_time_ = time_ed_;
				touched_ = true;
			}
			break;

		case MSG_TITLE_GAMESTART:
			if (!touched_)
			{
				env_.sound->stopAll();
				env_.sound->play("start");
				time_ = 0.0;
				alpha_st_ = alpha_;
				alpha_ed_ = 0;

				fin_time_ = push_time_ed_;
				started_ = true;
				touched_ = true;
			}
			break;
		}
	}

	
	void btnGameStart(const int type, const Vec2<float>& pos, Widget& widget)
	{
		if (!draw_ || touched_) return;

		switch (type)
		{
		case TouchMenu::TOUCH:
		case TouchMenu::MOVE_IN_EDGE:
			{
				for (size_t i = 0; i < widgets_.size(); ++i)
				{
					Widget& w = *(widgets_[i]);
					if (&widget == &w)													// FIXME:この比較は安全なのか…？
					{
						BtnScale& b = btn_scale_[i];
						b.ease = true;
						b.alpha_st = b.alpha;
						b.alpha_ed = params_["start_btneft_scale"].get<double>();
						b.time = 0;
						b.time_ed = params_["start_btneft_time"].get<double>();
						b.mode = BACK_OUT;
						break;
					}
				}
			}
			break;

		case TouchMenu::CANCEL_IN: 
			{
				for (size_t i = 0; i < widgets_.size(); ++i)
				{
					Widget& w = *(widgets_[i]);
					if (&widget == &w)													// FIXME:この比較は安全なのか…？
					{
						BtnScale& b = btn_scale_[i];
						b.ease = false;
						pushed_num_ = i;
						break;
					}
				}

				const int id = widget.id();
				env_.game_mode = id + GAME_MODE_NORMAL;
				env_.task->sendMsgAll(MSG_TITLE_GAMESTART);
			}
			break;

		case TouchMenu::MOVE_OUT_EDGE:
		case TouchMenu::CANCEL_OUT: 
			{
				for (size_t i = 0; i < widgets_.size(); ++i)
				{
					Widget& w = *(widgets_[i]);
					if (&widget == &w)													// FIXME:この比較は安全なのか…？
					{
						BtnScale& b = btn_scale_[i];
						b.ease = true;
						b.alpha_st = b.alpha;
						b.alpha_ed = 1.0;
						b.time = 0;
						b.time_ed = params_["start_btneft_time"].get<double>();
						b.mode = BACK_OUT;
						break;
					}
				}
			}
			break;
		}
	}
	
};

}
