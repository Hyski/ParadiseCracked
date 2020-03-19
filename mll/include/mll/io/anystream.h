#if !defined(__ANY_STREAM_INCLUDED__)
#define __ANY_STREAM_INCLUDED__

#include <mll/io/binstream.h>

namespace mll
{

namespace io
{

//=====================================================================================//
//                                    struct iomode                                    //
//=====================================================================================//
struct iomode
{
	typedef enum
	{
		binary,
		text
	} value_type;

};

template<class Tr>
class _oanystream;

template<class Tr>
class _ianystream;

struct _otextstream_traits;
struct _obinstream_traits;

typedef _oanystream<_otextstream_traits> oanytextstream;
typedef _oanystream<_obinstream_traits> oanybinstream;

struct _itextstream_traits;
struct _ibinstream_traits;

typedef _ianystream<_itextstream_traits> ianytextstream;
typedef _ianystream<_ibinstream_traits> ianybinstream;

//=====================================================================================//
//                                  class _ioreceiver                                  //
//=====================================================================================//
class _ioreceiver
{
public:
	virtual oanytextstream& execute(oanytextstream& s) = 0;
	virtual oanybinstream& execute(oanybinstream& s) = 0;
};

//=====================================================================================//
//                                  class oanystream                                   //
//=====================================================================================//
class oanystream
{
public:
	virtual ~oanystream() {}
	virtual oanystream& dispatch(_ioreceiver& r) = 0;
	virtual iomode::value_type mode(void) const = 0;
	virtual std::ostream& std_stream(void) = 0;
};

//=====================================================================================//
//                        class _oreceiver : public _ioreceiver                        //
//=====================================================================================//
template<class T>
class _oreceiver : public _ioreceiver
{
public:

	const T& m_value;

	_oreceiver(const T& value) : m_value(value) {}

	oanytextstream& execute(oanytextstream& s)
	{
		s.stream() << m_value;
		return s;
	}

	oanybinstream& execute(oanybinstream& s)
	{
		s.stream() << m_value;
		return s;
	}

};

//=====================================================================================//
//                              struct _anystream_traits                               //
//=====================================================================================//
template<class S,class H,class I>
struct _anystream_traits
{
	typedef S stream_type;
	typedef H stream_holder;
	typedef I stream_initiator;
};

//=====================================================================================//
//                                  class _oanystream                                  //
//=====================================================================================//

struct _obinstream_traits  : public _anystream_traits<obinstream,obinstream,std::ostream&> {};
struct _otextstream_traits : public _anystream_traits<std::ostream,std::ostream&,std::ostream&> {};

template<class Tr>
class _oanystream : public oanystream,
				    public _anystream_traits<typename Tr::stream_type,
										     typename Tr::stream_holder,
										     typename Tr::stream_initiator>
{
private:
	stream_holder m_stream;
public:

	_oanystream(stream_initiator stream) : m_stream(stream) {}
	virtual ~_oanystream() {}

	stream_type& stream(void) { return m_stream; }

	oanystream& dispatch(_ioreceiver& r) { return r.execute(*this); }

	iomode::value_type mode(void) const;

	std::ostream& std_stream(void);
};

template<class T>
inline oanystream& operator<<(oanystream& stream,const T& value)
{
	return stream.dispatch(_oreceiver<T>(value));
}

template<>
inline iomode::value_type _oanystream<_obinstream_traits>::mode(void) const
{ 
	return iomode::binary;
} 

template<>
inline iomode::value_type _oanystream<_otextstream_traits>::mode(void) const
{ 
	return iomode::text;
} 

template<>
inline std::ostream& _oanystream<_obinstream_traits>::std_stream(void)
{
	return m_stream.stream();
}

template<>
inline std::ostream& _oanystream<_otextstream_traits>::std_stream(void)
{
	return m_stream;
}

//=====================================================================================//
//                                  class _iireceiver                                  //
//=====================================================================================//
class _iireceiver
{
public:
	virtual ianytextstream& execute(ianytextstream& s) = 0;
	virtual ianybinstream& execute(ianybinstream& s) = 0;
};

//=====================================================================================//
//                                  class ianystream                                   //
//=====================================================================================//
class ianystream
{
public:
	virtual ~ianystream() {}
	virtual ianystream& dispatch(_iireceiver& r) = 0;
	virtual iomode::value_type mode(void) const = 0;
	virtual std::istream& std_stream(void) = 0;
};

//=====================================================================================//
//                        class _ireceiver : public _iireceiver                        //
//=====================================================================================//
template<class T>
class _ireceiver : public _iireceiver
{
public:

	T& m_value;

	_ireceiver(T& value) : m_value(value) {}

	ianytextstream& execute(ianytextstream& s)
	{
		s.stream() >> m_value;
		return s;
	}

	ianybinstream& execute(ianybinstream& s)
	{
		s.stream() >> m_value;
		return s;
	}

};

//=====================================================================================//
//                                  class _ianystream                                  //
//=====================================================================================//

struct _ibinstream_traits  : public _anystream_traits<ibinstream,ibinstream,std::istream&> {};
struct _itextstream_traits : public _anystream_traits<std::istream,std::istream&,std::istream&> {};

template<class Tr>
class _ianystream : public ianystream,
				    public _anystream_traits<typename Tr::stream_type,
										     typename Tr::stream_holder,
										     typename Tr::stream_initiator>
{
private:
	stream_holder m_stream;
public:

	_ianystream(stream_initiator stream) : m_stream(stream) {}
	virtual ~_ianystream() {}

	stream_type& stream(void) { return m_stream; }

	ianystream& dispatch(_iireceiver& r) { return r.execute(*this); }

	iomode::value_type mode(void) const;

	std::istream& std_stream(void);

};

template<class T>
inline ianystream& operator>>(ianystream& stream,T& value)
{
	return stream.dispatch(_ireceiver<T>(value));
}

template<>
inline iomode::value_type _ianystream<_ibinstream_traits>::mode(void) const
{ 
	return iomode::binary;
} 

template<>
inline iomode::value_type _ianystream<_itextstream_traits>::mode(void) const
{ 
	return iomode::text;
} 

template<>
inline std::istream& _ianystream<_ibinstream_traits>::std_stream(void)
{
	return m_stream.stream();
}

template<>
inline std::istream& _ianystream<_itextstream_traits>::std_stream(void)
{
	return m_stream;
}

} // namespace 'io'

} // namespace 'mll'

#endif