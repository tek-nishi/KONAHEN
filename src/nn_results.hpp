
#pragma once

//
// プレイ結果表示
//

#include <iostream>
#include <string>
#include "co_texmng.hpp"
#include "co_graph.hpp"
#include "co_task.hpp"
#include "co_easearray.hpp"
#include "nn_gameenv.hpp"
#include "nn_sound.hpp"
#include "nn_resetcamera.hpp"
#include "nn_fadelight.hpp"
#include "nn_agree.hpp"
#include "nn_widgets_map.hpp"
#include "nn_achievement.hpp"
#include "nn_score.hpp"
#include "nn_rankeft.hpp"
#include "GameCenter.h"
#include "Twitter.h"


namespace ngs {

const int play_max		= 9999;
const int konahen_max = 99999;
// 最大値の定義

class Results : public TaskProc, TouchCallBack
{
	GameEnv& env_;
	bool active_;

	picojson::object& params_;
	const picojson::array& rank_tbl_;

	float ease_time_;

	float input_delay_;
	bool do_skip_;
	bool eft_fin_;

	WidgetsMap::WidgetPtr title_;

	const TexMng::tex_ptr tex_num_;
	const TexMng::tex_ptr texture_;

	const float ofs_x_;
	const float ofs_y_;
	const float next_y_;
	Vec2<float> num_pos_;
	float text_ofs_y_;
	Vec2<float> rank_pos_;
	
	EaseArray< Vec2<float> > wipe_in_;
	EaseArray< Vec2<float> > wipe_out_;
	EaseArray< Vec2<float> > *wipe_ease_;
	float wipe_time_;
	Vec2<float> wipe_res_;
	bool wipe_;

	struct ScoreInfo {
		float base;
		float disp;
		int figures;
		bool avg;
		WidgetsMap::WidgetPtr widget;
		EaseArray<float> easing;
	};
	std::vector<ScoreInfo> scores_;
	WidgetsMap::WidgetPtr rank_;
	WidgetsMap::WidgetPtr game_rank_;
	EaseArray<Vec2<float> > rank_easing_;
	Vec2<float> rank_vec_;
	bool rank_eft_;

	u_int col_timer_;
	GrpCol<float> col_;

#ifdef _DEBUG
	const WidgetsMap& widgets_;
	u_int rank_debug_;
#endif

	bool agree_;
	bool finish_;

	
	void scoreSetup(const float value, const int figures, const bool avg, const std::string& widget, const std::string& easing, const WidgetsMap& widgets)
	{
		ScoreInfo info = { value, 0.0f, figures, avg };
		
		info.widget = widgets.get(widget);
		picojson::array& a = params_[easing].get<picojson::array>();
		EasingArayFloat(QUART_OUT, a, info.easing);
		scores_.push_back(info);
	}
	
public:
	Results(GameEnv& env, const WidgetsMap& widgets) :
		env_(env),
		active_(true),
		params_(env.params->value().get<picojson::object>()["results"].get<picojson::object>()),
		rank_tbl_(env.params->value().get<picojson::object>()["rank_tbl"].get<picojson::array>()),
		ease_time_(),
		input_delay_(params_["input_delay"].get<double>()),
		do_skip_(),
		eft_fin_(),
		title_(widgets.get("result_title")),
		tex_num_(env.texMng->read(*env.path + "devdata/game.png")),
		texture_(env.texMng->read(*env.path + "devdata/round.png")),
		ofs_x_(params_["ofs_x"].get<double>()),
		ofs_y_(params_["ofs_y"].get<double>()),
		next_y_(params_["next_y"].get<double>()),
		text_ofs_y_(),
		wipe_ease_(&wipe_in_),
		wipe_time_(),
		wipe_(true),
		rank_eft_(),
		col_timer_(),
#ifdef _DEBUG
		widgets_(widgets),
		rank_debug_(),
#endif
		agree_(),
		finish_()
	{
		DOUT << "Results()" << std::endl;

#ifdef VER_LITE
		EasingArayVec2(QUART_OUT, params_["easing_in_lite"].get<picojson::array>(), wipe_in_);
		EasingArayVec2(QUART_OUT, params_["easing_out_lite"].get<picojson::array>(), wipe_out_);
#else
		EasingArayVec2(QUART_OUT, params_["easing_in"].get<picojson::array>(), wipe_in_);
		EasingArayVec2(QUART_OUT, params_["easing_out"].get<picojson::array>(), wipe_out_);
#endif
		// 表示演出

		scoreSetup(env_.konahen, 2, false, "result_konahen", "ease_konahen", widgets);
		scoreSetup(env_.level + 1, 2, false, "result_level", "ease_level", widgets);
		scoreSetup(env_.just_total, 2, false, "result_just", "ease_just", widgets);
		scoreSetup(env_.just_max, 2, false, "result_just_max", "ease_just_max", widgets);

		float tap_avg = (env_.konahen > 0) ? (float)env_.tap_num / (float)env_.konahen : -1.0f;
		scoreSetup(tap_avg, 2, true, "result_tap_avg", "ease_tap_avg", widgets);

		float avg_answer = (env_.konahen > 0) ? (float)env_.total_time / (float)env_.konahen : -1.0f;
		scoreSetup(avg_answer, 2, true, "result_avg_answer", "ease_avg_answer", widgets);

		if (env_.game_mode != GAME_MODE_SURVIVAL)
		{
			scoreSetup(env_.miss_num, 2, false, "result_miss", "ease_miss", widgets);
		}
		else
		{
			text_ofs_y_ += 15.0f;
		}
		scoreSetup(env_.score, 4, false, "result_score", "ease_score", widgets);
		// 表示するスコア

		rank_ = widgets.get("result_rank");
		assert((u_int)env_.rank < rank_tbl_.size());
		game_rank_ = widgets.get(rank_tbl_[env_.rank].get<std::string>());
		EasingArayVec2(CUBIC_IN, params_["ease_rank"].get<picojson::array>(), rank_easing_);
		// ランク

		float ofs_y = 0.0f;
		u_int i;
		for (i = 0; i < (scores_.size() - 1); ++i)
		{
			WidgetsMap::WidgetPtr widget = scores_[i].widget;
			Vec2<float> pos = widget->posOrig();
			pos.y += ofs_y;
			widget->setPos(pos);
			ofs_y += next_y_;
		}
		{
			ofs_y += 5.0f;

			WidgetsMap::WidgetPtr widget = scores_[i].widget;
			Vec2<float> pos = widget->posOrig();
			pos.y += ofs_y;
			widget->setPos(pos);
			ofs_y += next_y_;

			ofs_y += 5.0f;
			pos = rank_->posOrig();
			pos.y += ofs_y;
			rank_->setPos(pos);
		}
		// スコアと検定の表示位置を調整

		env_.sound->play("result");
		env_.touch->resistCallBack(this);

		env_.task->sendMsgAll(MSG_GAME_CONTROL_STOP);

		Task::ProcPtr t = env_.task->add<ResetCamera>(TASK_PRIO_3D, env_);
		ResetCamera *cam = static_cast<ResetCamera *>(t.get());
		cam->type(QUAD_INOUT);
		cam->time(3.0);
		cam->randomDist();

		env_.earth->setRotSpeed(env_.earth->getRotSpeedOrig(), 1.0);
		env_.earth->setLightScale(1.0);
		t = env_.task->add<FadeLight>(TASK_PRIO_3D, env_);
		FadeLight *fl = static_cast<FadeLight *>(t.get());
		fl->type(QUAD_OUT);
		fl->scale(0.5);
		fl->time(3.0);

		bool value = true;
		env_.settings->set<bool>("played", value);

		{
			picojson::array& array = env_.settings->json().value().get<picojson::object>()["playnum"].get<picojson::array>();
			int num = array[env_.game_mode].get<double>();
			if ((num += 1) > play_max) num = play_max;
			array[env_.game_mode] = picojson::value(static_cast<double>(num));
			// モード別プレイ回数の記録
		}

		{
			picojson::array& array = env_.settings->json().value().get<picojson::object>()["konahen"].get<picojson::array>();
			int num = array[env_.game_mode].get<double>();
			if ((num += env_.konahen) > konahen_max) num = konahen_max;
			array[env_.game_mode] = picojson::value(static_cast<double>(num));
			// モード別こなへん数の記録

			num = 0;
			for (picojson::array::const_iterator it = array.begin(); it != array.end(); ++it)
			{
				num += it->get<double>();
			}
			DOUT << "Total Konahen:" << num << std::endl;
			if ((num >= 1000) && isAchievementUnlock("survival", env))
			{
				AchievementUnlock("nightmode", env);	
			}
			// 1000こなへん＆全モード開示で夜景モード開示
		}

		{
			picojson::array& array = params_["num_pos"].get<picojson::array>();
			num_pos_.x = array[0].get<double>();
			num_pos_.y = array[1].get<double>();
			// 数字のテクスチャ座標
		}
		
		{
			picojson::array& array = params_["rank_pos"].get<picojson::array>();
			rank_pos_.x = array[0].get<double>();
			rank_pos_.y = array[1].get<double>();
			// ランクの表示オフセット
		}
		
		if (env_.level > 0)
		{
			AchievementUnlock("review", env_, true);
			// レベル２以上で復習モード開示
		}

#ifdef VER_LITE
		{
			int num = 0;
			const picojson::array& array = env_.settings->json().value().get<picojson::object>()["playnum"].get<picojson::array>();
			for (picojson::array::const_iterator it = array.begin(); it != array.end(); ++it)
			{
				num += it->get<double>();
			}
			if ((num % 10) == 0)
			{
				AchievementLock("getfull", env_);
				// 10回に一回の頻度で表示
			}
			AchievementUnlock("getfull", env_, true);
			// １回以上プレイで GET FULLボタン 表示
		}
#endif
		
		SendScoreToGameCenter(env_.game_mode, env_.score, env_.konahen);
		// GameCenterにスコアを送信
		
		{
			const std::string modeTbl[] = {
				std::string("NORMAL"),
				std::string("ADVANCED"),
				std::string("SURVIVAL")
			};

			std::string text = env_.localize->get("tweet_text");
			// ローカライズ文字列から生成

			std::string::size_type pos = text.find("%1", 0);
			if (pos != std::string::npos)
			{
				text.replace(pos, 2, modeTbl[env_.game_mode]);
				// ゲームモードを置換
			}
			
			std::stringstream sstr;
			sstr << env_.konahen;
			pos = text.find("%2", 0);
			if (pos != std::string::npos)
			{
				text.replace(pos, 2, sstr.str());
				// こなへん数を置換
			}

			sstr.str("");
			sstr.clear(std::stringstream::goodbit);
      // TIPS:stringstreamのフラッシュ
			sstr << env_.score;
			pos = text.find("%3", 0);
			if (pos != std::string::npos)
			{
				text.replace(pos, 2, sstr.str());
				// スコアを置換
			}

			pos = text.find("%4", 0);
			if (pos != std::string::npos)
			{
				text.replace(pos, 2, rank_tbl_[env_.rank].get<std::string>());
				// ランクを置換
			}

			setTweetText(text);
			// ツイートを生成して保存
		}
	}
	
	~Results()
	{
		DOUT << "~Results()" << std::endl;
		env_.touch->removeCallBack(this);
	}

	bool active() const { return active_; }

	void step(const float delta_time)
	{
		++col_timer_;
		static const GrpCol<float> colTbl[] = {
			GrpCol<float>(1, 0, 0, 1),
			GrpCol<float>(0, 1, 0, 1),
			GrpCol<float>(1, 1, 0, 1),
			GrpCol<float>(0, 0, 1, 1),
			GrpCol<float>(1, 0, 1, 1),
			GrpCol<float>(0, 1, 1, 1),
		};
		col_ = colTbl[(col_timer_ / 6) % elemsof(colTbl)];

		if (wipe_)
		{
			wipe_time_ += delta_time;
			wipe_ = wipe_ease_->ease(wipe_res_, wipe_time_);
			if (!wipe_ && finish_)
			{
				active_ = false;
				return;
			}
		}

		if (input_delay_ > 0.0)
		{
			do_skip_ = (input_delay_ -= delta_time) <= 0.0f;
			// スキップ許可
		}
		
		ease_time_ += delta_time;
		bool finish = true;
		for (std::vector<ScoreInfo>::iterator it = scores_.begin(); it != scores_.end(); ++it)
		{
			float t;
			bool res = it->easing.ease(t, ease_time_);
			it->disp = it->base * t;
			if (res) finish = false;
			// スコアのカウントアップ
		}
		
		{
			bool res = rank_easing_.ease(rank_vec_, ease_time_);
			if (res) finish = false;
			if (!rank_eft_ && (ease_time_ >= 2.7f))
			{
				rank_eft_ = true;
				Vec2<float> center = game_rank_->pos();
				const Vec2<float>& size = game_rank_->size();
				center.x += ofs_x_ + size.x / 2.0f;
				center.y += ofs_y_ + size.y / 2.0f + text_ofs_y_;
#ifdef VER_LITE
				center.y += params_["lite_ofs"].get<double>();
#endif
				env_.task->add<RankEft>(TASK_PRIO_2D, env_, center);
				env_.sound->play("rank");
				// ランク決定演出
			}
			// 評価の表示演出
		}

		if (finish && !eft_fin_)
		{
			eft_fin_ = true;
			agree_ = true;
			env_.task->add<Agree>(TASK_PRIO_2D, env_);
			// 表示演出が終わったら「了解」ボタンを表示

			setTweetImage();
			DispGameCenerBtn();
			DispTweetBtn();
		}

#ifdef _DEBUG
		if (env_.keyinp->get() == 'N')
		{
			game_rank_ = widgets_.get(rank_tbl_[rank_debug_].get<std::string>());
			rank_debug_ = (rank_debug_ + 1) % rank_tbl_.size();
		}
#endif
	}
	
	void draw()
	{
		glPushMatrix();
#ifdef VER_LITE
		glTranslatef(0, params_["lite_ofs"].get<double>(), 0);
#endif
		glScalef(wipe_res_.x, wipe_res_.x, 1.0);

		GrpRoundBox obj;
		obj.pos(0, -50);
		obj.size(450, 485);
		obj.center();
		obj.texture(texture_);
		obj.col(0, 0, 0, 0.4 * wipe_res_.y);
		obj.draw();

		title_->setCol(1, 1, 1, wipe_res_.y);
		title_->draw();

		glPushMatrix();
		glTranslatef(ofs_x_, ofs_y_ + text_ofs_y_, 0);

		GrpCol<float> col(1, 1, 1, wipe_res_.y);
		for (std::vector<ScoreInfo>::iterator it = scores_.begin(); it != scores_.end(); ++it)
		{
			WidgetsMap::WidgetPtr widget = it->widget;
			widget->setCol(col);
			widget->draw();

			Vec2<float> pos = widget->pos();
			pos += num_pos_;
			if (it->avg)
			{
				if (it->base >= 0.0f)
				{
					DrawNumberSmall(it->disp, it->figures, pos, col, tex_num_);
					pos.x += 22 * it->figures - 5;
					DrawNumberSmall(10, 1, pos, col, tex_num_);
					pos.x += 22 - 5;
					DrawNumberSmall(it->disp * 100.0f, 2, pos, col, tex_num_);
					// 小数表示
				}
				else
				{
					DrawNumberSmall(11, 1, pos, col, tex_num_);
					pos.x += 22;
					DrawNumberSmall(11, 1, pos, col, tex_num_);
					pos.x += 22 - 5;
					DrawNumberSmall(10, 1, pos, col, tex_num_);
					pos.x += 22 - 5;
					DrawNumberSmall(11, 1, pos, col, tex_num_);
					pos.x += 22;
					DrawNumberSmall(11, 1, pos, col, tex_num_);
					// 記録無し表示
				}
			}
			else
			{
				DrawNumberSmall(it->disp + 0.9f, it->figures, pos, col, tex_num_);
				// 通常のスコア表示
			}
		}

		{
			rank_->setCol(col);
			rank_->draw();
			Vec2<float> pos = rank_->pos();
			pos += rank_pos_;

			game_rank_->setPos(pos);
			if (rank_vec_.x > 0.0f)
			{
				game_rank_->setCol(col_.r, col_.g, col_.b, col_.a * wipe_res_.y * rank_vec_.x);
				game_rank_->setScale(rank_vec_.y, rank_vec_.y);
				game_rank_->draw();
			}
			// ランク表示
		}

		glPopMatrix();

		glPopMatrix();
	}

	void msg(const int msg)
	{
		switch (msg)
		{
		case MSG_GAME_AGREE:
			if (agree_)
			{
				agree_ = false;

				finish_ = true;
				wipe_ = true;
				wipe_ease_ = &wipe_out_;
				wipe_time_ = 0.0f;
				HideGameCenerBtn();
				HideTweetBtn();
				env_.task->sendMsgAll(MSG_GAME_RESULT_END);
				// 結果画面終了
			}
			break;
		}
	}

	void touchStart(const Touch& touch, const std::vector<TouchInfo>& info) {}
	void touchMove(const Touch& touch, const std::vector<TouchInfo>& info) {}
	void touchEnd(const Touch& touch, const std::vector<TouchInfo>& info)
	{
		if (do_skip_)
		{
			do_skip_ = false;
			ease_time_ = 100.0f;
			// 表示演出を強制的に終える
		}
	}
};

}
