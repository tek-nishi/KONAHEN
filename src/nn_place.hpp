
#pragma once

//
// 場所を扱う多角形データ
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "picojson.h"
#include "co_vec2.hpp"
#include "co_graph.hpp"
#include "co_json.hpp"
#include "co_polygon.hpp"


namespace ngs {

class Place {
	std::vector<Polygon<float> > area_;
	float radius_;

public:
	explicit Place(const std::string& file) :
		radius_(1)
	{
		Json json = Json(file);
		area_.reserve(256);

#ifdef _DEBUG
		if (!json.value().is<picojson::array>())
		{
			DOUT << "place read error:" << file << std::endl;
			return;
		}
#endif

		picojson::array& array = json.value().get<picojson::array>();
		for(picojson::array::iterator it = array.begin(); it != array.end(); ++it)
		{
			Polygon<float> polygon;
			picojson::array& vtx = (*it).get<picojson::array>();
			for(picojson::array::iterator it = vtx.begin(); it != vtx.end(); ++it)
			{
				picojson::array& v = (*it).get<picojson::array>();
				float x = v[0].get<double>() / 100.0 - 1.0;
				float y = v[1].get<double>() / 50.0 - 1.0;
				// DXFでは 200mm x 100mm でデータを作っているのでその補正
				// ゲーム内では ±1 の座標系に収める
					
				polygon.setVtx(Vec2<float>(x, -y));
			}
			area_.push_back(polygon);
		}
	}

	void radius(const float r) { radius_ = r; }
	float radius() const { return radius_; }

	bool hit(const Vec2<float>& pos)
	{
		bool res = std::find_if(area_.begin(), area_.end(), std::tr1::bind(&Polygon<float>::intension, std::tr1::placeholders::_1, pos)) != area_.end();
		if (!res) res = std::find_if(area_.begin(), area_.end(), std::tr1::bind(&Polygon<float>::touch, std::tr1::placeholders::_1, pos, radius_)) != area_.end();
		return res;
	}

	void center(std::vector<Vec2<float> >& res) const
	{
		res.clear();
		for(std::vector<Polygon<float> >::const_iterator it = area_.begin(); it != area_.end(); ++it)
		{
			res.push_back(it->center());
		}
	}

	void draw()
	{
		std::for_each(area_.begin(), area_.end(), std::mem_fun_ref(&Polygon<float>::draw));
	}
};

}
