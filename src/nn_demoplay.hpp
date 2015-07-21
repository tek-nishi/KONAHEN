
#pragma once

//
// DEMOリプレイ処理
//

#include <iostream>
#include <string>
#include "co_graph.hpp"
#include "co_task.hpp"
#include "nn_gameenv.hpp"
#include "nn_widget.hpp"
#include "nn_widgets_map.hpp"


namespace ngs {

class DemoPlay : public TaskProc, TouchCallBack
{
	GameEnv& env_;
	bool active_;

	picojson::object& params_;
	const picojson::array& files_;

	WidgetsMap::WidgetPtr widget_;
	
	float delay_;
	bool draw_;

	float time_;
	const float time_ed_;

	float alpha_;
	
	bool finish_;

public:
	explicit DemoPlay(GameEnv& env, Replay& replay) :
		env_(env),
		active_(true),
		params_(env.params->value().get<picojson::object>()["title"].get<picojson::object>()),
		files_(params_["demo_files"].get<picojson::array>()),
		delay_(params_["demo_delay"].get<double>()),
		draw_(),
		time_(),
		time_ed_(params_["demo_time"].get<double>()),
		alpha_(1),
		finish_()
	{
		DOUT << "DemoPlay()" << std::endl;
		const WidgetsMap widgets(params_["demo_widgets"].get<picojson::object>(), env_.texMng, *env_.path, env_.size, env_.y_bottom);
		widget_ = widgets.get("demoplay");

		env_.demo = true;

		replay.read(files_[env_.replay_num].get<std::string>());
		env_.replay_num = (env_.replay_num + 1) % files_.size();

		env_.game_mode = GAME_MODE_NORMAL;
		env_.task->add<GameMain>(TASK_PRIO_SYS, env_, replay);
		env_.touch->resistCallBack(this);
	}

	~DemoPlay() {
		DOUT << "~DemoPlay()" << std::endl;
		env_.touch->removeCallBack(this);
	}

	bool active() const { return active_; }
	
	void step(const float delta_time)
	{
		if (delay_ > 0.0)  draw_ = (delay_ -= delta_time) <= 0.0;
		if (!draw_) return;

		time_ += delta_time;
		alpha_ = (int)(time_ * 1.5f) & 0x1 ? 1 : 0;

		if (finish_ || (time_ >= time_ed_))
		{
			active_ = false;
			env_.task->sendMsgAll(MSG_GAME_CLEANUP);
			env_.task->sendMsgAll(MSG_DEMOPLAY_END);
		}
	}
	
	void draw()
	{
		if (!draw_) return;

		widget_->setCol(1, 1, 1, alpha_);
		widget_->draw();
	}

	void msg(const int msg) {}

	void touchStart(const Touch& touch, const std::vector<TouchInfo>& info) {}
	void touchMove(const Touch& touch, const std::vector<TouchInfo>& info) {}
	void touchEnd(const Touch& touch, const std::vector<TouchInfo>& info)
	{
		if (draw_ && !finish_)
		{
			finish_ = true;
		}
	}

};

}
