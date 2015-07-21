
#pragma once

//
// 復習モード
//

#include "nn_review_intro.hpp"
#include "nn_review_bar.hpp"
#include "nn_review_info.hpp"
#include "nn_correct_rate.hpp"


namespace ngs {

class Review : public TaskProc
{
	GameEnv& env_;
	bool active_;

	picojson::object& params_;
	picojson::object& places_;

	std::vector<const std::string *> place_names_;
	std::vector<int> place_levels_;
	Place *place_;

	const float camera_time_;

	int update_num_;
	bool pause_;

	void readPlace(const int index)
	{
		const std::string *name = place_names_[index];
		picojson::object& place = places_[*name].get<picojson::object>();
		std::map<std::string, Place>::iterator it = env_.place_data->find(*name);
#ifdef NO_READ_PLACES
		if (it == env_.place_data->end())
		{
			std::string file = *env_.path + "devdata/place/" + place["file"].get<std::string>();
			Place place(file);
			env_.place_data->insert(std::map<std::string, Place>::value_type(*name, place));
			it = env_.place_data->find(*name);
			DOUT << "Read:" << *name << std::endl;
		}
#endif
		place_ = &it->second;

		env_.cur_place = place["name"].get<std::string>();
		env_.level = place["level"].get<double>();
		env_.cur_type = place["type"].get<std::string>();
		env_.answer = place["answer"].is<std::string>();
		if (env_.answer) env_.ans_place = place["answer"].get<std::string>();
		env_.onetime = place["onetime"].is<bool>() ? place["onetime"].get<bool>() : false;
		// 問題と答えを設定

		std::vector<Vec2<float> > center;
		place_->center(center);
		env_.place_center.clear();
		for(std::vector<Vec2<float> >::iterator it = center.begin(); it != center.end(); ++it)
		{
			Vec3<float> pos = locToPos(*it);
			env_.place_center.push_back(pos);
		}
		// 各範囲の中心を求める
	}
	
	void ansDisp()
	{
		env_.task->sendMsgAll(MSG_RESETCAMERA_ABORT);
		
		Task::ProcPtr t = env_.task->add<ResetCamera>(TASK_PRIO_3D, env_);
		ResetCamera *cam = static_cast<ResetCamera *>(t.get());
		cam->type(QUAD_INOUT);
		cam->time(camera_time_);

		const std::vector<Vec3<float> >& center = env_.place_center;
		Vec3<float> pos = center[0];

		env_.hit_pos = pos;

		const float ry = env_.earth->rotate();
		// オイラー角っぽく正解位置を設定
		Vec2<float> rot = posToLoc(pos);
		std::swap(rot.x, rot.y);
		float rx_max = env_.camera->pitch_max();
		if (rot.x > rx_max) rot.x = rx_max;
		if (rot.x < -rx_max) rot.x = -rx_max;
		rot.y = PI / 2.0 - rot.y - Ang2Rad(ry);
		cam->rotate(rot);

		float dist = env_.camera->getDist();
		if (dist < 1950.0) dist = 1950.0;
		cam->distance(dist);

		env_.task->sendMsgAll(MSG_GAME_PLACEDISP_START);
		if (env_.answer) env_.task->sendMsgAll(MSG_GAME_PLACEDISP_ANS);
		env_.task->sendMsgAll(MSG_GAME_TOUCH_DISP);
		// 各タスクにメッセージを送信
	}

	// 問題をソートする関数オブジェクト
	struct PlaceSort {
		picojson::object& places_;
		PlaceSort(picojson::object& places) :
			places_(places)
		{}
		
		bool operator()(const std::string *left, const std::string *right) const
		{
			picojson::object& l_place = places_[*left].get<picojson::object>();
			const int l_level = l_place["level"].get<double>();
			const std::string& l_name = l_place["name"].get<std::string>();
			const std::string& l_type = l_place["type"].get<std::string>();

			picojson::object& r_place = places_[*right].get<picojson::object>();
			const int r_level = r_place["level"].get<double>();
			const std::string& r_name = r_place["name"].get<std::string>();
			const std::string& r_type = r_place["type"].get<std::string>();

			return (l_level == r_level) ? ((l_type == r_type) ? l_name < r_name : l_type < r_type) : l_level < r_level;
			// レベルとタイプと名前でソート
		}
	};

	void taskSetup()
	{
		env_.task->add<GameMenu>(TASK_PRIO_2D, env_);
		env_.task->add<ReviewBar>(TASK_PRIO_2D, env_, params_, place_levels_);
		env_.task->add<ReviewInfo>(TASK_PRIO_2D, env_, params_);
		env_.task->add<PlaceDisp>(TASK_PRIO_2D, env_);
		bool review = true;
		env_.task->add<CorrectRate>(TASK_PRIO_2D, env_, review);
		
		env_.earth->setRotSpeed(0.0, 2.0, CIRC_IN);
		// 地球の自転を停止
	}
	
public:
	Review(GameEnv& env) :
		env_(env),
		active_(true),
		params_(env.params->value().get<picojson::object>()["review"].get<picojson::object>()),
		places_(env.places->value().get<picojson::object>()),
		place_(),
		camera_time_(params_["camera_time"].get<double>()),
		update_num_(2),
		pause_()
	{
		DOUT << "Review()" << std::endl;

		env_.question = -1;
		// 表示問題はquestionを兼用
		// 最初の表示用にわざと0以外の値を代入

		picojson::object& place = env.settings->get<picojson::object>("places");
		for (picojson::object::iterator it = place.begin(); it != place.end(); ++it)
		{
			if (it->second.get<double>() > 0.0f)
			{
				place_names_.push_back(&it->first);
			}
		}
		env_.question_total = place_names_.size();

		std::sort(place_names_.begin(), place_names_.end(), PlaceSort(places_));
		for (std::vector<const std::string *>::const_iterator it = place_names_.begin(); it != place_names_.end(); ++it)
		{
			picojson::object& place = places_[*(*it)].get<picojson::object>();
			const int level = place["level"].get<double>();
			place_levels_.push_back(level);
			// ソート済みの問題からレベルの情報を生成
			// FIXME:専用の構造体を作って処理させたい
		}

		env_.task->add<Equator>(TASK_PRIO_3D, env_);
		env_.task->add<NsDisp>(TASK_PRIO_2D, env_);
		if (!env_.settings->get<bool>("reviewexec"))
		{
			env_.task->add<ReviewIntro>(TASK_PRIO_2D, env_);
			// 初回のみの開始演出タスク
		}
		else
		{
			taskSetup();
		}

		env_.task->sendMsgAll(MSG_CONTROL_XY);
		// 回転モードを設定
		env_.task->sendMsgAll(MSG_GAME_TOUCH_STOP);
		// ゲーム入力停止
		env_.task->sendMsgAll(MSG_GAME_TOUCH_BLINK_ON);
		// 場所点滅表示する
	}
	
	~Review()
	{
		DOUT << "~Review()" << std::endl;
	}

	bool active() const { return active_; }
	
	void step(const float delta_time)
	{
		if (pause_) return;
	}

	void draw() {}

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

		case MSG_REVIEW_START:
			{
				taskSetup();
			}
			break;

		case MSG_REVIEW_UPDATE:
			{
				if ((update_num_ > 0) && (--update_num_ == 0))
				{
					bool value = true;
					env_.settings->set<bool>("reviewexec", value);
					// 操作されたらモードを実行したフラグを立てる
				}
				
				readPlace(env_.question);
				ansDisp();
				env_.sound->play("question", 1.0, 0.05f);				
			}
			break;
		}
	}

};

}
	
