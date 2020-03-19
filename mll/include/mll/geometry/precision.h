#if !defined(__PRECISION_H_INCLUDED_7552370787048739__)
#define __PRECISION_H_INCLUDED_7552370787048739__

#include <mll/debug/static_assert.h>

namespace mll
{
namespace geometry
{

template<unsigned a>
struct _get_shift
{
	MLL_STATIC_ASSERT( (a&1) == 0 );
	enum { value = _get_shift< a>>1 >::value };
};

template<>
struct _get_shift<1> { enum { value = 1 }; };

template<typename F>
struct precision {};

//=====================================================================================//
//                               struct precision<float>                               //
//=====================================================================================//
template<>
struct precision<float>
{
	typedef float number_type;
	typedef ::mll::algebra::numerical_traits<float> traits_type;

	typedef ::mll::algebra::base_vector3<float> vector3;
	typedef ::mll::algebra::base_vector2<float> vector2;
	typedef ::mll::algebra::base_point3<float> point3;
	typedef ::mll::algebra::base_point2<float> point2;

	enum
	{
		max_used_number			= 32768,
		max_used_exponent		= _get_shift<max_used_number>::value,
	};

	static float error()
	{
		return std::numeric_limits<float>::epsilon();
	}

	static float max_error()
	{
		return std::numeric_limits<float>::epsilon()*max_used_number;
	}

	static bool equals(float a, float b)
	{
		return traits_type::abs(a-b) <= max_error();
	}

	static bool equals(const point3 &pt1, const point3 &pt2)
	{
		return equals(pt1.x,pt2.x) && equals(pt1.y,pt2.y) && equals(pt1.z,pt2.z);
	}

	static bool equals(const vector3 &pt1, const vector3 &pt2)
	{
		return equals(pt1.x,pt2.x) && equals(pt1.y,pt2.y) && equals(pt1.z,pt2.z);
	}

	static bool equals(const vector2 &pt1, const vector2 &pt2)
	{
		return equals(pt1.x,pt2.x) && equals(pt1.y,pt2.y);
	}

	static bool equals(const point2 &pt1, const point2 &pt2)
	{
		return equals(pt1.x,pt2.x) && equals(pt1.y,pt2.y);
	}
};

} // namespace 'geometry'
} // namespace 'mll'

#endif // !defined(__PRECISION_H_INCLUDED_7552370787048739__)