
#pragma once

//
// グラフィックの小さな部品
// TODO: イージングによる移動
//

#include "co_vec2.hpp"
#include "co_graph.hpp"
#include "co_texmng.hpp"
#include "picojson.h"

namespace ngs {

class Widget {
	Vec2<float> pos_;
	Vec2<float> size_;
	Vec2<float> uv_;
	Vec2<float> center_;
	Vec2<float> scale_;
	Vec2<float> ofs_size_;
	GrpCol<float> col_;
	int blend_mode_;

	bool auto_center_;

	Vec2<float> pos_orig_;
	Vec2<float> size_orig_;
	Vec2<float> uv_orig_;
	Vec2<float> center_orig_;
	GrpCol<float> col_orig_;
	const TexMng::tex_ptr texture_;
	int layout_;
	int id_;
	const Vec2<int> *const window_size_;							// FIXME: 参照にするとコンテナに入れられない
	const float *const y_bottom_;

public:
	enum {
		LAYOUT_CENTER = 0,
		LAYOUT_LEFT		= (1 << 0),
		LAYOUT_RIGHT	= (1 << 1),
		LAYOUT_TOP		= (1 << 2),
		LAYOUT_BOTTOM = (1 << 3)
	};

	Widget(picojson::object& param, const TexMng::tex_ptr texture, const Vec2<int> *size, const float *y_bottom) :
		scale_(1,1),
		blend_mode_(),
		texture_(texture),
		layout_(LAYOUT_CENTER),
		id_(),
		window_size_(size),
		y_bottom_(y_bottom)
	{
		
    // FIXME: 以下、初期化リストで済ませたい
		{
			picojson::array& array = param["pos"].get<picojson::array>();
			pos_.set(array[0].get<double>(), array[1].get<double>());
			pos_orig_ = pos_;
		}
		{
			picojson::array& array = param["size"].get<picojson::array>();
			size_.set(array[0].get<double>(), array[1].get<double>());
			size_orig_ = size_;
		}
		{
			picojson::array& array = param["uv"].get<picojson::array>();
			uv_.set(array[0].get<double>(), array[1].get<double>());
			uv_orig_ = uv_;
		}
		{
			picojson::array& array = param["col"].get<picojson::array>();
			col_.set(array[0].get<double>(), array[1].get<double>(), array[2].get<double>(), array[3].get<double>());
			col_orig_ = col_;
		}
		{
			auto_center_ = !param["center"].is<picojson::array>();
			if (!auto_center_)
			{
				picojson::array& array = param["center"].get<picojson::array>();
				center_.set(array[0].get<double>(), array[1].get<double>());
				center_orig_ = center_;
			}
		}
		if (param["scale"].is<picojson::array>())
		{
			picojson::array& array = param["scale"].get<picojson::array>();
			scale_.set(array[0].get<double>(), array[1].get<double>());
		}

		if (param["ofs_size"].is<picojson::array>())
		{
			// UIで使う時のサイズに対するオフセット
			picojson::array& array = param["ofs_size"].get<picojson::array>();
			ofs_size_.set(array[0].get<double>(), array[1].get<double>());
		}

		if (param["x_layout"].is<double>())
		{
			layout_ |= (param["x_layout"].get<double>() > 0.0) ? LAYOUT_RIGHT : LAYOUT_LEFT;
		}
		if (param["y_layout"].is<double>())
		{
			layout_ |= (param["y_layout"].get<double>() > 0.0) ? LAYOUT_BOTTOM : LAYOUT_TOP;
		}
		if (param["id"].is<double>())
		{
			id_ = param["id"].get<double>();
		}
	}

	void setPos(const Vec2<float>& pos) { pos_ = pos; }
	void setPos(const float x, const float y) { pos_.set(x, y); }
	const Vec2<float>& pos() const { return pos_; }

	void setSize(const Vec2<float>& size) { size_ = size; }
	void setSize(const float w, const float h) { size_.set(w, h); }
	const Vec2<float>& size() const { return size_; }

	void setUv(const Vec2<float>& uv) { uv_ = uv; }
	void setUv(const float u, const float v) { uv_.set(u, v); }
	const Vec2<float>& uv() const { return uv_; }

	void setCenter(const Vec2<float>& center) {
		center_ = center;
		auto_center_ = false;
	}
	void setCenter(const float x, const float y) { center_.set(x, y); }
	const Vec2<float>& center() const { return center_; }

	void setScale(const Vec2<float>& scale) { scale_ = scale; }
	void setScale(const float w, const float h) { scale_.set(w, h); }
	const Vec2<float>& scale() const { return scale_; }

	void setCol(const GrpCol<float>& col) { col_ = col; }
	void setCol(const float r, const float g, const float b, const float a = 1) { col_.set(r, g, b, a); }
	const GrpCol<float>& col() const { return col_; }
	const GrpCol<float>& colOrig() const { return col_orig_; }

	const Vec2<float>& posOrig() const { return pos_orig_; }
	const Vec2<float>& sizeOrig() const { return size_orig_; }
	const Vec2<float>& uvOrig() const { return uv_orig_; }
	const Vec2<float>& centerOrig() const { return center_orig_; }
	const Vec2<float>& ofsSize() const { return ofs_size_; }

	const int id() const {return id_; }

	const int blend_mode() const { return blend_mode_; }
	void blend_mode(const int mode) { blend_mode_ = mode; }
	
	Vec2<float> dispPos() const
	{
		Vec2<float> pos = pos_;
		if (auto_center_)
		{
			pos.x -= size_.x / 2.0;
			pos.y -= size_.y / 2.0;
		}
		else
		{
			pos -= center_;
		}
		if (layout_ & LAYOUT_LEFT)
		{
			pos.x -= window_size_->x / 2;
		}
		else
		if (layout_ & LAYOUT_RIGHT)
		{
			pos.x += window_size_->x / 2;
		}
		if (layout_ & LAYOUT_TOP)
		{
			pos.y -= window_size_->y / 2;
		}
		else
		if (layout_ & LAYOUT_BOTTOM)
		{
			pos.y += window_size_->y / 2 - *y_bottom_;
		}

		return pos;
	}
	
	void draw()
	{
		GrpSprite obj;
		obj.size(size_);
		if (auto_center_)
		{
			obj.center();
		}
		else
		{
			obj.center(center_);
		}

		Vec2<float> pos = pos_;
		if (layout_ & LAYOUT_LEFT)
		{
			pos.x -= window_size_->x / 2;
		}
		else
		if (layout_ & LAYOUT_RIGHT)
		{
			pos.x += window_size_->x / 2;
		}
		if (layout_ & LAYOUT_TOP)
		{
			pos.y -= window_size_->y / 2;
		}
		else
		if (layout_ & LAYOUT_BOTTOM)
		{
			pos.y += window_size_->y / 2 - *y_bottom_;
		}
		
		obj.pos(pos);
		obj.texture(texture_);
		obj.uv(uv_);
		obj.scale(scale_);
		obj.col(col_);
		obj.blend(blend_mode_);
		obj.draw();
	}
	
};

}
