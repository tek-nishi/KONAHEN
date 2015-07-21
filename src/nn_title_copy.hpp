
#pragma once

//
// Copyright表示
//

#include <iostream>
#include <string>
#include "co_graph.hpp"
#include "co_task.hpp"
#include "nn_gameenv.hpp"
#include "nn_widget.hpp"
#include "nn_widgets_map.hpp"


namespace ngs {

class TitleCopy : public TaskProc {
	GameEnv& env_;
	bool active_;

	picojson::object& params_;

	WidgetsMap::WidgetPtr widget_;
	
	bool draw_;
	float delay_;

	float time_, time_ed_;
	float alpha_, alpha_st_, alpha_ed_;

	bool finish_;

public:
	explicit TitleCopy(GameEnv& env) :
		env_(env),
		active_(true),
		params_(env.params->value().get<picojson::object>()["title"].get<picojson::object>()),
		draw_(),
		delay_(params_["copy_delay"].get<double>()),
		time_(),
		time_ed_(params_["copy_in_time"].get<double>()),
		alpha_(),
		alpha_st_(),
		alpha_ed_(1),
		finish_()
	{
		DOUT << "TitleCopy()" << std::endl;
		const WidgetsMap widgets(params_["copy_widgets"].get<picojson::object>(), env.texMng, *env.path, env.size, env.y_bottom);
		widget_ = widgets.get("copyright");
	}
	
	~TitleCopy() {
		DOUT << "~TitleCopy()" << std::endl;
	}

	bool active() const { return active_; }
	
	void step(const float delta_time)
	{
		if(delay_ > 0.0)  draw_ = (delay_ -= delta_time) <= 0.0;
		if (!draw_) return;

		time_ += delta_time;
		Easing easing;
		easing.ease(alpha_, time_, alpha_st_, alpha_ed_, time_ed_, CUBIC_OUT);

		if (finish_ && (time_ >= time_ed_)) active_ = false;
	}
	
	void draw()
	{
		if(!draw_) return;

		widget_->setCol(1, 1, 1, alpha_);
		widget_->draw();
	}

	void msg(const int msg)
	{
		switch (msg)
		{
		case MSG_TITLE_INTRO_SKIP:
			{
				delay_ = 0;
				draw_ = true;
				time_ = 100;
			}
			break;

		case MSG_TITLE_END:
		case MSG_TITLE_GAMESTART:
			if (!finish_)
			{
				time_ = 0;
				time_ed_ = params_["copy_out_time"].get<double>();
				alpha_st_ = alpha_;
				alpha_ed_ = 0;
				finish_ = true;
			}
			break;
		}
	}
};

}
