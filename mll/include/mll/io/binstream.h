#if !defined(__BINARY_STREAMS_INCLUDED__)
#define __BINARY_STREAMS_INCLUDED__

#include <istream>
#include <ostream>

#include <mll/debug/static_assert.h>
#include <mll/patterns/type_traits.h>

namespace mll
{
namespace io
{

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// class ibinstream ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
class ibinstream
{
	std::istream &m_stream;

public:
	ibinstream(std::istream &stream) : m_stream(stream) {}
	std::istream &stream() { return m_stream; }

	bool good() const { return m_stream.good(); }
	bool bad() const { return m_stream.bad(); }
	bool eof() const { return m_stream.eof(); }
	bool fail() const { return m_stream.fail(); }
};

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// class obinstream ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
class obinstream
{
	std::ostream &m_stream;
	
public:
	obinstream(std::ostream &stream) : m_stream(stream) {}
	std::ostream &stream() { return m_stream; }

	bool good() const { return m_stream.good(); }
	bool bad() const { return m_stream.bad(); }
	bool eof() const { return m_stream.eof(); }
	bool fail() const { return m_stream.fail(); }
};

//=====================================================================================//
//                                   do_raw_input()                                    //
//=====================================================================================//
template<typename T>
inline ibinstream &do_raw_input(ibinstream &stream, T &t)
{
	MLL_STATIC_ASSERT( !patterns::is_pointer<T>::value );
	MLL_STATIC_ASSERT( !patterns::is_enum<T>::value );
	stream.stream().read(reinterpret_cast<char*>(&t),sizeof(T));
	return stream;
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
inline ibinstream &operator>>(ibinstream &i, unsigned char &a)
{
	return do_raw_input(i,a);
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
inline ibinstream &operator>>(ibinstream &i, unsigned int &a)
{
	return do_raw_input(i,a);
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
inline ibinstream &operator>>(ibinstream &i, unsigned short &a)
{
	return do_raw_input(i,a);
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
inline ibinstream &operator>>(ibinstream &i, __wchar_t &a)
{
	return do_raw_input(i,a);
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
inline ibinstream &operator>>(ibinstream &i, unsigned long &a)
{
	return do_raw_input(i,a);
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
inline ibinstream &operator>>(ibinstream &i, signed char &a)
{
	return do_raw_input(i,a);
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
inline ibinstream &operator>>(ibinstream &i, signed int &a)
{
	return do_raw_input(i,a);
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
inline ibinstream &operator>>(ibinstream &i, signed short &a)
{
	return do_raw_input(i,a);
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
inline ibinstream &operator>>(ibinstream &i, signed long &a)
{
	return do_raw_input(i,a);
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
inline ibinstream &operator>>(ibinstream &i, char &a)
{
	return do_raw_input(i,a);
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
inline ibinstream &operator>>(ibinstream &i, __int64 &a)
{
	return do_raw_input(i,a);
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
inline ibinstream &operator>>(ibinstream &i, unsigned __int64 &a)
{
	return do_raw_input(i,a);
}

//=====================================================================================//
//                                    operator>>();                                    //
//=====================================================================================//
inline ibinstream &operator>>(ibinstream &i, bool &a)
{
	return do_raw_input(i,a);
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
inline ibinstream &operator>>(ibinstream &i, float &a)
{
	return do_raw_input(i,a);
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
inline ibinstream &operator>>(ibinstream &i, double &a)
{
	return do_raw_input(i,a);
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
inline ibinstream &operator>>(ibinstream &i, long double &a)
{
	return do_raw_input(i,a);
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// operator<<() //////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline obinstream &do_raw_output(obinstream &stream, const T &t)
{
	MLL_STATIC_ASSERT( !patterns::is_pointer<T>::value );
	MLL_STATIC_ASSERT( !patterns::is_enum<T>::value );
	stream.stream().write(reinterpret_cast<const char*>(&t),sizeof(T));
	return stream;
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
inline obinstream &operator<<(obinstream &i, unsigned char a)
{
	return do_raw_output(i,a);
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
inline obinstream &operator<<(obinstream &i, unsigned int a)
{
	return do_raw_output(i,a);
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
inline obinstream &operator<<(obinstream &i, unsigned short a)
{
	return do_raw_output(i,a);
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
inline obinstream &operator<<(obinstream &i, unsigned long a)
{
	return do_raw_output(i,a);
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
inline obinstream &operator<<(obinstream &i, signed char a)
{
	return do_raw_output(i,a);
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
inline obinstream &operator<<(obinstream &i, signed int a)
{
	return do_raw_output(i,a);
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
inline obinstream &operator<<(obinstream &i, signed short a)
{
	return do_raw_output(i,a);
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
inline obinstream &operator<<(obinstream &i, __wchar_t a)
{
	return do_raw_output(i,a);
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
inline obinstream &operator<<(obinstream &i, signed long a)
{
	return do_raw_output(i,a);
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
inline obinstream &operator<<(obinstream &i, char a)
{
	return do_raw_output(i,a);
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
inline obinstream &operator<<(obinstream &i, __int64 a)
{
	return do_raw_output(i,a);
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
inline obinstream &operator<<(obinstream &i, unsigned __int64 a)
{
	return do_raw_output(i,a);
}

//=====================================================================================//
//                                    operator<<();                                    //
//=====================================================================================//
inline obinstream &operator<<(obinstream &i, bool a)
{
	return do_raw_output(i,a);
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
inline obinstream &operator<<(obinstream &i, float a)
{
	return do_raw_output(i,a);
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
inline obinstream &operator<<(obinstream &i, double a)
{
	return do_raw_output(i,a);
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
inline obinstream &operator<<(obinstream &i, long double a)
{
	return do_raw_output(i,a);
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
inline obinstream &operator<<(obinstream &stream, const char *str)
{
	stream.stream().write(str,strlen(str)+1);
	return stream;
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
inline obinstream &operator<<(obinstream &stream, const std::string &str)
{
	stream.stream().write(str.c_str(),str.length()+1);
	return stream;
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
inline ibinstream &operator>>(ibinstream &stream, std::string &str)
{
	str.clear();
	char ch = 1;

	while(stream.stream())
	{
		stream >> ch;
		if(!ch) break;
		str += ch;
	}

	return stream;
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
inline obinstream &operator<<(obinstream &stream, const std::wstring &str)
{
	stream.stream().write(reinterpret_cast<const char *>(str.c_str()),(str.length()+1)*sizeof(wchar_t));
	return stream;
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
inline ibinstream &operator>>(ibinstream &stream, std::wstring &str)
{
	str.clear();
	wchar_t ch = 1;

	while( stream.stream() )
	{
		stream >> ch;
		if(!ch) break;
		str += ch;
	}

	return stream;
}

} // namespace 'io'
} // namespace 'mll'

#endif