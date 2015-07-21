
#pragma once

//
// ゲーム内での操作
//

#include <iostream>
#include <string>
#include "co_vec3.hpp"
#include "co_quat.hpp"
#include "co_task.hpp"
#include "co_touch.hpp"
#include "nn_gameenv.hpp"


namespace ngs {

class Control : public TaskProc, TouchCallBack
{
	GameEnv& env_;
	bool active_;

	picojson::object& params_;

	Task &task_;

	Camera& camera_;

	const Earth& earth_;
	const float radius_;

	const float near_z_, far_z_;
	const float rot_speed_;
	const float zoom_speed_;
	const float dec_;																	// 減速係数
	const float dec_z_;
	const float bounce_near_;
	const float bounce_far_;

	enum {
		ROTATE_XY,
		ROTATE_MIX
	};
	int mode_;
	
	float len_;

	float arx_, ary_, arz_;
	float vrx_, vry_, vrz_;
	float dist_;
	
	bool	exec_;
	bool	touch_exec_;
	bool	touch_;
	float	touch_delay_;
	float	touch_delay_count_;
	bool	pause_;
	bool	lock_;
	bool  skip_;

	float touch_len_;

	bool hit_;

public:
	explicit Control(GameEnv& env) :
		env_(env),
		active_(true),
		params_(env.params->value().get<picojson::object>()["game"].get<picojson::object>()),
		task_(*env.task),
		camera_(*env.camera),
		earth_(*env.earth),
		radius_(earth_.getRadius()),
		near_z_(params_["near_z"].get<double>()),
		far_z_(params_["far_z"].get<double>()),
		rot_speed_(params_["rot_speed"].get<double>()),
		zoom_speed_(params_["zoom_speed"].get<double>()),
		dec_(params_["decrease"].get<double>()),
		dec_z_(params_["dec_z"].get<double>()),
		bounce_near_(params_["bounce_near"].get<double>()),
		bounce_far_(params_["bounce_far"].get<double>()),
		mode_(ROTATE_XY),
		arx_(),
		ary_(),
		arz_(),
		vrx_(),
		vry_(),
		vrz_(),
		dist_(env.camera->getDist()),
		len_(),
		exec_(true),
		touch_exec_(),
		touch_(),
		touch_delay_(params_["touch"].get<double>()),
		touch_delay_count_(),
		pause_(),
		lock_(),
		skip_(),
		touch_len_(params_["touch_len"].get<double>()),
		hit_()
	{
		DOUT << "Control()" << std::endl;
		env_.touch->resistCallBack(this);
	}
	
	~Control()
	{
		DOUT << "~Control()" << std::endl;
		env_.touch->removeCallBack(this);
	}

	bool active() const { return active_; }

	void step(const float delta_time)
	{
		if (lock_ || !exec_ || pause_) return;
		skip_ = false;

		if (mode_ == ROTATE_XY)
		{
			Vec2<float> rot = QuatToAngle(camera_.getRot());
			ary_ = ary_ / cos(rot.x);											// 極に近い時は加速度を増やす
			float ax = arx_ - vrx_ * dec_;
			float ay = ary_ - vry_ * dec_;
			rot.x += vrx_ * delta_time + ax * delta_time * delta_time * 0.5;
			rot.y += vry_ * delta_time + ay * delta_time * delta_time * 0.5;
			vrx_ += ax * delta_time;
			vry_ += ay * delta_time;
			arx_ = ary_ = 0.0;
			// けっこう真面目に加速度運動を計算
			
			float rx_max = camera_.pitch_max();
			if (rot.x > rx_max)
			{
				rot.x = rx_max;
				vrx_ = 0.0;
			}
			if (rot.x < -rx_max)
			{
				rot.x = -rx_max;
				vrx_ = 0.0;
			}
					
			Quat<float> qrx;
			qrx.rotate(rot.x, Vec3<float>(1, 0, 0));
			Quat<float> qry;
			qry.rotate(rot.y, Vec3<float>(0, 1, 0));
			qrx = qrx * qry;
			qrx.unit();
			camera_.setRot(qrx);
		}
		else
		{
			float ax = arx_ - vrx_ * dec_;
			float ay = ary_ - vry_ * dec_;
			float az = arz_ - vrz_ * dec_;
			float sx = vrx_ * delta_time + ax * delta_time * delta_time * 0.5;
			float sy = vry_ * delta_time + ay * delta_time * delta_time * 0.5;
			float sz = vrz_ * delta_time + az * delta_time * delta_time * 0.5;
			vrx_ += ax * delta_time;
			vry_ += ay * delta_time;
			vrz_ += az * delta_time;
			arx_ = ary_ = arz_ = 0.0;

			Quat<float> qrx;
			qrx.rotate(sx, Vec3<float>(1, 0, 0));
			Quat<float> qry;
			qry.rotate(sy, Vec3<float>(0, 1, 0));
			Quat<float> qrz;
			qrz.rotate(sz, Vec3<float>(0, 0, 1));
			qrx = qrx * qry * qrz;
			qrx.unit();

			Quat<float> rot = camera_.getRot();
			rot = qrx * rot;
			rot.unit();
			camera_.setRot(rot);
		}

		{
			float dist = camera_.getDist();
			float a = (dist_ - dist) * dec_z_;
			dist += a * 0.5 * delta_time * delta_time;
			camera_.setDist(dist);

			// 追従するZを計算
			if (dist_ < near_z_)
			{
				dist_ += (near_z_ - dist_) * bounce_near_ * delta_time;
			}
			else
			if (dist_ > far_z_)
			{
				dist_ += (far_z_ - dist_) * bounce_far_ * delta_time;
			}

			// 範囲制限
			if (dist_ < (camera_.getNearZ() + radius_))
			{
				dist_ = camera_.getNearZ() + radius_;
			}
			else
			if (dist_ > (camera_.getFarZ() - radius_))
			{
				dist_ = camera_.getFarZ() - radius_;
			}
		}

		if (touch_delay_count_ > 0.0f) touch_delay_count_ -= delta_time;
	}

	void draw()	{}

	void msg(const int msg)
	{
		switch (msg)
		{
		case MSG_DEMOPLAY_END:
		case MSG_RESETCAMERA_STOP:
			{
				dist_ = env_.camera->getDist();
			}
			break;
			
		case MSG_GAME_PAUSE:
			{
				pause_ = !pause_;
				skip_ = true;																// 1フレームだけ入力をスキップ
			}
			break;
			
		case MSG_GAME_CONTROL_START:
			{
				exec_ = true;
				pause_ = false;
				touch_ = false;
			}
			break;

		case MSG_GAME_CONTROL_STOP:
			{
				exec_ = false;
				vrx_ = arx_ = 0.0;
				vry_ = ary_ = 0.0;
				vrz_ = arz_ = 0.0;
			}
			break;
				
		case MSG_GAME_TOUCH_START:
			{
				touch_exec_ = true;
				touch_delay_count_ = 0.0f;
				touch_ = false;
			}
			break;

		case MSG_GAME_TOUCH_STOP:
			{
				touch_exec_ = false;
			}
			break;

		case MSG_GAME_TOUCH_OUT:
			{
				touch_delay_count_ = touch_delay_;
			}
			break;

		case MSG_CONTROL_LOCK:
			{
				lock_ = true;
				touch_delay_count_ = 0;
				vrx_ = arx_ = 0.0;
				vry_ = ary_ = 0.0;
				vrz_ = arz_ = 0.0;
			}
			break;

		case MSG_CONTROL_UNLOCK:
			{
				lock_ = false;
			}
			break;

		case MSG_CONTROL_XY:
			{
				mode_ = ROTATE_XY;
				vrx_ = arx_ = 0.0;
				vry_ = ary_ = 0.0;
				vrz_ = arz_ = 0.0;
			}
			break;

		case MSG_CONTROL_MIX:
			{
				mode_ = ROTATE_MIX;
				vrx_ = arx_ = 0.0;
				vry_ = ary_ = 0.0;
				vrz_ = arz_ = 0.0;
			}
			break;

		case MSG_REVIEW_CONTROL_STOP:
			{
				exec_ = false;
			}
			break;

		case MSG_REVIEW_CONTROL_START:
			{
				exec_ = true;
				touch_ = false;
			}
			break;
		}
	}

	
	void touchStart(const Touch& touch, const std::vector<TouchInfo>& info)
	{
		if (lock_ || !exec_ || pause_) return;

		touch_ = true;
		len_ = 0.0f;
	}
	
	void touchMove(const Touch& touch, const std::vector<TouchInfo>& info)
	{
		if (lock_ || !exec_ || pause_) return;

		switch (info.size())
		{
		case 1:
			{
				Vec2<float> pos = info[0].pos - info[0].l_pos;
				float len = pos.length() / env_.scale;			// 3Gを考慮
				if (len > 0.0f)
				{
					len_ += len;
					touch_ = touch_ && (len_ < touch_len_);

					Matrix mat;
					mat.translate(0, 0, -camera_.getDist());
					
					Vec3<float> p(0, 0, radius_);
					p = camera_.posToScreen(p, mat.value());
					// カメラから見た地球の表面のz位置を求める

					Vec3<float> p1 = camera_.posToWorld(info[0].pos, p.z, mat.value());
					Vec3<float> p2 = camera_.posToWorld(info[0].l_pos, p.z, mat.value());
					// タッチ座標を地球の表面上での座標に変換

					Vec3<float> wpos = p1 - p2;
					float l = std::sqrt(wpos.x * wpos.x);
					float n = l / radius_;
					float ry = std::asin(minmax(n, -1.0f, 1.0f)) * rot_speed_;
					if (wpos.x < 0) ry = -ry;
					ary_ = ry;

					l = std::sqrt(wpos.y * wpos.y);
					n = l / radius_;
					float rx = std::asin(minmax(n, -1.0f, 1.0f)) * rot_speed_;
					if (wpos.y < 0) rx = -rx;
					arx_ = rx;
				}
			}
			break;

		default:
			{
				touch_ = false;

				Matrix mat;
				mat.translate(0, 0, -camera_.getDist());
				
				Vec3<float> p(0, 0, radius_);
				p = camera_.posToScreen(p, mat.value());
				// カメラから見た地球の表面のz位置を求める

				Vec3<float> p1 = camera_.posToWorld(info[0].pos, p.z, mat.value());
				Vec3<float> p2 = camera_.posToWorld(info[1].pos, p.z, mat.value());
				Vec3<float> p3 = camera_.posToWorld(info[0].l_pos, p.z, mat.value());
				Vec3<float> p4 = camera_.posToWorld(info[1].l_pos, p.z, mat.value());
				// タッチ座標を地球の表面上での座標に変換
				
				p1 = p1 - p2;
				p3 = p3 - p4;
				float dz =  sqrt(p3.x * p3.x + p3.y * p3.y) - sqrt(p1.x * p1.x + p1.y * p1.y);
				dist_ += dz * zoom_speed_;

				{
					Vec2<float> v1 = info[0].pos - info[1].pos;
					Vec2<float> v2 = info[0].l_pos - info[1].l_pos;
					arz_ = v1.angle(v2) * rot_speed_ * 0.6f;
					if (v2.cross(v1) < 0.0f) arz_ = -arz_;
					// Z軸回転
				}
			}
			break;
		}
	}
	
	void touchEnd(const Touch& touch, const std::vector<TouchInfo>& info)
	{
		if (lock_ || !exec_ || pause_ || skip_) return;

		if (touch_exec_ && touch_ && (touch_delay_count_ <= 0.0f) && (info.size() == 1))
		{
			hit_ = crossSpherePos(env_.hit_pos, touch.pos2window(info[0].pos), radius_, Vec3<float>(), camera_, earth_.mtx());
			env_.hit = hit_;
			if (hit_) task_.sendMsgAll(MSG_GAME_TOUCH);
		}
	}
	
};

}
