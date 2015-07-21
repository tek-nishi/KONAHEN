
#pragma once

//
// Widges を読み込んで管理
//

#include <map>
#include "co_json.hpp"
#include "nn_widget.hpp"
#include "picojson.h"


namespace ngs {

class WidgetsMap
{
public:
  typedef std::tr1::shared_ptr<Widget> WidgetPtr;

private:
  TexMng& texmng_;
  std::map<std::string, WidgetPtr> widgets_;

public:
  WidgetsMap(picojson::object& params, TexMng *const texmng, const std::string& path, const Vec2<int> *const size, const float *y_bottom) :
    texmng_(*texmng)
  {
    DOUT << "WidgetsMap()" << std::endl;
    for(picojson::object::iterator it = params.begin(); it != params.end(); ++it)
    {
      picojson::object& param = it->second.get<picojson::object>();
      const TexMng::tex_ptr texture = texmng_.read(path + param["texture"].get<std::string>());
      WidgetPtr widget(new Widget(param, texture, size, y_bottom));
      widgets_[it->first] = widget;
    }
  }

  ~WidgetsMap()
  {
    DOUT << "~WidgetsMap()" << std::endl;
  }

  WidgetPtr get(const std::string& name) const
  {
    std::map<std::string, WidgetPtr>::const_iterator it = widgets_.find(name);
    if(it == widgets_.end())
    {
      DOUT << "No widget:" << name << std::endl;
      return WidgetPtr();
    }
    else
    {
      return it->second;
    }
  }

  const std::map<std::string, WidgetPtr>& get() const
  {
    return widgets_;
  }

  u_int size() const
  {
    return widgets_.size();
  }
  
};

}

