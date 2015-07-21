
#pragma once

//
// イージング
// SOURCE: http://robertpenner.com/easing/
// TODO: float, Vec2, Vec3 の透過的なサポート
//

#include <cmath>
#include "co_defines.hpp"
#include "co_vec2.hpp"
#include "co_vec3.hpp"
#include "co_vec4.hpp"


namespace ngs {

float EasingLinear(float t, float b, float c, float d) {
	return c*t/d + b;
}


float EasingBackIn(float t, float b, float c, float d) {
	float s = 1.70158;
	t/=d;
	return c*t*t*((s+1)*t - s) + b;
}

float EasingBackOut(float t, float b, float c, float d) {
	float s = 1.70158;
	t=t/d-1;
	return c*(t*t*((s+1)*t + s) + 1) + b;
}

float EasingBackInOut(float t, float b, float c, float d) {
	float s = 1.70158 * 1.525;
	if ((t/=d/2) < 1) return c/2*(t*t*((s+1)*t - s)) + b;
	t-=2;
	return c/2*(t*t*((s+1)*t + s) + 2) + b;
}


float EasingBounceOut(float t, float b, float c, float d) {
	if ((t/=d) < (1/2.75)) {
		return c*(7.5625*t*t) + b;
	} else if (t < (2/2.75)) {
		t-=(1.5/2.75);
		return c*(7.5625*t*t + .75) + b;
	} else if (t < (2.5/2.75)) {
		t-=(2.25/2.75);
		return c*(7.5625*t*t + .9375) + b;
	} else {
		t-=(2.625/2.75);
		return c*(7.5625*t*t + .984375) + b;
	}
}

float EasingBounceIn(float t, float b, float c, float d) {
	return c - EasingBounceOut(d-t, 0, c, d) + b;
}

float EasingBounceInOut(float t, float b, float c, float d) {
	if (t < d/2) return EasingBounceIn(t*2, 0, c, d) * .5 + b;
	else return EasingBounceOut(t*2-d, 0, c, d) * .5 + c*.5 + b;
}


float EasingCircIn(float t, float b, float c, float d) {
	t/=d;
	return -c * (std::sqrt(1 - t*t) - 1) + b;
}

float EasingCircOut(float t, float b, float c, float d) {
	t=t/d-1;
	return c * std::sqrt(1 - t*t) + b;
}

float EasingCircInOut(float t, float b, float c, float d) {
	if ((t/=d/2) < 1) return -c/2 * (std::sqrt(1 - t*t) - 1) + b;
	t-=2;
	return c/2 * (std::sqrt(1 - t*t) + 1) + b;
}


float EasingCubicIn(float t, float b, float c, float d) {
	t/=d;
	return c*t*t*t + b;
}

float EasingCubicOut(float t, float b, float c, float d) {
	t=t/d-1;
	return c*(t*t*t + 1) + b;
}

float EasingCubicInOut(float t, float b, float c, float d) {
	if ((t/=d/2) < 1) return c/2*t*t*t + b;
	t-=2;
	return c/2*(t*t*t + 2) + b;
}


float EasingElasticIn(float t, float b, float c, float d) {
	if (t==0) return b;
	if ((t/=d)==1) return b+c;
	float p = d*.3;

	float a, s;
	a=c; s=p/4;
	t-=1;
	return -(a * std::pow(2,10*t) * std::sin( (t*d-s)*(2 * PI)/p )) + b;
}

float EasingElasticOut(float t, float b, float c, float d) {
	if (t==0) return b;
	if ((t/=d)==1) return b+c;
	float p = d*.3;

	float a, s;
	a=c; s=p/4;
	return (a* std::pow(2,-10*t) * std::sin( (t*d-s)*(2 * PI)/p ) + c + b);
}

float EasingElasticInOut(float t, float b, float c, float d) {
	if (t==0) return b;
	if ((t/=d/2)==2) return b+c;
	float p = d*(.3*1.5);

	float a, s;
	a=c; s=p/4;
	if (t < 1)
	{
		t-=1;
		return -.5*(a * std::pow(2,10*t) * std::sin( (t*d-s)*(2 * PI)/p )) + b;
	}
	t-=1;
	return a * std::pow(2,-10*t) * std::sin( (t*d-s)*(2 * PI)/p )*.5 + c + b;
}


float EasingExpoIn(float t, float b, float c, float d) {
	return (t==0) ? b : c * std::pow(2, 10 * (t/d - 1)) + b;
}

float EasingExpoOut(float t, float b, float c, float d) {
	return (t==d) ? b+c : c * (-std::pow(2, -10 * t/d) + 1) + b;
}

float EasingExpoInOut(float t, float b, float c, float d) {
	if (t==0) return b;
	if (t==d) return b+c;
	if ((t/=d/2) < 1) return c/2 * std::pow(2, 10 * (t - 1)) + b;
	return c/2 * (-std::pow(2, -10 * --t) + 2) + b;
}


float EasingQuadIn(float t, float b, float c, float d) {
	t/=d;
	return c*t*t + b;
}

float EasingQuadOut(float t, float b, float c, float d) {
	t/=d;
	return -c *t*(t-2) + b;
}

float EasingQuadInOut(float t, float b, float c, float d) {
	if ((t/=d/2) < 1) return c/2*t*t + b;
	--t;
	return -c/2 * (t*(t-2) - 1) + b;
}


float EasingQuartIn(float t, float b, float c, float d) {
	t/=d;
	return c*t*t*t*t + b;
}

float EasingQuartOut(float t, float b, float c, float d) {
	t=t/d-1;
	return -c * (t*t*t*t - 1) + b;
}

float EasingQuartInOut(float t, float b, float c, float d) {
	if ((t/=d/2) < 1) return c/2*t*t*t*t + b;
	t-=2;
	return -c/2 * (t*t*t*t - 2) + b;
}


float EasingQuintIn(float t, float b, float c, float d) {
	t/=d;
	return c*t*t*t*t*t + b;
}

float EasingQuintOut(float t, float b, float c, float d) {
	t=t/d-1;
	return c*(t*t*t*t*t + 1) + b;
}

float EasingQuintInOut(float t, float b, float c, float d) {
	if ((t/=d/2) < 1) return c/2*t*t*t*t*t + b;
	t-=2;
	return c/2*(t*t*t*t*t + 2) + b;
}


float EasingSineIn(float t, float b, float c, float d) {
	return -c * std::cos(t/d * (PI/2)) + c + b;
}

float EasingSineOut(float t, float b, float c, float d) {
	return c * std::sin(t/d * (PI/2)) + b;
}

float EasingSineInOut(float t, float b, float c, float d) {
	return -c/2 * (std::cos(PI*t/d) - 1) + b;
}


enum {
	LINEAR,

	BACK_IN,
	BACK_OUT,
	BACK_INOUT,

	BOUNCE_IN,
	BOUNCE_OUT,
	BOUNCE_INOUT,

	CIRC_IN,
	CIRC_OUT,
	CIRC_INOUT,

	CUBIC_IN,
	CUBIC_OUT,
	CUBIC_INOUT,
	
	ELASTIC_IN,
	ELASTIC_OUT,
	ELASTIC_INOUT,
	
	EXPO_IN,
	EXPO_OUT,
	EXPO_INOUT,
	
	QUAD_IN,
	QUAD_OUT,
	QUAD_INOUT,
	
	QUART_IN,
	QUART_OUT,
	QUART_INOUT,
	
	QUINT_IN,
	QUINT_OUT,
	QUINT_INOUT,

	SINE_IN,
	SINE_OUT,
	SINE_INOUT,
};

float (* const easeFunc[])(float t, float b, float c, float d) = {
	EasingLinear,

	EasingBackIn,
	EasingBackOut,
	EasingBackInOut,

	EasingBounceIn,
	EasingBounceOut,
	EasingBounceInOut,

	EasingCircIn,
	EasingCircOut,
	EasingCircInOut,

	EasingCubicIn,
	EasingCubicOut,
	EasingCubicInOut,

	EasingElasticIn,
	EasingElasticOut,
	EasingElasticInOut,

	EasingExpoIn,
	EasingExpoOut,
	EasingExpoInOut,

	EasingQuadIn,
	EasingQuadOut,
	EasingQuadInOut,

	EasingQuartIn,
	EasingQuartOut,
	EasingQuartInOut,

	EasingQuintIn,
	EasingQuintOut,
	EasingQuintInOut,

	EasingSineIn,
	EasingSineOut,
	EasingSineInOut,
};
// FIXME:苦肉の策の関数テーブル


class Easing {
public:
	void ease(float& res, const float t, const float st, const float ed, const float d, const int type = LINEAR)
	{
		float time = 0.0 < t ? (t < d ? t : d) : 0.0;
		res = easeFunc[type](time, st, ed - st, d);
	}
		
	void ease(Vec2<float>& res, const float t, const Vec2<float>& st, const Vec2<float>& ed, const float d, const int type = LINEAR)
	{
		float time = 0.0 < t ? (t < d ? t : d) : 0.0;
		res.set(easeFunc[type](time, st.x, ed.x - st.x, d), easeFunc[type](time, st.y, ed.y - st.y, d));
	}

	void ease(Vec3<float>& res, const float t, const Vec3<float>& st, const Vec3<float>& ed, const float d, const int type = LINEAR)
	{
		float time = 0.0 < t ? (t < d ? t : d) : 0.0;
		res.set(easeFunc[type](time, st.x, ed.x - st.x, d), easeFunc[type](time, st.y, ed.y - st.y, d), easeFunc[type](time, st.z, ed.z - st.z, d));
	}

	void ease(Vec4<float>& res, const float t, const Vec4<float>& st, const Vec4<float>& ed, const float d, const int type = LINEAR)
	{
		float time = 0.0 < t ? (t < d ? t : d) : 0.0;
		res.set(easeFunc[type](time, st.x, ed.x - st.x, d), easeFunc[type](time, st.y, ed.y - st.y, d), easeFunc[type](time, st.z, ed.z - st.z, d), easeFunc[type](time, st.w, ed.w - st.w, d));
	}
};

}
