#pragma once

/////////////////////////////////////////////////////////////////////////
//	STL Library files
#include <sstream>
#include <memory>
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//	MiST land Library files
#include <mll/_choose_lib.h>
#include <mll/_export_rules.h>
/////////////////////////////////////////////////////////////////////////

namespace mll
{

namespace utils { template<typename T> class oneobj; }

namespace debug
{

/**
\brief Создатель отчетов

класс накапливает для отчета информацию о произошедших ошибках, предупреждениях.
при удалении объекта отчет сбрасывается в виде html фала и по желанию запускается браузер
класс - типичный синглетон. instance() является одновременно и фабрикой.

module - первый параметр, по которому осуществляется сортировка (например 'Менеджер пути')
message - собственно ошибка или предупреждение, по нему тоже идет сортировка (например 'не найден файл')
params - конкретная информация (например 'файл: p_engine.dat') 
*/

//=====================================================================================//
//                                   class reporter                                    //
//=====================================================================================//
class MLL_EXPORT reporter
{
private:

	class isolation;
	std::auto_ptr<isolation> m_isolation;		///< объект внутренней реализации

private:
	reporter();
	~reporter();
	
public:
	enum report_kind
	{
		rk_error,
		rk_warning,
		rk_message,
		
		rk_count
	};
	
public:

public:
	static void add_report(report_kind k, const std::string &s1, const std::string &s2, const std::string &s3)
	{
		do_add_report(k,s1,s2,s3);
	}

	///	Изменить параметры
	static void set_parameters(const std::string& file_name, bool auto_start);
	/// Показать репорт и очистить его
	static void flush();

	/// Изменить имя файла с репортом.
	static void set_report_name(const std::string &report_name);
	/// Возвращает имя файла с репортом.
	static const std::string &get_report_name();

	/// Установить, будет ли автоматически стартовать броузер в случае наличия сообщений в репорте.
	static void set_auto_start(bool enable);
	/// Возвращает, будет ли автоматически стартовать броузер в случае наличия сообщений в репорте.
	static bool get_auto_start();

	/// Разрешить/запретить репорт.
	static void enable_report(bool enable);
	/// Возвращает, разрешен ли репорт.
	static bool is_report_enabled();

private:
	static void do_add_report(report_kind, const std::string &, const std::string &, const std::string &);

	///	получить экземпляр класса
	static reporter* instance(void);

	friend utils::oneobj<reporter>;
};

#define __MLL_DO_REPORT(type,module,message,parameter)														\
	do																										\
	{																										\
		std::ostringstream modulestr,messagestr,parameterstr;												\
		modulestr << module;																				\
		messagestr << message;																				\
		parameterstr << parameter;																			\
		::mll::debug::reporter::add_report(type,modulestr.str(),messagestr.str(),parameterstr.str());		\
	} while(false)

/// добавить в отчет предупреждение
#define MLL_REPORT_WARNING(module,message,parameter)	__MLL_DO_REPORT(::mll::debug::reporter::rk_warning,module,message,parameter)
/// добавить в отчет ошибку
#define MLL_REPORT_ERROR(module,message,parameter)		__MLL_DO_REPORT(::mll::debug::reporter::rk_error,module,message,parameter)
/// добавить в отчет сообщение
#define MLL_REPORT_MESSAGE(module,message,parameter)	__MLL_DO_REPORT(::mll::debug::reporter::rk_message,module,message,parameter)

#define MLL_REPORT(module,message,parameter) MLL_REPORT_ERROR(module,message,parameter)

}	//	namespace debug

}	//	namespace mll
