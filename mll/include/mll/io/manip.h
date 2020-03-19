#if !defined(__MANIP_INCLUDED__)
#define __MANIP_INCLUDED__

#include <mll/_export_rules.h>
#include <mll/io/binstream.h>
#include <mll/_choose_lib.h>
#include <mll/debug/exception.h>

namespace mll
{

namespace io
{

typedef debug::exception_template<struct _exact_manip_failure_tag> exact_manip_failure;

//=====================================================================================//
//                              struct _text_manipulator                               //
//=====================================================================================//
struct MLL_EXPORT _text_manipulator
{
	const char* m_value;
	_text_manipulator(const char* value) : m_value(value) {}
	void do_it(std::istream &);
};

inline _text_manipulator text(const char* value)
{
	return _text_manipulator(value);
}

inline std::ostream& operator<<(std::ostream& stream,const _text_manipulator& tm)
{
	stream << tm.m_value;
	return stream;
}

inline obinstream& operator<<(obinstream& stream,const _text_manipulator& tm)
{
	return stream;
}

inline std::istream& operator>>(std::istream& stream,_text_manipulator& tm)
{
	tm.do_it(stream);//stream.ignore(strlen(tm.m_value));
	return stream;
}

inline ibinstream& operator>>(ibinstream& stream,_text_manipulator& tm)
{
	return stream;
}

//=====================================================================================//
//                     struct _const_exact_string_manipulator                          //
//=====================================================================================//
struct MLL_EXPORT _const_exact_string_manipulator
{
	const std::string& m_value;
	_const_exact_string_manipulator(const std::string& value) : m_value(value) {}
	std::ostream& save(std::ostream& stream) const;
};

inline _const_exact_string_manipulator exact(const std::string& value)
{
	return _const_exact_string_manipulator(value);
}

inline _const_exact_string_manipulator exact_write(const std::string &value)
{
	return exact(value);
}

inline std::ostream& operator<<(std::ostream& stream,const _const_exact_string_manipulator& m)
{
	return m.save(stream);
}

inline obinstream& operator<<(obinstream& stream,const _const_exact_string_manipulator& m)
{
	stream << m.m_value;
	return stream;
}

//=====================================================================================//
//                          struct _exact_string_manipulator                           //
//=====================================================================================//
struct MLL_EXPORT _exact_string_manipulator
{
	std::string& m_value;
	_exact_string_manipulator(std::string& value) : m_value(value) {}
	std::istream& load(std::istream& stream);
};

inline _exact_string_manipulator exact(std::string& value)
{
	return _exact_string_manipulator(value);
}

inline _exact_string_manipulator exact_read(std::string &value)
{
	return exact(value);
}

inline std::istream& operator>>(std::istream& stream,_exact_string_manipulator& m)
{
	m.load(stream);
	return stream;
}

inline ibinstream& operator>>(ibinstream& stream,_exact_string_manipulator& m)
{
	stream >> m.m_value;
	return stream;
}

//=====================================================================================//
//                     struct _const_exact_wstring_manipulator                         //
//=====================================================================================//
struct MLL_EXPORT _const_exact_wstring_manipulator
{
	const std::wstring& m_value;
	_const_exact_wstring_manipulator(const std::wstring& value) : m_value(value) {}
	std::ostream& save(std::ostream& stream) const;
};

inline _const_exact_wstring_manipulator exact(const std::wstring& value)
{
	return _const_exact_wstring_manipulator(value);
}

inline _const_exact_wstring_manipulator exact_write(const std::wstring &value)
{
	return exact(value);
}

inline std::ostream& operator<<(std::ostream& stream,const _const_exact_wstring_manipulator& m)
{
	return m.save(stream);
}

inline obinstream& operator<<(obinstream& stream,const _const_exact_wstring_manipulator& m)
{
	stream << m.m_value;
	return stream;
}

//=====================================================================================//
//                          struct _exact_wstring_manipulator                          //
//=====================================================================================//
struct MLL_EXPORT _exact_wstring_manipulator
{
	std::wstring& m_value;
	_exact_wstring_manipulator(std::wstring& value) : m_value(value) {}
	std::istream& load(std::istream& stream);
};

inline _exact_wstring_manipulator exact(std::wstring& value)
{
	return _exact_wstring_manipulator(value);
}

inline _exact_wstring_manipulator exact_read(std::wstring &value)
{
	return exact(value);
}

inline std::istream& operator>>(std::istream& stream,_exact_wstring_manipulator& m)
{
	m.load(stream);
	return stream;
}

inline ibinstream& operator>>(ibinstream& stream,_exact_wstring_manipulator& m)
{
	stream >> m.m_value;
	return stream;
}

//=====================================================================================//
//                              struct _array_manipulator                              //
//=====================================================================================//
template<class It>
struct _array_manipulator
{
	typedef It iterator;

	iterator m_begin;
	iterator m_end;

	_array_manipulator(const iterator& begin,const iterator& end) : m_begin(begin),
																	m_end(end)
																    {}
};

template<class It>
inline _array_manipulator<It> array(const It& begin,const It& end)
{
	return _array_manipulator<It>(begin,end);
}

template<class It>
inline std::ostream& operator<<(std::ostream& stream,const _array_manipulator<It>& am)
{
	for(It it=am.m_begin;it!=am.m_end;++it)
		stream << *it << " ";
	return stream;
}

template<class It>
inline obinstream& operator<<(obinstream& stream,const _array_manipulator<It>& am)
{
	for(It it=am.m_begin;it!=am.m_end;++it)
		stream << *it;
	return stream;
}

template<class It>
inline std::istream& operator>>(std::istream& stream,_array_manipulator<It>& am)
{
	for(It it=am.m_begin;it!=am.m_end;++it)
		stream >> *it  >> text(" ");
	return stream;
}

template<class It>
inline ibinstream& operator>>(ibinstream& stream,_array_manipulator<It>& am)
{
	for(It it=am.m_begin;it!=am.m_end;++it)
		stream >> *it;
	return stream;
}

//=====================================================================================//
//                              struct _feed_manipulator                               //
//=====================================================================================//
template<class InIt>
struct _feed_manipulator
{
	typedef InIt insert_iterator;

	insert_iterator m_begin;
	unsigned int m_size;

	_feed_manipulator(const insert_iterator& begin,unsigned int size) : m_begin(begin),
																	    m_size(size)
																	    {}
};

template<class InIt>
inline _feed_manipulator<InIt> feed(const InIt& begin,unsigned int size)
{
	return _feed_manipulator<InIt>(begin,size);
}

template<class InIt>
inline std::istream& operator>>(std::istream& stream,_feed_manipulator<InIt>& fm)
{
	InIt it = fm.m_begin;
	for(unsigned i=0;i<fm.m_size;++i)
	{
		stream >> *it++;
	}
	return stream;
}

template<class InIt>
inline ibinstream& operator>>(ibinstream& stream,_feed_manipulator<InIt>& fm)
{
	InIt it = fm.m_begin;
	for(unsigned i=0;i<fm.m_size;++i)
	{
		stream >> *it++;
	}
	return stream;
}

} // namespace 'io'

} // namespace 'mll'

#endif
