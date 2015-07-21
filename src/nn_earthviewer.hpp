
#pragma once

//
// 地球プレビュワー
//

#if !(TARGET_OS_IPHONE) && _DEBUG

#include "picojson.h"
#include "co_vec2.hpp"
#include "co_touch.hpp"
#include "co_keyinp.hpp"
#include "co_json.hpp"
#include "co_task.hpp"
#include "nn_localize.hpp"
#include "nn_earth.hpp"
#include "nn_gameproc.hpp"
#include "nn_game2dsetup.hpp"
#include "nn_gameenv.hpp"
#include "nn_control.hpp"
#include "nn_gameworld.hpp"
#include "nn_equator.hpp"


namespace ngs {

class EarthViewer : public ProcBase
{
	Touch& touch_;
	Keyinp& keyinp_;
	Vec2<int> size_;

	Json params_;
	const float scale_;
	float aspect_;
	Camera camera_;
	Camera cockpit_;

	FntMng fonts_;
	TexMng texMng_;
	Localize localize_;

	Earth earth_;
	std::vector<Light> lights_;

	bool rotate_mix_;

	picojson::object places_;
	std::vector<const std::string *> place_names_;
	int place_index_;
	std::tr1::shared_ptr<Place> place_;
	
	GameEnv env_;
	Task task_;

	void ansDisp()
	{
		env_.task->sendMsgAll(MSG_RESETCAMERA_ABORT);

		Task::ProcPtr t = env_.task->add<ResetCamera>(TASK_PRIO_3D, env_);
		ResetCamera *cam = static_cast<ResetCamera *>(t.get());
		cam->type(QUAD_INOUT);
		cam->time(1.0);

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
	}
	
public:
	EarthViewer(const Vec2<int>& size, const float scale, const bool oblong, Touch& touch, Keyinp& keyinp, const std::string& path, const std::string& lang) :
		touch_(touch),
		keyinp_(keyinp),
		size_(size.x / scale, size.y / scale),
		params_(path + "devdata/params.json"),
		scale_(scale),
		aspect_((float)size.x / (float)size.y),
		camera_(Camera::PERSPECTIVE),
		cockpit_(Camera::ORTHOGONAL),
		localize_(lang, path),
		earth_(params_.value().get<picojson::object>()["earth"].get<picojson::object>(), path, camera_),
		rotate_mix_(),
		place_index_()
	{
		DOUT << "EarthViewer()" << std::endl;

		picojson::object& params = params_.value().get<picojson::object>();

		camera_.oblong(oblong);
		SetupCamera(camera_, params["camera"].get<picojson::object>(), aspect_);
		cockpit_.setSize(size.x / scale, size.y / scale);

		{
			picojson::array& array = params["lights"].get<picojson::array>();
			for (picojson::array::iterator it = array.begin(); it != array.end(); ++it)
			{
				Light light;
				SetupLight(light, it->get<picojson::object>());
				lights_.push_back(light);
			}
			earth_.light(lights_);
		}

		env_.touch			 = &touch_;
		env_.keyinp			 = &keyinp_;
		env_.path				 = &path;
		env_.savePath		 = &path;
		env_.size				 = &size_;
		env_.scale			 = scale_;
		env_.params			 = &params_;
		env_.task				 = &task_;
		env_.camera			 = &camera_;
		env_.cockpit		 = &cockpit_;
		env_.earth			 = &earth_;
		env_.earthLight	 = &lights_[0];
		env_.fonts			 = &fonts_;
		env_.texMng			 = &texMng_;
		env_.localize    = &localize_;

		env_.earth_texture = 0;
		earth_.texture(env_.earth_texture);
		earth_.setRotSpeed(0);

		{
			const std::string& file = params["game"].get<picojson::object>()["places"].get<std::string>();
			Json places = Json(path + file);
			places_ = places.value().get<picojson::object>();
			for (picojson::object::iterator it = places_.begin(); it != places_.end(); ++it)
			{
				place_names_.push_back(&it->first);
			}
		}

		task_.add<Control>(TASK_PRIO_SYS, env_);
		task_.add<Game2DSetup>(TASK_PRIO_2D_TOPPRIO, env_);
		task_.add<GameWorld>(TASK_PRIO_3D_TOPPRIO, env_);
		env_.task->add<PlaceDisp>(TASK_PRIO_2D, env_);
		// task_.add<Equator>(TASK_PRIO_3D, env_);
		task_.add<TouchEft>(TASK_PRIO_3D, env_);
	}

	~EarthViewer()
	{
		DOUT << "~EarthViewer()" << std::endl;
	}
	
	void resize(const int w, const int h)
	{
		aspect_ = (float)w / (float)h;
		camera_.setAspect(aspect_);
	}
	void resize(const int w, const int h, const float sx, const float sy)
	{
		camera_.scale(sx, sy);
		cockpit_.setSize(w / scale_, h / scale_);
		size_.set(w / scale_, h / scale_);
	}

	void y_bottom(const float y) {}

	bool step(const float delta_time)
	{
		task_.step(delta_time);
		
		u_char key_inp = env_.keyinp->get();
		if (key_inp != '\0')
		{
			static const u_char tbl[] = {
				'1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c',
				'd', 'e'
			};
			int i;
			for (i = 0; i < elemsof(tbl); ++i)
			{
				if (tbl[i] == key_inp) break;
			}
		
			if (i < elemsof(tbl))
			{
				env_.earth_texture = i;
				earth_.texture(env_.earth_texture);
			}

			if (key_inp == 'r')
			{
				rotate_mix_ = !rotate_mix_;
				env_.task->sendMsgAll(rotate_mix_ ? MSG_CONTROL_MIX : MSG_CONTROL_XY);
			}
		}
		// 地球のテクスチャを変更する

		// 問題と回答を表示するテスト
		if (key_inp == 'q' || key_inp == 'w')
		{
			if (key_inp == 'w')
			{
				--place_index_;
				if (place_index_ < 0) place_index_ = place_names_.size() - 1;
			}

			DOUT << *place_names_[place_index_] << ":" << place_index_ << "(" << place_names_.size() << ")" << std::endl;
			
			picojson::object place = places_[*place_names_[place_index_]].get<picojson::object>();
			env_.cur_place = place["name"].get<std::string>();
			env_.answer = place["answer"].is<std::string>();
			if (env_.answer) env_.ans_place = place["answer"].get<std::string>();
			env_.onetime = place["onetime"].is<bool>() ? place["onetime"].get<bool>() : false;

			if (key_inp == 'q')
			{
				place_index_ = (place_index_ + 1) % place_names_.size();
			}

			{
				std::string& f = place["file"].get<std::string>();
				std::string path = *(env_.path) + "devdata/place/" + f;

				place_ = std::tr1::shared_ptr<Place>(new Place(path));
				
				std::vector<Vec2<float> > center;
				place_->center(center);												// 各範囲の中心を求める
				env_.place_center.clear();
				for(std::vector<Vec2<float> >::iterator it = center.begin(); it != center.end(); ++it)
				{
					Vec3<float> pos = locToPos(*it);
					env_.place_center.push_back(pos);
				}
				this->ansDisp();
			}
			// 問題の正解位置を表示

			env_.task->sendMsgAll(MSG_GAME_PLACEDISP_START);
			if (env_.answer) env_.task->sendMsgAll(MSG_GAME_PLACEDISP_ANS);
			env_.task->sendMsgAll(MSG_GAME_TOUCH_DISP);
		}
		else
		if (key_inp == 'E')
		{
			localize_.reload("en.lang");
			DOUT << "Localize:en.lang" << std::endl;
			// 強制的に英語モード
		}
		else
		if (key_inp == 'J')
		{
			localize_.reload("jp.lang");
			DOUT << "Localize:jp.lang" << std::endl;
			// 強制的に日本語モード
		}
		else
		if (key_inp == 'Z')
		{
			earth_.airDraw();
		}
		else
		if (key_inp == 'X')
		{
			earth_.bodyDraw();
		}
		else
		if (key_inp == 'C')
		{
			earth_.cloudshadowDraw();
		}
		else
		if (key_inp == 'V')
		{
			earth_.cloudbodyDraw();
		}
		else
		if (key_inp == 'B')
		{
			earth_.atmosbodyDraw();
		}
		
		return true;
	}
	
	void draw()
	{
		task_.draw();
	}

#ifdef _DEBUG
	void forceFrame(const bool force) {}
#endif
	
};

}

#endif

