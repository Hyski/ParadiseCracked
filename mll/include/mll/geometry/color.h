#if !defined(__COLOR_INCLUDED__)
#define __COLOR_INCLUDED__

namespace mll 
{ 

namespace geometry
{

/////////////////////////////////////////////////////////////////////////
//////////////////////////    class color    ////////////////////////////
/////////////////////////////////////////////////////////////////////////
class color
{
public:

	unsigned int value;

	color();
	color(const color& c);
	color(unsigned int value);
	color(unsigned int r,unsigned int g,unsigned int b,unsigned int alpha = 0);

	~color();

	const color& operator=(const color& c);
};

inline color::color() : value(0xffffffff)
{
}

inline color::color(const color& c) : value(c.value)
{
}

inline color::color(unsigned int v) : value(v)
{
}

inline color::color(unsigned int r,unsigned int g,unsigned int b,unsigned int a) : value((r<<16)|(g<<8)|b|(a<<24))
{
}

inline color::~color()
{
}

inline const color& color::operator=(const color& c)
{
	value = c.value;
	return *this;
}

}	//	namespace debug

}	//	namespace mll

#endif