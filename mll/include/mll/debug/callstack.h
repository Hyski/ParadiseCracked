#if !defined(__CALLSTACK_H_INCLUDED_7552370787048739__)
#define __CALLSTACK_H_INCLUDED_7552370787048739__

#include <ostream>
#include <string>

#include <mll/_export_rules.h>
#include <mll/_choose_lib.h>
#include <mll/patterns/nocopy.h>

namespace mll
{
namespace debug
{

//=====================================================================================//
//                            class ExceptionHandlerInitor                             //
//=====================================================================================//
class MLL_EXPORT _exception_handler_initor
{
	void *m_oldFilter;

public:
	_exception_handler_initor();
	void dummy_func();
	~_exception_handler_initor();
};

//extern _exception_handler_initor MLL_EXPORT __MiST_land_Exception_Initor;

//namespace { struct _initor_user { _initor_user(){ __MiST_land_Exception_Initor.dummy_func(); } } _ex; }

//=====================================================================================//
//                             class MLL_EXPORT crush_info                             //
//=====================================================================================//
class MLL_EXPORT crash_info
{
public:
	static std::ostream &get_section(const std::string &);
	static void remove_section(const std::string &);
	static void flush_infos(std::ostream &);
};

class possible_info
{
	std::string m_section;
	
public:
	possible_info(const std::string &section)
	:	m_section(section)
	{
	}

	possible_info(const std::string &section, const std::string &message)
	:	m_section(section)
	{
		stream() << message;
	}
	
	~possible_info()
	{
		crash_info::remove_section(m_section);
	}

	std::ostream &stream() const { return crash_info::get_section(m_section); }
};

void MLL_EXPORT set_exception_message(const std::string &);
void MLL_EXPORT set_error_caption(const std::string &);

void MLL_EXPORT make_crash_log();

}	// 'debug'
}	// 'mll'

#endif // !defined(__CALLSTACK_H_INCLUDED_7552370787048739__)