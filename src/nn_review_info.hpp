
#pragma once

//
// 復習モード・各種情報
//


namespace ngs {

class ReviewInfo : public TaskProc
{
	GameEnv& env_;
	bool active_;

	picojson::object& params_;
	const TexMng::tex_ptr texture_;

	WidgetsMap::WidgetPtr widget_slash_;
	WidgetsMap::WidgetPtr widget_konahen_;
	WidgetsMap::WidgetPtr level_;
	WidgetsMap::WidgetPtr type_;
	std::map<std::string, WidgetsMap::WidgetPtr> types_;

	EaseArray<Vec3<float> > ease_konahen_;
	bool konahen_eft_;
	Vec3<float> konahen_ofs_;
	EaseArray<Vec2<float> > ease_level_;
	bool level_eft_;
	Vec2<float> level_ofs_;
	EaseArray<Vec2<float> > ease_type_;
	bool type_eft_;
	Vec2<float> type_ofs_;

	float ease_time_;
	
	bool pause_;
	
public:
	ReviewInfo(GameEnv& env, picojson::object& params) :
		env_(env),
		active_(true),
		params_(params),
		texture_(env.texMng->read(*env.path + "devdata/game.png")),
		konahen_eft_(true),
		level_eft_(true),
		level_ofs_(),
		type_eft_(true),
		type_ofs_(),
		ease_time_(),
		pause_()
	{
		DOUT << "ReviewInfo()" << std::endl;

		const WidgetsMap widgets(params_["widgets"].get<picojson::object>(), env_.texMng, *env_.path, env_.size, env_.y_bottom);
		widget_slash_ = widgets.get("/");
		widget_konahen_ = widgets.get("konahen");
		level_ = widgets.get("level");
		type_ = widgets.get("type");

		{
			picojson::array& type = env_.params->value().get<picojson::object>()["game"].get<picojson::object>()["types"].get<picojson::array>();
			for (picojson::array::iterator it = type.begin(); it != type.end(); ++it)
			{
				std::string& name = it->get<std::string>();
				types_[name] = widgets.get(name);
			}
		}
		// タイプ表示用のWidgetを取り出す

		EasingArayVec3(QUART_OUT, params_["easing_konahen"].get<picojson::array>(), ease_konahen_);
		EasingArayVec2(QUART_OUT, params_["easing_level"].get<picojson::array>(), ease_level_);
		EasingArayVec2(QUART_OUT, params_["easing_type"].get<picojson::array>(), ease_type_);
		// 表示演出読み込み
		
	}
	
	~ReviewInfo()
	{
		DOUT << "~ReviewInfo()" << std::endl;
	}

	bool active() const { return active_; }
	
	void step(const float delta_time)
	{
		if (pause_) return;

		ease_time_ += delta_time;

		if (konahen_eft_)
		{
			konahen_eft_ = ease_konahen_.ease(konahen_ofs_, ease_time_);
		}
		if (level_eft_)
		{
			level_eft_ = ease_level_.ease(level_ofs_, ease_time_);
		}
		if (type_eft_)
		{
			type_eft_ = ease_type_.ease(type_ofs_, ease_time_);
		}
	}

	void draw()
	{
		{
			glPushMatrix();
			glTranslatef(konahen_ofs_.x, 0, 0);

			GrpCol<float> col(1, 1, 1, konahen_ofs_.y);

			widget_konahen_->setCol(col);
			widget_konahen_->draw();
			widget_slash_->setCol(col);
			widget_slash_->draw();

			Vec2<float> wpos = widget_slash_->dispPos();
			Vec2<float> pos(wpos.x - 66, wpos.y - 1);
			DrawNumberSmall(env_.question + 1, 3, pos, col, texture_);

			pos.set(wpos.x + 23, wpos.y - 1);
			DrawNumberSmall(env_.question_total * konahen_ofs_.z + 0.9f, 3, pos, col, texture_);
			// FIXME:値を切り上げるのは実装依存っぽい

			glPopMatrix();
			// こなへん数と、表示番号
		}

		{
			glPushMatrix();
			glTranslatef(level_ofs_.x, 0, 0);

			GrpCol<float> col(1, 1, 1, level_ofs_.y);

			level_->setCol(col);
			level_->draw();
			Vec2<float> wpos = level_->dispPos();
			Vec2<float> pos(wpos.x + 1, wpos.y + 37);
			DrawNumberSmall(env_.level + 1, 2, pos, col, texture_);

			glPopMatrix();
			// レベル
		}
		
		{
			glPushMatrix();
			glTranslatef(type_ofs_.x, 0, 0);

			GrpCol<float> col(1, 1, 1, type_ofs_.y);

			type_->setCol(col);
			type_->draw();

			WidgetsMap::WidgetPtr type = types_[env_.cur_type];
			type->setCol(col);
			type->draw();

			glPopMatrix();
			// タイプ
		}
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

		case MSG_GAME_PAUSE:
			{
				pause_ = !pause_;
			}
			break;
		}
	}
};

}
	
