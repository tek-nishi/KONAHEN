
#pragma once

//
// 地球
// FIXME:コンストラクタでテクスチャを読まないのが屈辱的
//

#include <ctime>
#include "co_vec3.hpp"
#include "co_matrix.hpp"
#include "co_easing.hpp"
#include "nn_sphere.hpp"
#include "nn_hemisphere.hpp"
#include "nn_air.hpp"
#include "nn_light.hpp"
#include "nn_camera.hpp"


namespace ngs {

enum {
  EARTH_APP_TEXTURE,
  EARTH_APP_CLOUD,
  EARTH_APP_ATMOS,
  EARTH_APP_ATMOS_TEXTURE,
  EARTH_APP_AIR_TEXTURE,
};

const picojson::array& getEarthTextures(picojson::object& params, const int index)
{
  picojson::array& array = params["earth_appearance"].get<picojson::array>();
  return array[index].get<picojson::array>()[EARTH_APP_TEXTURE].get<picojson::array>();
}

bool getEarthCloudDraw(picojson::object& params, const int index)
{
  picojson::array& array = params["earth_appearance"].get<picojson::array>();
  return array[index].get<picojson::array>()[EARTH_APP_CLOUD].get<bool>();
}

bool getEarthAtmosDraw(picojson::object& params, const int index)
{
  picojson::array& array = params["earth_appearance"].get<picojson::array>();
  return array[index].get<picojson::array>()[EARTH_APP_ATMOS].get<bool>();
}

const std::string& getAtmosTexture(picojson::object& params, const int index)
{
  picojson::array& array = params["earth_appearance"].get<picojson::array>();
  return array[index].get<picojson::array>()[EARTH_APP_ATMOS_TEXTURE].get<std::string>();
}

const std::string& getAirTexture(picojson::object& params, const int index)
{
  picojson::array& array = params["earth_appearance"].get<picojson::array>();
  return array[index].get<picojson::array>()[EARTH_APP_AIR_TEXTURE].get<std::string>();
}


class Earth {
  const std::string& path_;
  picojson::object& params_;
  const Camera& camera_;

  int h_div_;
  int v_div_;
  
  float radius_;
  float rotY_;
  float rotSpeed_;

  float lightScale_;
  float cloud_scale_;
  float cloud_shadow_;
  float air_scale_;
  float atmos_scale_;

  // Sphere body_;
  Hemisphere body_;

  Hemisphere atmos_;
  Air air_;

  typedef std::tr1::shared_ptr<Texture> tex_ptr;

  tex_ptr body_t_l_;
  tex_ptr body_t_r_;

  Texture cloud_t_l_;
  Texture cloud_t_r_;

  tex_ptr atmos_t_;
  tex_ptr air_t_;

  Light *body_l_;
  // FIXME: ポインタは微妙

  float cloud_alpha_;

  bool atmos_draw_;
  bool cloud_draw_;

  bool air_draw_;
  bool body_draw_;
  bool cloud_shadow_draw_;
  bool cloud_body_draw_;
  bool atmos_body_draw_;

  GLdouble mtx_[16];

  float time_cur_;
  float rot_st_;
  float rot_ed_;
  float time_ed_;
  int ease_type_;
  bool ease_;

public:
  Earth(picojson::object& params, const std::string& path, const Camera& camera) :
    path_(path),
    params_(params),
    camera_(camera),
    h_div_(params["h_div"].get<double>()),
    v_div_(params["v_div"].get<double>()),
    radius_(params["radius"].get<double>()),
    rotY_(),
    rotSpeed_(params["rotate"].get<double>()),
    lightScale_(1.0),
    cloud_scale_(params["cloud_scale"].get<double>()),
    cloud_shadow_(params["cloud_shadow"].get<double>()),
    air_scale_(params["air_scale"].get<double>()),
    atmos_scale_(params["atmos_scale"].get<double>()),
    // body_(h_div_, v_div_, radius_),
    body_(h_div_ / 2, v_div_, radius_),
    atmos_(h_div_ / 2, v_div_, radius_, true),
    air_(h_div_, radius_ * air_scale_, radius_ * 0.995),
    cloud_t_l_(path + "devdata/" + params["cloud_texture_l"].get<std::string>(), true),
    cloud_t_r_(path + "devdata/" + params["cloud_texture_r"].get<std::string>(), true),
    body_l_(),
    cloud_alpha_(1.0),
    atmos_draw_(true),
    cloud_draw_(true),
    air_draw_(true),
    body_draw_(true),
    cloud_shadow_draw_(true),
    cloud_body_draw_(true),
    atmos_body_draw_(true),
    time_cur_(),
    rot_st_(),
    rot_ed_(),
    time_ed_(),
    ease_type_(),
    ease_()
  {
    DOUT << "Earth()" << std::endl;

    const GLfloat color[] = { 0.0, 0.0, 0.0, 1.0 };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, color);
    // 光が当たらない面に対する色の混合具合
  }

  ~Earth()
  {
    DOUT << "~Earth()" << std::endl;
  }

  void light(std::vector<Light>& lights)
  {
    body_l_ = &lights[0];
  }

  const GLdouble *mtx() const { return mtx_; }
  float lightScale() { return lightScale_; }
  void setLightScale(const float scale) { lightScale_  = scale; }
  float getRadius() const { return radius_; }

  float rotate() const { return rotY_; }
  void rotate(const float rotate) { rotY_ = rotate; }
  
  float getRotSpeedOrig() const { return params_["rotate"].get<double>(); }
  void setRotSpeed(const float speed) { rotSpeed_ = speed; }
  void setRotSpeed(const float rot, const float time, const int type = CIRC_IN)
  {
    ease_ = true;
    time_cur_ = 0.0f;
    rot_st_ = rotSpeed_;
    rot_ed_ = rot;
    time_ed_ = time;
    ease_type_ = type;
  }
  void cloud_alpha(const float alpha) { cloud_alpha_ = alpha; }
  float cloud_alpha() const { return cloud_alpha_; }

  void texture(const int index)
  {
    int idx = index;
    if (idx < 0)
    {
#ifdef VER_LITE
      idx = 3;                                      // LITE版は４月固定
#else
      time_t now;
      time(&now);
      tm *date = localtime(&now);
      idx = date->tm_mon;
      assert((idx >=0) && (idx < 12));
      DOUT << "Earth texture:" << idx << std::endl;
      // 現在の月のテクスチャを使う
#endif
    }

    body_t_l_.reset();
    body_t_r_.reset();
    // メモリの二重化を防ぐ

    const picojson::array& array = getEarthTextures(params_, idx);
    body_t_l_ = tex_ptr(new Texture(path_ + "devdata/" + array[0].get<std::string>(), true));
    body_t_r_ = tex_ptr(new Texture(path_ + "devdata/" + array[1].get<std::string>(), true));
    cloud_draw_ = getEarthCloudDraw(params_, idx);
    atmos_draw_ = getEarthAtmosDraw(params_, idx);
    if (atmos_draw_)
    {
      DOUT << "atmos texture" << std::endl;
      atmos_t_.reset();
      air_t_.reset();
      atmos_t_ = tex_ptr(new Texture(path_ + "devdata/" + getAtmosTexture(params_, idx)));
      air_t_ = tex_ptr(new Texture(path_ + "devdata/" + getAirTexture(params_, idx)));
    }
  }

  
  void step(const float delta_time)
  {
    if (ease_)
    {
      time_cur_ += delta_time;
      ease_ = (time_cur_ < time_ed_);
      Easing easeing;
      easeing.ease(rotSpeed_, time_cur_, rot_st_, rot_ed_, time_ed_, ease_type_);
    }
    
    rotY_ += rotSpeed_ * delta_time;
  }
  
  void draw()
  {
    {
      const GLfloat dcolor[] = {
        1.0f * lightScale_,
        1.0f * lightScale_,
        1.0f * lightScale_,
        1.0f
      };
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, dcolor);
    }

    glCullFace(GL_BACK);
    glEnable(GL_TEXTURE_2D);

    if (atmos_draw_)
    {
      glPushMatrix();
    
      const Matrix& mtx = camera_.getMatrix();
      glMultMatrixd(mtx.value());
      // 常にカメラの方を向く

      float r = std::asin(radius_ / camera_.getDist());
      float d = std::sin(PI / 2.0f - r);
      glScalef(1 / d, 1 / d, 1);
      // 球体が表示される大きさを求める

      glDisable(GL_LIGHTING);
      glEnable(GL_BLEND);
      glBlendFunc(GL_ONE, GL_ONE);
      glColor4f(lightScale_, lightScale_, lightScale_, 1);
      if (air_draw_)
      {
        air_t_->bind();
        air_.draw();
        // 大気のぼんやりした部分を描画
      }
      glPopMatrix();
    }
    
    glPushMatrix();
    body_l_->setup();
    // 光源設定

    glDisable(GL_BLEND);
    glRotatef(rotY_, 0.0, 1.0, 0.0);
    if (body_draw_)
    {
      body_t_l_->bind();
      body_.draw();
      // 本体描画
    }
    glGetDoublev(GL_MODELVIEW_MATRIX, mtx_);
    
    glRotatef(180, 0.0, 1.0, 0.0);
    if (body_draw_)
    {
      body_t_r_->bind();
      body_.draw();
      // 本体描画
    }
    glDisable(GL_LIGHTING);
    glPopMatrix();

    if (cloud_draw_)
    {
      // 雲は減算モードで影を描画、続いて加算モードで本体を描画
      glPushMatrix();
      glEnable(GL_BLEND);
      float scale = cloud_alpha_ * lightScale_;
      if (use_glex_blend)
      {
        glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
        glBlendFunc(GL_ONE, GL_ONE);
        scale *= cloud_shadow_;
      }
      else
      {
        glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
      }
      glColor4f(scale, scale, scale, 1);

      glRotatef(rotY_, 0.0, 1.0, 0.0);

      if (cloud_shadow_draw_)
      {
        cloud_t_l_.bind();
        body_.draw();
        // 雲の影を描画
      }
      glRotatef(180, 0.0, 1.0, 0.0);
      if (cloud_shadow_draw_)
      {
        cloud_t_r_.bind();
        body_.draw();
        // 雲の影を描画
      }

      if (use_glex_blend)
      {
        glBlendEquation(GL_FUNC_ADD);
      }
      glPopMatrix();

      glPushMatrix();
      scale = cloud_alpha_ * lightScale_;
      glColor4f(scale, scale, scale, 1);

      glRotatef(rotY_, 0.0, 1.0, 0.0);
      glScalef(cloud_scale_, cloud_scale_, cloud_scale_);
      glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
      if (cloud_body_draw_)
      {
        cloud_t_l_.bind();
        body_.draw();
        // 雲本体を描画
      }
      glRotatef(180, 0.0, 1.0, 0.0);
      if (cloud_body_draw_)
      {
        cloud_t_r_.bind();
        body_.draw();
        // 雲本体を描画
      }
    
      glPopMatrix();
    }

    if (atmos_draw_)
    {
      glPushMatrix();

      const Matrix& mtx = camera_.getMatrix();
      glMultMatrixd(mtx.value());
      glScalef(atmos_scale_, atmos_scale_, atmos_scale_);
      // 常にカメラの方を向く

      glMatrixMode(GL_TEXTURE);
      glPushMatrix();
      float r = std::asin(radius_ / camera_.getDist());
      float sx = sin((PI / 2.0) - r * 1.4);
      glTranslatef((sx - 1.0) / 2.0, (sx - 1.0) / 2.0, 1.0);
      glScalef(1.0 - (sx - 1.0), 1.0 - (sx - 1.0), 1.0);
      // カメラと地球の距離からテクスチャのスケーリングを求める

      glColor4f(lightScale_, lightScale_, lightScale_, 1);
      glEnable(GL_BLEND);
      glBlendFunc(GL_ONE, GL_ONE);
      if (atmos_body_draw_)
      {
        atmos_t_->bind();
        atmos_.draw();
        // 大気を氷描画
      }
      glPopMatrix();

      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
    }
  }

#ifdef _DEBUG

  void airDraw()
  {
    air_draw_ = !air_draw_;
  }

  void bodyDraw()
  {
    body_draw_ = !body_draw_;
  }

  void cloudshadowDraw()
  {
    cloud_shadow_draw_ = !cloud_shadow_draw_;
  }

  void cloudbodyDraw()
  {
    cloud_body_draw_ = !cloud_body_draw_;
  }

  void atmosbodyDraw()
  {
    atmos_body_draw_ = !atmos_body_draw_;
  }
  
#endif
  
};

}
