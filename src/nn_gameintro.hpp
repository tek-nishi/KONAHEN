
#pragma once

//
// 開始演出
//

#include <iostream>
#include <string>
#include "co_task.hpp"
#include "nn_gameenv.hpp"
#include "nn_widget.hpp"
#include "nn_widgets_map.hpp"


namespace ngs {

class GameIntro : public TaskProc {
	GameEnv& env_;
	bool active_;

	picojson::object& params_;
	const picojson::array& array_;
	const TexMng::tex_ptr texture_;
	std::vector<WidgetsMap::WidgetPtr> widgets_;

	FntMng::FontPtr font_;
	const int height_;
	const float y_ofs_;
	const float nextline_;

	struct TextDisp {
		const std::string *text;
		Vec2<float> pos;
	};
	std::vector<TextDisp> disp_;
	Vec2<float> size_;

	bool ease_;
	EaseArray<Vec2<float> > easeArray_;
	Vec2<float> vct_;
	float time_;

	float alpha_;
	float alpha_cur_;
	
	float delay_;
	float frame_;
	bool draw_;

public:
	GameIntro(GameEnv& env) :
		env_(env),
		active_(true),
		params_(env.params->value().get<picojson::object>()["gameintro"].get<picojson::object>()),
		array_(params_["text"].get<picojson::array>()),
		texture_(env.texMng->read(*env.path + "devdata/round.png")),
		font_(ReadFont("place", *env.fonts, *env.path, env.params->value().get<picojson::object>())),
		height_(font_->height()),
		y_ofs_(params_["y_ofs"].get<double>()),
		nextline_(params_["nextline"].get<double>()),
		ease_(true),
		time_(),
		alpha_(params_["alpha"].get<double>()),
		alpha_cur_(1.0f),
		delay_(params_["delay"].get<double>()),
		frame_(params_["frame"].get<double>()),
		draw_()
	{
		DOUT << "GameIntro()" << std::endl;
		if (env_.demo)
		{
			delay_ = 0.0;
			draw_ = true;
			frame_ = 0.0;

			return;
			// デモプレイの場合は即時終了
		}

		char const *tbl[] = {
			"touch",
			"l_drag",
			"r_drag",
			"rotate_body",
			"zoom_body",
			"konahen_body",
			"rotate",
			"zoom",
			"konahen",
		};
		const WidgetsMap widgets(params_["widgets"].get<picojson::object>(), env.texMng, *env.path, env.size, env.y_bottom);
		for (int i = 0; i < elemsof(tbl); ++i)
		{
			widgets_.push_back(widgets.get(tbl[i]));
		}
		
		// 事前に表示領域を計算しておく
		float max_x = 0;
		float max_y = -10;
		float y = (array_.size() * (height_ + nextline_)) / -2.0;
		for (picojson::array::const_iterator it = array_.begin(); it != array_.end(); ++it)
		{
			const std::string& text = env.localize->get(it->get<std::string>());

			const Vec2<float>& size = font_->size(text);
			float x = size.x / -2.0;
			if (max_x < size.x) max_x = size.x;

			TextDisp disp = { &text, Vec2<float>(x, y) };
			disp_.push_back(disp);
			
			y += height_ + nextline_;
			max_y += height_ + nextline_;
		}
		size_.set(max_x + 40, max_y + 40 + 210);				// 操作説明分だけ縦サイズを伸ばす

		{
			picojson::array& array = params_["easing"].get<picojson::array>();
			EasingArayVec2(QUAD_OUT, array, easeArray_);
		}

		// プレイ回数で表示時間を調整
		{
			picojson::array& array = env_.settings->json().value().get<picojson::object>()["playnum"].get<picojson::array>();
			float num = 0;
			for (picojson::array::iterator it = array.begin(); it != array.end(); ++it)
			{
				num += it->get<double>();
			}
			const float frame_num = params_["frame_num"].get<double>();
			const float frame_add = params_["frame_add"].get<double>();

			if (num > frame_num) num = frame_num;					// 一定回以上は無視
			frame_ += frame_add * (frame_num - num) / frame_num;
			DOUT << "Play num:" << num << "," << frame_ << std::endl;
		}


		{
			Task::ProcPtr t = env_.task->add<FadeLight>(TASK_PRIO_3D, env_);
			FadeLight *fl = static_cast<FadeLight *>(t.get());
			fl->type(QUAD_OUT);
			fl->scale(params_["light"].get<double>());
			fl->time(params_["light_st"].get<double>());
			fl->delay(params_["light_st_delay"].get<double>());
		}
		env_.sound->play("unlock", 0.8, 0.3);
	}

	~GameIntro() {
		DOUT << "~GameIntro()" << std::endl;
	}

	bool active() const { return active_; }

	void step(const float delta_time)
	{
		draw_ = !((delay_ > 0.0) && (delay_ -= delta_time) > 0.0);
		if (draw_)
		{
			if (ease_)
			{
				time_ += delta_time;
				Vec2<float> res;
				ease_ = easeArray_.ease(vct_, time_);
			}
			
			frame_ -= delta_time;
			if (frame_ < alpha_)
			{
				alpha_cur_ = frame_ / alpha_;
			}

			
			if (frame_ < 0.0)
			{
				env_.task->sendMsgAll(MSG_GAME_INTRO_END);

				{
					env_.task->sendMsgAll(MSG_FADELIGHT_STOP);
					Task::ProcPtr t = env_.task->add<FadeLight>(TASK_PRIO_3D, env_);
					FadeLight *fl = static_cast<FadeLight *>(t.get());
					fl->type(QUAD_OUT);
					fl->scale(1.0);
					fl->time(params_["light_ed"].get<double>());
					fl->delay(params_["light_ed_delay"].get<double>());
				}
				active_ = false;
			}
		}
	}

	void draw()
	{
		if (!draw_) return;

		glPushMatrix();

#if (TARGET_OS_IPHONE)
		glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
#endif

		glScalef(vct_.x, vct_.x, 1.0);
		glTranslatef(0, y_ofs_, 0);

		GrpRoundBox obj;
		obj.pos(0, -height_ - 3 + 105);									// 操作説明で伸びている
		obj.size(size_);
		obj.center();
		obj.texture(texture_);
		obj.col(0, 0, 0, 0.4 * vct_.y * alpha_cur_);
		obj.draw();

		int index = 0;
		for(std::vector<TextDisp>::const_iterator it = disp_.begin(); it != disp_.end(); ++it, ++index)
		{
			const Vec2<float>& pos = it->pos;
			const std::string& text = *(it->text);
			font_->pos(pos);
			font_->center(0, 0);
			font_->scale(1.0f, 1.0f);

			if (index == 1) font_->col(1, 0, 0, vct_.y * alpha_cur_);
			else font_->col(1, 1, 1, vct_.y * alpha_cur_);
			
			font_->draw(text);
		}

		for (std::vector<WidgetsMap::WidgetPtr>::iterator it =  widgets_.begin(); it != widgets_.end(); ++it)
		{
			GrpCol<float> col = (*it)->colOrig();
			col.a *= vct_.y * alpha_cur_;
			(*it)->setCol(col);
			(*it)->draw();
		}

#if (TARGET_OS_IPHONE)
		glPopClientAttrib();
#endif

		glPopMatrix();
	}
	
	void msg(const int msg) {}

};

}
