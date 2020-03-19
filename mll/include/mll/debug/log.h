#if !defined(__LOG_H_INCLUDED_7552370787048739__)
#define __LOG_H_INCLUDED_7552370787048739__

#include <mll/_export_rules.h>
#include <mll/_choose_lib.h>

#include <memory>
#include <ostream>
#include <string>

// ВНИМАНИЕ: определите #define MLL_DISABLE_ALL_LOGS для отключения ВСЕХ логов

namespace mll
{
namespace debug
{

/*=====================================================================================*\
 *                                      class log                                      * 
\*=====================================================================================*/
class MLL_EXPORT log
{
	friend class log_holder;

	std::auto_ptr<std::ostream> m_stream;

	log(const std::string &file);

public:
	log() {}
	log(const log &);

	std::ostream &get_stream() { return *m_stream; }
	bool valid() const { return m_stream.get() != 0; }
	void flush();

	void set_debug_out();
};

/*=====================================================================================*\
 *                                  class log_holder                                   * 
\*=====================================================================================*/
class MLL_EXPORT log_holder
{
	log_holder();

public:
	~log_holder();

	log &operator[] (const char *);
	log &debug_out();

	static log_holder &instance();
};

// Макросы для упрощения создания логов
#define __MLL_LOG(File,List)				{if((File).valid()) {(File).get_stream() << List; (File).flush(); }}

#if defined(_DEBUG)
#	define __MLL_DEBUG_LOG(File,List)		__MLL_LOG(File,List)
#else
#	define __MLL_DEBUG_LOG(File,List)
#endif

#define __MLL_MAKE_LOG_STREAM(Name)			::mll::debug::log_holder::instance()[Name]

#if defined(MLL_DISABLE_ALL_LOGS)
#define	MLL_MAKE_DEBUG_LOG(File,List)
#define MLL_MAKE_LOG(File,List)
#define MLL_MAKE_DEBUG_OLOG(List)
#define MLL_MAKE_OLOG(List)
#else
#define	MLL_MAKE_DEBUG_LOG(File,List)		__MLL_DEBUG_LOG(__MLL_MAKE_LOG_STREAM(File),List)
#define MLL_MAKE_LOG(File,List)				__MLL_LOG(__MLL_MAKE_LOG_STREAM(File),List)
#define MLL_MAKE_DEBUG_OLOG(List)			__MLL_DEBUG_LOG(::mll::debug::log_holder::instance().debug_out(),List)
#define MLL_MAKE_OLOG(List)					__MLL_LOG(::mll::debug::log_holder::instance().debug_out(),List)
#endif

// Пример создания собственного лога
// #define MY_LOG(List)	MLL_MAKE_DEBUG_LOG("my_file.log",List)

// Пример использования объявленного лога
// MY_LOG("Created instance of " << instance.name() << std::endl);

} // namespace 'debug'
} // namespace 'mll'

#endif // !defined(__LOG_H_INCLUDED_7552370787048739__)