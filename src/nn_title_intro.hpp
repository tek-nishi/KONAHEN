
#pragma once

//
// タイトル画面イントロ
//

#include <iostream>
#include <string>
#include "co_graph.hpp"
#include "co_task.hpp"
#include "co_easearray.hpp"
#include "nn_gameenv.hpp"
#include "nn_widgets_map.hpp"


namespace ngs {

class TitleIntro : public TaskProc, TouchCallBack {
	GameEnv& env_;
	bool active_;

	picojson::object& params_;

	std::vector<WidgetsMap::WidgetPtr> widget_;

	bool draw_;
	float delay_;
	Vec2<float> alpha_;

	EaseArray<Vec2<float> > easing_;
	float time_;

	bool firstexec_;
	bool touched_;

public:
	explicit TitleIntro(GameEnv& env) :
		env_(env),
		active_(true),
		params_(env.params->value().get<picojson::object>()["title"].get<picojson::object>()),
		draw_(),
		delay_(params_["intro_delay"].get<double>()),
		alpha_(),
		time_(),
		firstexec_(env_.settings->get<bool>("firstexec")),
		touched_()
	{
		DOUT << "TitleIntro()" << std::endl;

		EasingArayVec2(LINEAR, params_["intro_easing"].get<picojson::array>(), easing_);

		const WidgetsMap widgets(params_[env.localize->get("intro_widgets")].get<picojson::object>(), env_.texMng, *env_.path, env_.size, env_.y_bottom);
		picojson::array& list = params_[env.localize->get("intro_list")].get<picojson::array>();
		for (picojson::array::iterator it = list.begin(); it != list.end(); ++it)
		{
			WidgetsMap::WidgetPtr widget = widgets.get(it->get<std::string>());
			widget_.push_back(widget);
		}

		env_.touch->resistCallBack(this);
	}
	~TitleIntro() {
		DOUT << "~TitleIntro()" << std::endl;
		env_.touch->removeCallBack(this);
	}

	bool active() const { return active_; }
	
	void step(const float delta_time)
	{
		if(delay_ > 0.0)
		{
			draw_ = (delay_ -= delta_time) <= 0.0;
			if (!draw_) return;
		}

		time_ += delta_time;
		bool active = easing_.ease(alpha_, time_) && !touched_;
		if (!active)
		{
			active_ = false;
			env_.task->sendMsgAll(MSG_TITLE_INTRO_FIN);
			if (touched_)
			{
				env_.task->sendMsgAll(MSG_TITLE_INTRO_SKIP);
			}
		}
	}
	
	void draw()
	{
		if (!draw_) return;

		float tbl[] = {
			alpha_.x, alpha_.y, alpha_.y, alpha_.y
		};
		
		int idx = 0;
		for (std::vector<WidgetsMap::WidgetPtr>::iterator it = widget_.begin(); it != widget_.end(); ++it, ++idx)
		{
			(*it)->setCol(1, 1, 1, tbl[idx]);
			(*it)->draw();
		}
	}

	void msg(const int msg)
	{
		switch (msg)
		{
		case MSG_TITLE_END:
			{
				active_ = false;
			}
			break;
		}
	}


	void touchStart(const Touch& touch, const std::vector<TouchInfo>& info) {}
	void touchMove(const Touch& touch, const std::vector<TouchInfo>& info) {}
	void touchEnd(const Touch& touch, const std::vector<TouchInfo>& info)
	{
		if (draw_ && !firstexec_) touched_ = true;
	}
	
};

}
