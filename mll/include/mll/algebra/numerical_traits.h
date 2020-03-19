#if !defined(__NUMERICAL_TRAITS_INCLUDED__)
#define __NUMERICAL_TRAITS_INCLUDED__

// 29.04.2002 - GvozdodeR
//   - исправил sin на sinf, cos на cosf в mll::algebra::numerical_traits<float>
//   - исправил тип возвращаемого значения функций get_null и get_one в
//     mll::algebra::numerical_traits<int> на int вместо float
//   - убрал вызовы функций ::sin и ::cos из методов sin и cos класса
//     mll::algebra::numerical_traits<int> в силу их бессмысленности. Теперь эти
//     функции всегда возвращают нуль и единицу соответственно
//   - исправил static_cast<int> на static_cast<float> в вызове sqrtf класса
//     mll::algebra::numerical_traits<int>
//   - изменил вызовы ::sin и ::cos на вызовы ::cosf и ::sinf в методах
//     sin и cos класса mll::algebra::numerical_traits<float>
//
// 27.05.2002 - GvozdodeR
//   - добавил метод abs

#include <math.h>

namespace mll
{

namespace algebra
{

template<class T>
class numerical_traits {};

/////////////////////////////////////////////////////////////////////////
/////////////////    struct numerical_traits<float>    //////////////////
/////////////////////////////////////////////////////////////////////////
template<>
class numerical_traits<float>
{
public:

	typedef float quotient_type;
	
	static quotient_type sqrt(float v) { return ::sqrtf(v); }
	static bool is_null(float v) { return v == 0.0f; }
	static quotient_type inverse(quotient_type qt) { return 1.0f/qt; }
	static quotient_type to_quotient_type(float v) { return v; }
	static float get_null(void) { return 0.0f; }
	static float get_one(void) { return 1.0f; }
	static bool is_bad_ray_intersection(float f) { return f == -1.0f; }
	static float get_bad_ray_intersection() { return -1.0f; }

	static float cos(float angle) { return ::cosf(angle); }
	static float sin(float angle) { return ::sinf(angle); }
	static float acos(float a) { return ::acosf(a); }
	static float asin(float a) { return ::asinf(a); }

	static float abs(float a) { return fabsf(a); }
};
/////////////////////////////////////////////////////////////////////////

//=====================================================================================//
//                           class numerical_traits<double>                            //
//=====================================================================================//
template<>
class numerical_traits<double>
{
public:

	typedef double quotient_type;
	
	static quotient_type sqrt(double v) { return ::sqrt(v); }
	static bool is_null(double v) { return v == 0.0; }
	static quotient_type inverse(quotient_type qt) { return 1.0/qt; }
	static quotient_type to_quotient_type(double v) { return v; }
	static double get_null(void) { return 0.0; }
	static double get_one(void) { return 1.0; }
	static bool is_bad_ray_intersection(double f) { return f == -1.0; }
	static double get_bad_ray_intersection() { return -1.0; }

	static double cos(double angle) { return ::cos(angle); }
	static double sin(double angle) { return ::sin(angle); }
	static double acos(double a) { return ::acos(a); }
	static double asin(double a) { return ::asin(a); }

	static double abs(double a) { return fabs(a); }
};

/////////////////////////////////////////////////////////////////////////
//////////////////    struct numerical_traits<int>    ///////////////////
/////////////////////////////////////////////////////////////////////////
template<>
class numerical_traits<int>
{
public:

	typedef float quotient_type;
	
	static quotient_type sqrt(int v) { return ::sqrtf(static_cast<float>(v)); }
	static bool is_null(int v) { return v == 0l; }
	static quotient_type inverse(quotient_type qt) { return 1.0f/qt; }
	static quotient_type to_quotient_type(int v) { return static_cast<float>(v); }
	static int get_null(void) { return 0l; }
	static int get_one(void) { return 1l; }
	static bool is_bad_ray_intersection(int f) { return f == -1; }
	static int get_bad_ray_intersection() { return -1; }

	static int cos(int) { return 1l; }
	static int sin(int) { return 0l; }

	static int acos(int) { return 0l; }
	static int asin(int) { return 0l; }

	static int abs(int a) { return abs(a); }
};

//=====================================================================================//
//                        class numerical_traits<unsigned int>                         //
//=====================================================================================//
template<>
class numerical_traits<unsigned int>
{
public:

	typedef float quotient_type;
	
	static quotient_type sqrt(unsigned v) { return ::sqrtf(static_cast<float>(v)); }
	static bool is_null(unsigned v) { return v == 0l; }
	static quotient_type inverse(quotient_type qt) { return 1.0f/qt; }
	static quotient_type to_quotient_type(unsigned v) { return static_cast<float>(v); }
	static unsigned get_null(void) { return 0l; }
	static unsigned get_one(void) { return 1l; }
	static bool is_bad_ray_intersection(unsigned f) { return f == 0xFFFFFFFF; }
	static unsigned get_bad_ray_intersection() { return 0xFFFFFFFF; }

	static unsigned cos(unsigned) { return 1l; }
	static unsigned sin(unsigned) { return 0l; }

	static unsigned acos(unsigned) { return 0l; }
	static unsigned asin(unsigned) { return 0l; }

	static unsigned abs(unsigned a) { return a; }
};

//=====================================================================================//
//                       class numerical_traits<unsigned short>                        //
//=====================================================================================//
template<>
class numerical_traits<unsigned short>
{
public:

	typedef float quotient_type;
	
	static quotient_type sqrt(unsigned short v) { return ::sqrtf(static_cast<float>(v)); }
	static bool is_null(unsigned short v) { return v == 0; }
	static quotient_type inverse(quotient_type qt) { return 1.0f/qt; }
	static quotient_type to_quotient_type(unsigned short v) { return static_cast<float>(v); }
	static unsigned short get_null(void) { return 0; }
	static unsigned short get_one(void) { return 1; }
	static bool is_bad_ray_intersection(unsigned short f) { return f == 0xFFFF; }
	static unsigned short get_bad_ray_intersection() { return 0xFFFF; }

	static unsigned short cos(unsigned short) { return 1; }
	static unsigned short sin(unsigned short) { return 0; }

	static unsigned short acos(unsigned short) { return 0; }
	static unsigned short asin(unsigned short) { return 0; }

	static unsigned short abs(unsigned short a) { return a; }
};

/////////////////////////////////////////////////////////////////////////

template<typename T>
bool is_bad_ray(T f)
{
	return numerical_traits<T>::is_bad_ray_intersection(f);
}

}	//	namesapce algebra

}	//	namesapce mll


#endif
