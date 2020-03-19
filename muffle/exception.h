#if !defined(__EXCEPTION_H_INCLUDED_8093570542662288__)
#define __EXCEPTION_H_INCLUDED_8093570542662288__

#if defined(BREAK_ON_EXCEPTIONS)
#define ML_MAKE_DBG_BREAK __asm { int 3 }
#else
#define ML_MAKE_DBG_BREAK
#endif

static class nobreak_t {} nobreak;

//=====================================================================================//
//                      class sound_error : public std::exception                      //
//=====================================================================================//
class sound_error : public std::exception, public CasusImprovisus
{
	class DbgBreaker
	{
	public:
		DbgBreaker() { dbgBreak(); }
		DbgBreaker(nobreak_t) {}
	};

	DbgBreaker m_dbgbrk;
	std::string m_message;

public:
	sound_error(const std::string &msg) : m_message(msg) {}
	sound_error(const char *msg) : m_message(msg) {}
	sound_error(const std::string &msg, nobreak_t) : m_dbgbrk(nobreak), m_message(msg) {}
	sound_error(const char *msg, nobreak_t) : m_dbgbrk(nobreak), m_message(msg) {}

	virtual const char *what() const { return m_message.c_str(); }
	virtual const char *Content() { return m_message.c_str(); }
	static void dbgBreak() { ML_MAKE_DBG_BREAK; }
};

//=====================================================================================//
//                  class cannot_create_decoder : public sound_error                   //
//=====================================================================================//
class cannot_create_decoder : public sound_error
{
public:
	cannot_create_decoder(const std::string &msg) : sound_error(msg,nobreak) {dbgBreak();}
	cannot_create_decoder(const char *msg) : sound_error(msg,nobreak) {dbgBreak();}
	cannot_create_decoder(const std::string &msg, nobreak_t) : sound_error(msg,nobreak) {}
	cannot_create_decoder(const char *msg, nobreak_t) : sound_error(msg,nobreak) {}
};

//=====================================================================================//
//                      class file_not_found : public sound_error                      //
//=====================================================================================//
class file_not_found : public sound_error
{
public:
	file_not_found(const std::string &msg) : sound_error(msg,nobreak) {dbgBreak();}
	file_not_found(const char *msg) : sound_error(msg,nobreak) {dbgBreak();}
	file_not_found(const std::string &msg, nobreak_t) : sound_error(msg,nobreak) {}
	file_not_found(const char *msg, nobreak_t) : sound_error(msg,nobreak) {}
};

#undef ML_MAKE_DBG_BREAK

#endif // !defined(__EXCEPTION_H_INCLUDED_8093570542662288__)