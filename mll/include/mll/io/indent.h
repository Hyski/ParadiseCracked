#if !defined(__INDENT_H_INCLUDED_8093570542662288__)
#define __INDENT_H_INCLUDED_8093570542662288__

#include <ostream>
#include <string>
#include <streambuf>
#include <list>

namespace mll
{
namespace io
{

//=====================================================================================//
//                                    class indent                                     //
//=====================================================================================//
class indent : private std::streambuf
{
	friend class auto_indent;

	typedef std::list<int> indent_list;

	std::ostream * const m_stream;
	std::streambuf * const m_rdbuf;
	int_type m_indentSymbol;
	int m_indentStep;

	int m_pos;

	int_type m_indentAfter;
	int_type m_unindentBefore;

	int m_indent;
	bool m_needIndent;

	indent_list m_indentList;

	void _indent();

public:
	indent(std::ostream *stream, int_type indentSymbol = ' ', int indentStep = 4);
	virtual ~indent();

	inline void do_indent(int count = 1);
	inline void undo_indent(int count = 1);
	inline void at();

	inline int_type indent_symbol() const { return m_indentSymbol; }
	inline int indent_step() const { return m_indentStep; }
	inline int_type indent_after() const { return m_indentAfter; }
	inline int_type unindent_before() const { return m_unindentBefore; }

	inline int_type indent_symbol(int_type indentSymbol);
	inline int indent_step(int indentStep);
	inline int_type indent_after(int_type c);
	inline int_type unindent_before(int_type c);

protected:
	virtual int_type overflow(int c);
	virtual int sync() { return m_rdbuf->pubsync(); }
};

//=====================================================================================//
//                                  class auto_indent                                  //
//=====================================================================================//
class auto_indent
{
	indent &m_indent;
	const int m_count;

public:
	auto_indent(std::ostream &s, int count = 1)
	: m_indent(static_cast<indent&>(*s.rdbuf())), m_count(count)
	{
		m_indent.do_indent(m_count);
	}

	~auto_indent() {m_indent.undo_indent(m_count);}
};

#pragma warning(push)
#pragma warning(disable:4355)
//=====================================================================================//
//                                  indent::indent()                                   //
//=====================================================================================//
inline indent::indent(std::ostream *stream, int_type indentSymbol, int indentStep)
:	m_stream(stream),
	m_rdbuf(m_stream->rdbuf(this)),
	m_indent(0),
	m_indentSymbol(indentSymbol),
	m_indentAfter(-512),
	m_unindentBefore(-512),
	m_indentStep(indentStep),
	m_pos(0) // не всегда будет правильно
{
}
#pragma warning(pop)

//=====================================================================================//
//                                  indent::~indent()                                  //
//=====================================================================================//
inline indent::~indent()
{
	_indent();
	m_stream->rdbuf(m_rdbuf);
}

//=====================================================================================//
//                                  indent::_indent()                                  //
//=====================================================================================//
inline void indent::_indent()
{
	if(m_needIndent)
	{
		for(int i = 0; i < m_indent; ++i)
		{
			m_rdbuf->sputc(m_indentSymbol);
		}
		m_pos += m_indent;
		m_needIndent = false;
	}
}

//=====================================================================================//
//                                 indent::do_indent()                                 //
//=====================================================================================//
inline void indent::do_indent(int count)
{
	m_indent += count * m_indentStep;
	m_indentList.push_back(count*m_indentStep);
}

//=====================================================================================//
//                                indent::undo_indent()                                //
//=====================================================================================//
inline void indent::undo_indent(int count)
{
	for(int i = 0; i < count; ++i)
	{
		m_indent -= m_indentList.back();
		m_indentList.pop_back();
	}
}

//=====================================================================================//
//                                    indent::at()                                     //
//=====================================================================================//
inline void indent::at()
{
	m_indentList.push_back(m_pos-m_indent);
	m_indent = m_pos;
}

//=====================================================================================//
//                                 indent::overflow()                                  //
//=====================================================================================//
inline indent::int_type indent::overflow(int_type i)
{
	if(i == m_unindentBefore)
	{
		undo_indent();
	}

	_indent();

	if(i == '\n')
	{
		m_needIndent = true;
		m_pos = 0;
	}

	if(i == m_indentAfter)
	{
		do_indent();
	}

	if(i == '\n')
	{
		m_pos = 0;
	}
	else
	{
		++m_pos;
	}
	return m_rdbuf->sputc(i);
}

//=====================================================================================//
//                               indent::indent_symbol()                               //
//=====================================================================================//
inline indent::int_type indent::indent_symbol(int_type indentSymbol)
{
	int_type t = m_indentSymbol;
	m_indentSymbol = indentSymbol;
	return t;
}

//=====================================================================================//
//                                indent::indent_step()                                //
//=====================================================================================//
inline int indent::indent_step(int indentStep)
{
	int t = m_indentStep;
	m_indentStep = indentStep;
	return t;
}

//=====================================================================================//
//                               indent::indent_after()                                //
//=====================================================================================//
inline indent::int_type indent::indent_after(int_type c)
{
	int_type t = m_indentAfter;
	m_indentAfter = c;
	return t;
}


//=====================================================================================//
//                              indent::unindent_before()                              //
//=====================================================================================//
inline indent::int_type indent::unindent_before(int_type c)
{
	int_type t = m_unindentBefore;
	m_unindentBefore = c;
	return t;
}


} // namespace 'io'
} // namespace 'mll'

#endif // !defined(__INDENT_H_INCLUDED_8093570542662288__)