#if !defined(__DEBUG_OSTREAM_H_INCLUDED_7511533552742567__)
#define __DEBUG_OSTREAM_H_INCLUDED_7511533552742567__


#include <iostream>
#include <memory>
#include <string>


extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char *);


namespace mll
{

namespace io
{

//=====================================================================================//
//                    class debug_ostreambuf: public std::streambuf                    //
//=====================================================================================//
/*!	\class debug_ostreambuf
	Буфер для debug_ostream, внутренняя буферизация отключена.
*/
class debug_ostreambuf: public std::streambuf
{
protected:
	char m_buf[100];

public:
	debug_ostreambuf();

protected:
	virtual int_type overflow(int_type _Meta = std::streambuf::traits_type::eof());
	virtual int sync();
};
////////////////////////////////////////////////////////
inline debug_ostreambuf::debug_ostreambuf()
{
	setp(&m_buf[0], &m_buf[98]);
	m_buf[99] = 0;
}
////////////////////////////////////////////////////////
inline std::streambuf::int_type debug_ostreambuf::overflow(int_type _Meta)
{
	*pptr() = 0;
	OutputDebugStringA(m_buf);
	m_buf[0] = traits_type::to_char_type(_Meta);
	setp(&m_buf[1], &m_buf[98]);
	return traits_type::not_eof(_Meta);
}
////////////////////////////////////////////////////////
inline int debug_ostreambuf::sync()
{
    overflow(0);
    return 0; 
}


//=====================================================================================//
//                      class debug_ostream: public std::ostream                       //
//=====================================================================================//
/*!	\class debug_ostream
	Поток вывода, который пишет в окно Output. Используйте все стандартные функции.
*/
class debug_ostream: public std::ostream
{
protected:
	debug_ostreambuf m_buf;

public:
	debug_ostream();
};
////////////////////////////////////////////////////////
inline debug_ostream::debug_ostream()
: std::ostream(0)
{
	init(&m_buf);
}


#define MLL_DEBUG_OUT(String)	\
	static_cast<std::ostream&>(::mll::io::debug_ostream()) << String << std::flush

}

}

#endif // !defined(__DEBUG_OSTREAM_H_INCLUDED_7511533552742567__)