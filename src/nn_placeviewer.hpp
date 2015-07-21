
#pragma once

//
// place Viewer
//

#if !(TARGET_OS_IPHONE) && _DEBUG

#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include "co_touch.hpp"
#include "co_keyinp.hpp"
#include "co_vec2.hpp"
#include "co_texmng.hpp"
#include "co_dxf.hpp"
#include "co_font.hpp"
#include "co_json.hpp"
#include "nn_proc_base.hpp"
#include "nn_camera.hpp"
#include "nn_place.hpp"
#include "nn_sound.hpp"
#include "nn_misc.hpp"


namespace ngs {

class PlaceViewer : public ProcBase
{
  Keyinp& keyinp_;
  Vec2<int> size_;
  std::string path_;
  Camera cockpit_;
  Localize localize_;

  TexMng texmng_;

  picojson::object params_;
  Font font_;

  Json json_;
  
  std::size_t place_mode_;
  int level_;
  int level_max_;
  std::size_t start_;
  std::size_t disp_num_;
  bool refresh_;
  bool mode_chg_;
  std::vector<std::string> place_type_;
  std::vector<std::string> place_names_;
  std::vector<std::string> place_wnames_;
  std::string tex_name_l_;
  std::string tex_name_r_;

  bool disp_place_;
  std::tr1::shared_ptr<Place> place_;
  
  void refreshMenu()
  {
    place_wnames_.clear();
    picojson::object& obj = json_.value().get<picojson::object>();
    std::size_t num = (start_ + disp_num_) < place_names_.size() ? disp_num_ : place_names_.size() - start_;
    for (std::size_t i = 0; i < num; ++i)
    {
      std::string name = place_names_[start_ + i];

      picojson::object& params = obj[name].get<picojson::object>();
      char a = 'a' + i;
      std::stringstream sstr;
      sstr << a << ". " << localize_.get(params["name"].get<std::string>());
      place_wnames_.push_back(sstr.str());
    }
  }

  void readPlace(std::string name)
  {
    picojson::object& obj = json_.value().get<picojson::object>();
    picojson::object& params = obj[name].get<picojson::object>();
    std::string& s = params["file"].get<std::string>();
    std::string path = path_ + "devdata/place/" + s;
    place_.reset();
    place_ = std::tr1::shared_ptr<Place>(new Place(path));
  }
  
  void previewPlace(Place& place)
  {
    {
      const TexMng::tex_ptr texture = texmng_.get(tex_name_l_);
      const Vec2<int>& ts = texture->size(); 

      GrpSprite obj;
      obj.size(size_.x / 2.0f, size_.y);
      obj.center(size_.x / 2.0f, size_.y / 2.0);
      obj.texture(texture);
    
      obj.uv(0, 0, ts.x, ts.y);
      obj.col(1.0, 1.0, 1.0);
      obj.draw();
      // 左側
    }
    {
      const TexMng::tex_ptr texture = texmng_.get(tex_name_r_);
      const Vec2<int>& ts = texture->size(); 

      GrpSprite obj;
      obj.size(size_.x / 2.0f, size_.y);
      obj.center(0, size_.y / 2.0);
      obj.texture(texture);
    
      obj.uv(0, 0, ts.x, ts.y);
      obj.col(1.0, 1.0, 1.0);
      obj.draw();
      // 右側
    }

    glScalef(size_.x / 2.0, size_.y / 2.0, 1.0);
    place.draw();
  }

  void convertPlaces(const std::string& name)
  {
    Dxf dxf(path_ + "devdata/" + name);
    picojson::object& object = dxf.json().get<picojson::object>();
    for (picojson::object::iterator it = object.begin(); it != object.end(); ++it)
    {
      std::string fpath = path_ + "devdata/place/" + it->first + ".json";
      picojson::value json = picojson::value(it->second);
      std::ofstream fstr(fpath.c_str());
      if (fstr.is_open()) { fstr << json; };
    }
  }

  void listup(const int mode, const int level)
  {
    place_names_.clear();
    picojson::object& obj = json_.value().get<picojson::object>();
    std::string& type = place_type_[mode];
    for (picojson::object::iterator it = obj.begin(); it != obj.end(); ++it)
    {
      picojson::object& data = (it->second).get<picojson::object>();
      std::string& t = data["type"].get<std::string>();
      int l = data["level"].get<double>();
      bool same_type = (mode == 0) || (t == type);
      bool same_level = (level == -1) || (l == level);
      if (same_type && same_level) place_names_.push_back(it->first);
    }
    // TIPS: mapはsort済み
  }
  
public:
  PlaceViewer(const Vec2<int>& size, const float scale, Touch& touch, Keyinp& keyinp, const std::string& path, const std::string& lang) :
    keyinp_(keyinp),
    size_(size),
    path_(path),
    cockpit_(Camera::ORTHOGONAL),
    localize_(lang, path),
    params_(Json(path + "devdata/params.json").value().get<picojson::object>()),
    font_(FONT_TEXTURE, path + params_["placeview"].get<picojson::object>()["font"].get<std::string>(), params_["placeview"].get<picojson::object>()["height"].get<double>()),
    json_(path + params_["game"].get<picojson::object>()["places"].get<std::string>()),
    place_mode_(),
    level_(-1),
    start_(),
    disp_num_(20),
    refresh_(true),
    mode_chg_(),
    disp_place_(),
    place_()
  {
    DOUT << "PlaceViewer()" << std::endl;
    cockpit_.setSize(size.x, size.y);

    {
      place_type_.push_back(std::string(""));
      picojson::array& type = params_["game"].get<picojson::object>()["types"].get<picojson::array>();
      for (picojson::array::iterator it = type.begin(); it != type.end(); ++it)
      {
        std::string& name = it->get<std::string>();
        place_type_.push_back(std::string(name));
      }
    }
    // タイプをリストアップ

    const picojson::array& array = getEarthTextures(params_["earth"].get<picojson::object>(), 3);
    const TexMng::tex_ptr tex_l = texmng_.read(path + "devdata/" + array[0].get<std::string>());
    tex_name_l_ = tex_l->name();
    const TexMng::tex_ptr tex_r = texmng_.read(path + "devdata/" + array[1].get<std::string>());
    tex_name_r_ = tex_r->name();

    level_max_ = params_["game"].get<picojson::object>()["question"].get<picojson::array>().size() - 1;

    this->listup(place_mode_, level_);
  }
  ~PlaceViewer() {
    DOUT << "~PlaceViewer()" << std::endl;
  }

  void resize(const int w, const int h) {}
  void resize(const int w, const int h, const float sx, const float sy) {
    cockpit_.setSize(w, h);
  }
  void y_bottom(const float y) {}
  
  bool step(const float delta_time)
  {
    if (refresh_)
    {
      if (mode_chg_)
      {
        this->listup(place_mode_, level_);
        start_ = 0;
      }
      this->refreshMenu();
      mode_chg_ = false;
      refresh_ = false;
    }

    std::size_t key_inp = keyinp_.get() - 'a';
    if (key_inp < place_wnames_.size())
    {
      std::string name = place_names_[start_ + key_inp];
      this->readPlace(name);
      disp_place_ = true;
    }
    if (keyinp_.get() == '-')
    {
      disp_place_ = false;
    }
    
    if (keyinp_.get() == INP_KEY_LEFT)
    {
      if (start_ > 0)
      {
        start_ -= disp_num_;
        refresh_ = true;
      }
    }
    else
    if (keyinp_.get() == INP_KEY_RIGHT)
    {
      if ((start_ + disp_num_) < place_names_.size())
      {
        start_ += disp_num_;
        refresh_ = true;
      }
    }

    if (keyinp_.get() == INP_KEY_DOWN)
    {
      if (level_ >= 0)
      {
        --level_;
        mode_chg_ = true;
        refresh_ = true;
      }
    }
    else
    if (keyinp_.get() == INP_KEY_UP)
    {
      if (level_ < level_max_)
      {
        ++level_;
        mode_chg_ = true;
        refresh_ = true;
      }
    }

    key_inp = keyinp_.get() - '0';
    if (key_inp < place_type_.size())
    {
      if (place_mode_ != key_inp)
      {
        place_mode_ = key_inp;
        mode_chg_ = true;
        refresh_ = true;
      }
      
    }

    key_inp = keyinp_.get();
    if (key_inp == CTRL_E)
    {
      DOUT << "Convert" << std::endl;
      this->convertPlaces(std::string("world.dxf"));
    }
    else
    if (key_inp == 'E')
    {
      localize_.reload("en.lang");
      refresh_ = true;
      DOUT << "Localize:en.lang" << std::endl;
      // 強制的に英語モード
    }
    else
    if (key_inp == 'J')
    {
      localize_.reload("jp.lang");
      refresh_ = true;
      DOUT << "Localize:jp.lang" << std::endl;
      // 強制的に日本語モード
    }
    
    return true;
  }

  void draw() {
    cockpit_.setup();
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    if (disp_place_)
    {
      this->previewPlace(*place_);
    }
    else
    {
      float x = size_.x / -2.0 + 40;
      float y = size_.y / -2.0 + 40;
      for (std::vector<std::string>::iterator it = place_wnames_.begin(); it != place_wnames_.end(); ++it)
      {
        font_.pos(x, y);
        font_.center(0, 0);
        font_.draw(*it);
        y += 20.0;
#if 0
        if (y > size_.y)
        {
          y = size_.y / -2.0;
          x += 128.0;
        }
#endif
      }
      y += 20.0;
      
      {
        std::ostringstream sstr;
        sstr << "Total:" << place_names_.size() << " Level:" << level_;
        font_.pos(size_.x / -2.0 + 20, y);
        font_.center(0, 0);
        font_.draw(sstr.str());
      }
    }

    {
      std::ostringstream sstr;
      
      picojson::array& array = params_["game"].get<picojson::object>()["types"].get<picojson::array>();
      picojson::object& text = params_["placeview"].get<picojson::object>()["types"].get<picojson::object>();
      int idx = 0;
      sstr << idx << ":全て ";
      ++idx;
      for (picojson::array::iterator it = array.begin(); it != array.end(); ++it)
      {
        sstr << idx << ":" << text[it->get<std::string>()].get<std::string>() << " ";
        ++idx;
      }
      const Vec2<float> size = font_.size(sstr.str());
      font_.pos(0, size_.y / 2.0 - size.y - 25.0);
      font_.center(size.x / 2.0, size.y);
      font_.draw(sstr.str());
      // ジャンル表示
    }
    {
      std::wstring help(L"←・→ ページ移動 ↓・↑表示レベル変更 C-eで変換");
      const Vec2<float> size = font_.size(help);
      font_.pos(0, size_.y / 2.0 - size.y - 5.0);
      font_.center(size.x / 2.0, size.y);
      font_.draw(help);
      // その他操作説明
    }
  }

#ifdef _DEBUG
  void forceFrame(const bool force) {}
#endif
};
    
}

#endif
