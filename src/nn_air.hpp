
#pragma once

//
// 大気の描画
//

#include <vector>

namespace ngs {

class Air {
	struct Vtx {
		GLfloat x, y;
	};
	struct Uv {
		GLfloat u, v;
	};
	struct AirVtx {
		Vtx vtx;
		Uv uv;
	};
	
	std::vector<AirVtx> vtx_;

	GLuint vbo_;
	
public:
	Air(const int div, const float radius, const float hole)
	{
		vtx_.reserve(64);

		for (int i = 0; i <= div; ++i)
		{
			float r = (PI * -2.0f * (float)i) / div;
			float sin_r = sin(r);
			float cos_r = cos(r);
			
			AirVtx v_out = {
				{ radius * sin_r, radius * cos_r },
				{ 0.5, 0 }
			};
			vtx_.push_back(v_out);

			AirVtx v_in = {
				{ hole * sin_r, hole * cos_r },
				{ 0.5, 1 }
			};
			vtx_.push_back(v_in);
		}

		if (use_glex_vbo)
		{
			glGenBuffers(1, &vbo_);

			glBindBuffer(GL_ARRAY_BUFFER, vbo_);
			glBufferData(GL_ARRAY_BUFFER, sizeof(AirVtx) * vtx_.size(), &vtx_[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			// 割り当てを解除しておく
		}
	}
	
	~Air()
	{
		if (use_glex_vbo)
		{
			glDeleteBuffers(1, &vbo_);
		}
	}

	void draw()
	{
		glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		if (use_glex_vbo)
		{
			glBindBuffer(GL_ARRAY_BUFFER, vbo_);
			glVertexPointer(2, GL_FLOAT, sizeof(AirVtx), 0);
			glTexCoordPointer(2, GL_FLOAT, sizeof(AirVtx), (GLvoid *)(sizeof(Vtx)));
		
			glDrawArrays(GL_TRIANGLE_STRIP, 0, vtx_.size());

			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		else
		{
			glVertexPointer(2, GL_FLOAT, sizeof(AirVtx), &vtx_[0]);
			glTexCoordPointer(2, GL_FLOAT, sizeof(AirVtx), (GLbyte *)(&vtx_[0]) + (sizeof(Vtx)));
			glDrawArrays(GL_TRIANGLE_STRIP, 0,vtx_.size());
		}

		glPopClientAttrib();
	}
	
};

}

