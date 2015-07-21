
#pragma once

//
// ソフトのセッティングを書き出したり保存したり
//

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "co_json.hpp"
#include "co_misc.hpp"
#include "nn_misc.hpp"


namespace ngs {


// セーブデータを1.0→1.1へ
void Upgrade_10to11(Json& json, const std::string& path, const std::string& savePath)
{
  DOUT << "Upgrade_10to11()" << std::endl;
  
  Json new_data(path + "devdata/settings.json");

  picojson::object& src = new_data.value().get<picojson::object>();
  picojson::object& dst = json.value().get<picojson::object>();

  dst["version"] = src["version"];
  // バージョン更新

  dst["konahen"] = src["konahen"];
  // 各モードのこなへん数

  dst["reviewexec"] = src["reviewexec"];
  // 復習モード実行フラグ

  dst["volume"] = src["volume"];
  // 音量は初期化
  dst["bgm_mute"] = src["bgm_mute"];
  dst["se_mute"] = src["se_mute"];
  // ミュート設定

  {
    picojson::object& src_places = dst["places"].get<picojson::object>();
    picojson::object dst_places;
    for (picojson::object::iterator it = src_places.begin(); it != src_places.end(); ++it)
    {
      double num = (it->second).get<bool>() ? 1.0 : 0.0;
      dst_places[it->first] = picojson::value(num);
    }
    dst["places"] = picojson::value(dst_places);
    // 問題に答えた数を記録
  }
  
  if (isSavedParamExists(savePath + "results.json"))
  {
    Json src_res(path + "devdata/results.json");
    Json dst_res(savePath + "results.json");

    picojson::object& src = src_res.value().get<picojson::object>();
    picojson::object& dst = dst_res.value().get<picojson::object>();

    for (picojson::object::iterator it = dst.begin(); it != dst.end(); ++it)
    {
      picojson::array& dst_array = (it->second).get<picojson::array>();
      picojson::array& src_array = src[it->first].get<picojson::array>();

      for (u_int i = 0; i < dst_array.size(); ++i)
      {
        int num = (i < src_array.size()) ? i : src_array.size() - 1;
        // プログラムの設計上、ランキングには11データ入っている
        dst_array[i].get<picojson::object>()["rank"] = src_array[num].get<picojson::object>()["rank"];
        // 評価を追加
      }
    }

    std::string fpath = savePath + "results.json";
    dst_res.write(fpath);
    // ランキングのデータも更新
  }
}


class Settings
{
  const std::string& path_;
  const std::string& savePath_;

  Json json_;
  
public:
  Settings(const std::string& path, const std::string& savePath) :
    path_(path),
    savePath_(savePath),
    json_(isSavedParamExists(savePath_ + "settings.json") ? savePath_ + "settings.json" : path_ + "devdata/settings.json")
  {
    DOUT << "Settings()" << std::endl;

    // バージョンチェック
    double version = this->get<double>("version");
    if (version < 1.1)
    {
      Upgrade_10to11(json_, path_, savePath_);
      write();
      // 一旦書き出す
    }
  }
  
  ~Settings()
  {
    DOUT << "~Settings()" << std::endl;
    write();
  }
  
  void reload()
  {
    json_.reload();
  }

  void write()
  {
    const std::string path = savePath_ + "settings.json";
    json_.write(path);
  }

  void reset()
  {
    const std::string path = path_ + "devdata/settings.json";
    json_.reload(path);
  }

  Json& json() { return json_; }

  template <typename T>
  T& get(const std::string& name)
  {
    return json_.value().get<picojson::object>()[name].get<T>();
  }

  template <typename T>
  void set(const std::string name, const T& value)
  {
    json_.value().get<picojson::object>()[name] = picojson::value(value);
  }
  
};

}
