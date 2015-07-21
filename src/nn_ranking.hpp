
#pragma once

//
// ランキング
//

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <algorithm>
#include "co_graph.hpp"
#include "co_fntmng.hpp"
#include "co_texmng.hpp"
#include "co_easing.hpp"
#include "nn_gameenv.hpp"
#include "nn_fadelight.hpp"
#include "nn_achievement.hpp"
#include "nn_agree.hpp"
#include "GameCenter.h"
#include "Twitter.h"


namespace ngs {

struct GameResult {
	int level;
	int konahen;
	int score;
	int rank;
	bool played;																			// 今回プレイした結果

	bool operator <(const GameResult& r) const {
		return (score == r.score) ? ((konahen == r.konahen) ? ((level == r.level) ? rank < r.rank : level < r.level) : konahen < r.konahen) : score < r.score;
		// スコア、こなへん数、レベル、ランクでソート
	}
};


class Ranking : public TaskProc, TouchCallBack {
	enum {
		RESULT_NUM = 10,
		RANKIN_INDEX = 10,															// 隠し要素を出す為のランクイン位置
	};

	GameEnv& env_;
	bool active_;
	const int mode_;
	const bool record_;

	picojson::object& params_;
	const picojson::array& rank_tbl_;

	Json json_;
	FntMng::FontPtr font_;

	const TexMng::tex_ptr texture_;

	std::vector<GameResult> results_;
	const std::string& title_;

	float skip_delay_;
	bool do_skip_;

	bool wipe_;
	float time_cur_;
	float time_ed_;
	Vec3<float> ofs_st_, ofs_ed_, ofs_;
	float scale_, scale_st_, scale_ed_;
	float scale_time_ed_;
	float disp_time_;

	GrpCol<float> col_;																// プレイ結果を明滅させる用
	int col_index_;
	int col_timer_;
	float col_demo_time_;
	float col_demo_time_ed_;
	int col_demo_index_;

	bool agree_;
	bool touched_;
	bool finish_;
	
public:
	Ranking(GameEnv& env, const int mode, const bool record) :
		env_(env),
		active_(true),
		mode_(mode),
		record_(record),
		params_(env.params->value().get<picojson::object>()["ranking"].get<picojson::object>()),
		rank_tbl_(env.params->value().get<picojson::object>()["rank_tbl"].get<picojson::array>()),
		json_(isSavedParamExists(*env.savePath + "results.json") ? *env.savePath + "results.json" : *env.path + "devdata/results.json"),
		font_(ReadFont("ranking", *env.fonts, *env.path, env.params->value().get<picojson::object>())),
		texture_(env.texMng->read(*env.path + "devdata/round.png")),
		title_(params_["title"].get<picojson::array>()[mode_].get<std::string>()),
		skip_delay_(params_["skip_delay"].get<double>()),
		do_skip_(),
		wipe_(true),
		time_cur_(),
		time_ed_(params_["time_ed"].get<double>()),
		ofs_st_(1, 0, 1),
		ofs_ed_(0, 1, 0),
		scale_(),
		scale_st_(0),
		scale_ed_(1),
		scale_time_ed_(time_ed_ * 0.4),
		disp_time_(record ? 0 : params_["disp_time"].get<double>()),
		col_(1, 1, 1, 1),
		col_index_(),
		col_timer_(),
		col_demo_time_(10000),													// 最初は動かさない
		col_demo_time_ed_(params_["col_demo_time"].get<double>()),
		col_demo_index_(RESULT_NUM),
		agree_(),
		touched_(),
		finish_()
	{
		DOUT << "Ranking()" << std::endl;

		static const char *record_tbl[] = {
			"normal",
			"advanced",
			"survival"
		};

		picojson::array& array = json_.value().get<picojson::object>()[record_tbl[mode_]].get<picojson::array>();
		// FIXME:コードの下の方でも array を使ってます
		for (picojson::array::iterator it = array.begin(); it != array.end(); ++it)
		{
			int level = it->get<picojson::object>()["level"].get<double>();
			int konahen = it->get<picojson::object>()["konahen"].get<double>();
			int score = it->get<picojson::object>()["score"].get<double>();
			int rank = it->get<picojson::object>()["rank"].get<double>();
			
			GameResult result = {
				level,
				konahen,
				score,
				rank,
				false
			};
			results_.push_back(result);
		}

		if (record_)
		{
			GameResult result = {
				env_.level + 1,
				env_.konahen,
				env_.score,
				env_.rank,
				true
			};
			results_.push_back(result);

			{
				picojson::object obj;
				obj["level"] = picojson::value((float)result.level);
				obj["konahen"] = picojson::value((float)result.konahen);
				obj["score"] = picojson::value((float)result.score);
				obj["rank"] = picojson::value((float)result.rank);

				array.push_back(picojson::value(obj));

				std::string fpath = *env_.savePath + "results.json";
				json_.write(fpath);
			}
		}
		std::sort(results_.begin(), results_.end());
		std::reverse(results_.begin(), results_.end());
		results_.resize(RESULT_NUM);

		bool played = false;
		int index = 0;
		for (std::vector<GameResult>::iterator it = results_.begin(); it != results_.end(); ++it, ++index)
		{
			if (it->played)
			{
				played = it->played;
				break;
			}
		}
		// FIXME:ここはアルゴリズムを使うべきか…

		if (played && (index < RANKIN_INDEX))
		{
			switch (env_.game_mode)
			{
			case GAME_MODE_NORMAL:
				{
					AchievementUnlock("advanced", env_);
				}
				break;

			case GAME_MODE_ADVANCED:
				{
					AchievementUnlock("survival", env_);
				}
				break;

			case GAME_MODE_SURVIVAL:
				{
					AchievementUnlock("earth_texture", env_);
				}
				break;
			}
			// ランクインしたら実績解除
		}

		env_.sound->play("ranking", 1, 0.6);
		env_.touch->resistCallBack(this);
	}
	
	~Ranking()
	{
		DOUT << "~Ranking()" << std::endl;
		env_.touch->removeCallBack(this);
	}

	bool active() const { return active_; }

	void step(const float delta_time)
	{
		if (wipe_)
		{
			time_cur_ += delta_time;

			Easing easing;
			easing.ease(ofs_, time_cur_, ofs_st_, ofs_ed_, time_ed_, QUART_OUT);
			easing.ease(scale_, time_cur_, scale_st_, scale_ed_, scale_time_ed_, QUART_OUT);

			if (time_cur_ >= time_ed_)
			{
				wipe_ = false;
				if (finish_)
				{
					active_ = false;
					return;
					// 演出が終わったらタスク終了
				}
				else
				{
					col_demo_time_ = 0.0;
					agree_ = true;
					if (record_)
					{
						env_.task->add<Agree>(TASK_PRIO_2D, env_);
						// 確認ボタン表示
						DispGameCenerBtn();
						DispTweetBtn();
					}
				}
			}
		}

		if (skip_delay_ > 0.0f)
		{
			do_skip_ = (skip_delay_ -= delta_time) <= 0.0f;
			// スキップ許可
		}

		if (col_demo_time_ < col_demo_time_ed_)
		{
			col_demo_time_ += delta_time;
			if (col_demo_time_ >= col_demo_time_ed_)
			{
				col_demo_time_ = 0.0;
				col_demo_index_ -= 1;
				if (col_demo_index_ < 0) col_demo_index_ += RESULT_NUM;
			}
		}
		
		if (--col_timer_ < 0)
		{
			static const GrpCol<float> colTbl[] = {
				GrpCol<float>(1, 0, 0, 1),
				GrpCol<float>(0, 1, 0, 1),
				GrpCol<float>(1, 1, 0, 1),
				GrpCol<float>(0, 0, 1, 1),
				GrpCol<float>(1, 0, 1, 1),
				GrpCol<float>(0, 1, 1, 1)
			};
			col_ = colTbl[col_index_];
			col_index_ = (col_index_ + 1) % (sizeof(colTbl) / sizeof(colTbl[0]));
			col_timer_ = 4;
		}

		if (disp_time_ > 0.0)
		{
			if ((disp_time_ -= delta_time) <= 0.0)
			{
				touched_ = true;
				// デモの時は勝手に抜ける
			}
		}

		if (touched_)
		{
			touched_ = false;
			finish_ = true;

			wipe_ = true;
			time_cur_ = 0.0;
			ofs_st_ = ofs_;
			ofs_ed_.set(-1.0, 0.0, 1.0);
			scale_st_ = scale_;
			scale_ed_ = 0;
			time_ed_ *= 0.8f;
			col_demo_time_ = 10000;
			col_demo_index_ = -1;
			disp_time_ = 0.0;
			env_.task->sendMsgAll(MSG_RANKING_END);

			if (record_)
			{
				HideGameCenerBtn();
				HideTweetBtn();
			}
				
			{
				env_.task->sendMsgAll(MSG_FADELIGHT_STOP);
				Task::ProcPtr t = env_.task->add<FadeLight>(TASK_PRIO_3D, env_);
				FadeLight *fl = static_cast<FadeLight *>(t.get());
				fl->type(QUAD_OUT);
				fl->scale(1.0);
				fl->time(1.0);
				// 地球を明るくする
			}
		}
	}

	void draw()
	{
#if (TARGET_OS_IPHONE)
		glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
#endif
		float y_ofs = record_ ? -230 : -200;
		
#ifdef VER_LITE
		y_ofs += params_["lite_ofs"].get<double>();
#endif

		GrpRoundBox obj;
		obj.pos(0, 198 + y_ofs);
		obj.size(530, 475 * scale_);
		obj.center();
		obj.texture(texture_);
		obj.col(0, 0, 0, 0.6 * scale_);
		obj.draw();

		const Vec2<float>& size = font_->size(title_);
		font_->pos(0, y_ofs - 80 * ofs_.z);
		font_->center(size.x / 2, 0);
		font_->col(1, 1, 1, ofs_.y);
		font_->scale(0.9, 1);
		font_->draw(title_);
		
		float x = -250.0;
		float y = 60 + y_ofs;
		float ofs = ofs_.x * env_.size->x;
		int index = 0;
		for (std::vector<GameResult>::iterator it = results_.begin(); it != results_.end(); ++it, ++index)
		{
			std::stringstream sstr;
			sstr <<
				"Lv:" << std::setw(2) << it->level <<
				" Konahen:" << std::setw(2) << it->konahen <<
				" Score:" << std::setw(4) << it->score <<
				" " << rank_tbl_[it->rank].get<std::string>();
			font_->pos(x + ofs, y);

			static const GrpCol<float>tbl[] = {
				GrpCol<float>(1, 0, 0, 1),
				GrpCol<float>(1, 0, 0, 1),
				GrpCol<float>(1, 0, 0, 1),
				GrpCol<float>(1, 1, 1, 1),
				GrpCol<float>(1, 1, 1, 1),
				
				GrpCol<float>(0.9, 0.9, 0.9, 1),
				GrpCol<float>(0.8, 0.8, 0.8, 1),
				GrpCol<float>(0.7, 0.7, 0.7, 1),
				GrpCol<float>(0.6, 0.6, 0.6, 1),
				GrpCol<float>(0.5, 0.5, 0.5, 1),
			};
			
			GrpCol<float> col = tbl[index];
			if ((record_ && it->played) || (!record_ && (index == col_demo_index_)))
			{
				col = col_;
			}
			
			col.a = ofs_.y;
			font_->col(col);

			font_->center(0, 0);
			font_->draw(sstr.str());
			
			ofs = -ofs;
			y += 40.0f;
		}
#if (TARGET_OS_IPHONE)
		glPopClientAttrib();
#endif
	}

	void msg(const int msg)
	{
		switch (msg)
		{
		case MSG_GAME_AGREE:
			if (agree_)
			{
				agree_ = false;
				touched_ = true;
				// ランキング画面終了
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
			time_cur_ = 100.0f;
			// 表示演出をスキップ
		}
		else
		if (agree_ && !record_)
		{
			agree_ = false;
			touched_ = true;
			// ランキング画面終了
		}
	}
};
	
}
