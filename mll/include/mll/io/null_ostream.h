#if !defined(__NULL_OSTREAM_H_INCLUDED_7552370787048739__)
#define __NULL_OSTREAM_H_INCLUDED_7552370787048739__


#include <iostream>
#include <vector>
#include <memory>


namespace mll
{
namespace io
{

class null_ostream;


//=====================================================================================//
//                       class null_ostream: public std::ostream                       //
//=====================================================================================//
/*!	\class null_ostream
	Поток вывода, который пишет в никуда. 
*/
class null_ostream: public std::ostream
{
private:
	std::auto_ptr<null_ostreambuf> m_buf;

public:
	null_ostream();
};


//=====================================================================================//
//                    class null_ostreambuf: public std::streambuf                     //
//=====================================================================================//
/*!	\class null_ostreambuf
	Буфер для null_ostream. Внутренняя буферизация включена, в отладке содержимое буфера 
	можно посмотреть.
*/
class null_ostreambuf: public std::streambuf
{
private:
	typedef std::vector<char> buffer_t;
	buffer_t m_buf;

public:
	null_ostreambuf();

protected:
	virtual int_type overflow(int_type _Meta = traits_type::eof());
};





//=====================================================================================//
//                         null_ostreambuf::null_ostreambuf()                          //
//=====================================================================================//
inline null_ostreambuf::null_ostreambuf()
{
	m_buf.resize(100);
	setp(&m_buf[0], &m_buf[0] + 100);
}

//=====================================================================================//
//                std::streambuf::int_type null_ostreambuf::overflow()                 //
//=====================================================================================//
inline std::streambuf::int_type null_ostreambuf::overflow(int_type _Meta)
{
	setp(&m_buf[0], &m_buf[0] + 100);
	return 0;
}

//=====================================================================================//
//                            null_ostream::null_ostream()                             //
//=====================================================================================//
inline null_ostream::null_ostream()
: std::ostream(0)
{
	m_buf.reset(new null_ostreambuf());
	this->init(m_buf.get());
}

}
}

#endif // !defined(__NULL_OSTREAM_H_INCLUDED_7552370787048739__)