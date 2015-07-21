
#pragma once

//
// DXFパース
// dxfを読み込んでjsonにする
// Illustrator10 での単位はミリ。200mm x 100mm で作成する
//

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <map>
#include "picojson.h"


namespace ngs {

class Dxf {
	picojson::value json_;

public:
	explicit Dxf(const std::string& file)
	{
		std::ifstream fstr(file.c_str());
		if(fstr.is_open())
		{
			picojson::object object;
			bool poly_section = false;
			bool poly_name = false;
			picojson::array vtx_array;
			std::string name;
			vtx_array.reserve(256);

			while(!fstr.eof())
			{
				std::string str;
				getline(fstr, str);
				if(str == "SECTION")
				{
				}
				else
				if(str == "POLYLINE")
				{
					poly_section = true;
					poly_name = true;
					vtx_array.clear();
					// ポリゴンデータ開始
				}
				else
				if(str == "AcDbEntity")
				{
					if(poly_name)
					{
						int dummy;
						fstr >> dummy >> name;
						std::transform(name.begin(), name.end(), name.begin(), static_cast<int (*)(int)>(std::tolower));

						poly_name = false;
					}
				}
				else
				if(str == "VERTEX")
				{
					// 頂点データ開始
				}
				else
				if(str == "AcDb2dVertex")
				{
					int x_d, y_d;
					float x, y;
					fstr >> x_d >> x >> y_d >> y;
					picojson::array array;
					array.push_back(picojson::value(x));
					array.push_back(picojson::value(y));
					vtx_array.push_back(picojson::value(array));
				}
				else
				if(str == "SEQEND")
				{
					if(poly_section)
					{
						// DOUT << name << std::endl;
						std::map<std::string, picojson::value>::iterator it = object.find(name);
						if(it == object.end())
						{
							object[name] = picojson::value(picojson::array());
						}
						picojson::array& a = object[name].get<picojson::array>();
						a.push_back(picojson::value(vtx_array));
						
						poly_section = false;
					}
					// ポリゴンデータ終了
				}
				else
				if(str == "ENDSEC")
				{
				}
			}
			json_ = picojson::value(object);
		}
	}

	picojson::value& json() { return json_; }
};

}
