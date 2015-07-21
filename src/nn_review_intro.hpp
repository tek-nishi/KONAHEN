
#pragma once

//
// 復習モード開始演出
//

#include <iostream>
#include <string>
#include "co_task.hpp"
#include "nn_gameenv.hpp"
#include "nn_agree.hpp"


namespace ngs {

class ReviewIntro : public TaskProc {
	GameEnv& env_;
	bool active_;

	picojson::object& params_;
	const TexMng::tex_ptr texture_;

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
	EaseArray<Vec2<float> > ease_in_;
	EaseArray<Vec2<float> > ease_out_;
	EaseArray<Vec2<float> > *easing_;
	Vec2<float> vct_;
	float time_;

	bool finish_;

public:
	ReviewIntro(GameEnv& env) :
		env_(env),
		active_(true),
		params_(env.params->value().get<picojson::object>()["reviewintro"].get<picojson::object>()),
		texture_(env.texMng->read(*env.path + "devdata/round.png")),
		font_(ReadFont("place", *env.fonts, *env.path, env.params->value().get<picojson::object>())),
		height_(font_->height()),
		y_ofs_(params_["y_ofs"].get<double>()),
		nextline_(params_["nextline"].get<double>()),
		ease_(true),
		easing_(&ease_in_),
		time_(),
		finish_()
	{
		DOUT << "ReviewIntro()" << std::endl;
		
		{
			const picojson::array& array = params_["text"].get<picojson::array>();
			float max_x = 0;
			float max_y = -10;
			float y = (array.size() * (height_ + nextline_)) / -2.0;
			for (picojson::array::const_iterator it = array.begin(); it != array.end(); ++it)
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
			size_.set(max_x + 40, max_y + 40);
			// 事前に表示領域を計算しておく
		}
		
		{
			picojson::array& array = params_["ease_in"].get<picojson::array>();
			EasingArayVec2(QUAD_OUT, array, ease_in_);
		}
		{
			picojson::array& array = params_["ease_out"].get<picojson::array>();
			EasingArayVec2(QUAD_OUT, array, ease_out_);
		}

		{
			Task::ProcPtr t = env_.task->add<FadeLight>(TASK_PRIO_3D, env_);
			FadeLight *fl = static_cast<FadeLight *>(t.get());
			fl->type(QUAD_OUT);
			fl->scale(params_["light"].get<double>());
			fl->time(params_["light_st"].get<double>());
			fl->delay(params_["light_st_delay"].get<double>());
			// 地球を暗くする演出を設定
		}
		env_.sound->play("unlock", 0.8, 0.6);
	}

	~ReviewIntro() {
		DOUT << "~ReviewIntro()" << std::endl;
	}

	bool active() const { return active_; }

	void step(const float delta_time)
	{
		if (ease_)
		{
			time_ += delta_time;
			ease_ = easing_->ease(vct_, time_);
			if (!ease_)
			{
				if (!finish_)
				{
					env_.task->add<Agree>(TASK_PRIO_2D, env_);
					// 確認ボタンを表示
				}
				else
				{
					env_.task->sendMsgAll(MSG_REVIEW_START);
					active_ = false;
					// タスクを完了
				}
			}
		}
	}

	void draw()
	{
		if (!(vct_.y > 0.0f)) return;
		
		glPushMatrix();

#if (TARGET_OS_IPHONE)
		glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
#endif

		glScalef(vct_.x, vct_.x, 1.0);
		glTranslatef(0, y_ofs_, 0);

		GrpRoundBox obj;
		obj.pos(0, -height_ - 3);
		obj.size(size_);
		obj.center();
		obj.texture(texture_);
		obj.col(0, 0, 0, 0.4 * vct_.y);
		obj.draw();

		for(std::vector<TextDisp>::const_iterator it = disp_.begin(); it != disp_.end(); ++it)
		{
			const Vec2<float>& pos = it->pos;
			const std::string& text = *(it->text);

			font_->pos(pos);
			font_->center(0, 0);
			font_->scale(1.0f, 1.0f);
			font_->col(1, 1, 1, vct_.y);
			font_->draw(text);
		}

#if (TARGET_OS_IPHONE)
		glPopClientAttrib();
#endif

		glPopMatrix();
	}
	
	void msg(const int msg)
	{
		switch (msg)
		{
		case MSG_GAME_CLEANUP:
			{
				active_ = false;
			}
			break;
			
		case MSG_GAME_AGREE:
			{
				finish_ = true;
				ease_ = true;
				time_ = 0;
				easing_ = &ease_out_;
				
				env_.task->sendMsgAll(MSG_FADELIGHT_STOP);
				Task::ProcPtr t = env_.task->add<FadeLight>(TASK_PRIO_3D, env_);
				FadeLight *fl = static_cast<FadeLight *>(t.get());
				fl->type(QUAD_OUT);
				fl->scale(1.0);
				fl->time(params_["light_ed"].get<double>());
				fl->delay(params_["light_ed_delay"].get<double>());
				// 地球の明るさを元に戻す
			}
			break;
		}
	}

};

}
