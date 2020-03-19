#if !defined(__EXCEPTION_INCLUDED__)
#define __EXCEPTION_INCLUDED__

#include <exception>
#include <string>
#include <mll/io/debug_ostream.h>

namespace mll 
{ 

namespace debug 
{

#if defined(_BREAK_ON_EXCEPTION)
#define MLL_MAKE_DBG_BREAK __asm { int 3 }
#else
#define MLL_MAKE_DBG_BREAK
#endif

//=====================================================================================//
//                                   class exception                                   //
//=====================================================================================//
//! Базовый класс-исключение
class exception : public std::exception
{
	std::string m_message;

public:
	//! \name Конструкторы
	//! @{

	//! Конструктор по умолчанию
	exception() : m_message("Unknown Error.") { writeDebugOut(); }
	//! Конструктор копирования
	exception(const exception& e) : m_message(e.m_message) {}
	//! Берет текст ошибки из std::string msg
	exception(const std::string &msg) : m_message(msg) { writeDebugOut(); }
	//! Берет текст ошибки из const char *msg
	exception(const char *msg) : m_message(msg) { writeDebugOut(); }

	//! @}

	//! Дает возможность узнать о сути ошибки
	//! \return строку с сообщением об ошибке
	virtual const char *what() const { return m_message.c_str(); }

	/// Добавляет информацию в начало сообщения об ошибке
	void prepend_info(const std::string &i)
	{
		m_message = i + m_message;
		MLL_DEBUG_OUT("mll::debug::exception: message changed [" << m_message << "]\n");
	}
	/// Добавляет информацию в конец сообщения об ошибке
	void append_info(const std::string &i)
	{
		m_message = m_message + i;
		MLL_DEBUG_OUT("mll::debug::exception: message changed [" << m_message << "]\n");
	}

	//! Устанавливает точку останова
	static void dbg_break() { __asm { int 3 } }

private:
	void writeDebugOut() const
	{
		MLL_DEBUG_OUT("mll::debug::exception: [" << m_message << "]\n");
	}
};

//=====================================================================================//
//                              class exception_template                              //
//=====================================================================================//
//! Шаблонный класс для упрощения создания собственных типов исключений
/*! \param B базовый класс-исключение
 *  \param T тэг исключения для однозначности
 *
 *  Параметр T используется для того, чтобы можно было создать несколько
 *  классов-исключений, наследованых от одного класса-исключения. 
 *  Для определения собственного класса-исключения используйте пример:
 *  \code
 *  typedef exception_template<struct MyExceptionTag, exception> MyException;
 *  \endcode
 */
template<typename T, typename B = exception>
class exception_template : public B
{
public:
	exception_template() {}
	exception_template(const exception_template& e) : B(e) {}
	exception_template(const std::string &msg) : B(msg) {}
	exception_template(const char *msg) : B(msg) {}
};

#undef MLL_MAKE_DBG_BREAK

#define MLL_MAKE_EXCEPTION(Name,Parent)										\
class Name : public Parent													\
{																			\
public:																		\
	Name() {}																\
	Name(const Name& e) : Parent(e) {}										\
	Name(const std::string &msg) : Parent(msg) {}							\
	Name(const char *msg) : Parent(msg) {}									\
}

}

}

#endif