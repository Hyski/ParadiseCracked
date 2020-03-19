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
\brief ��������� �������

����� ����������� ��� ������ ���������� � ������������ �������, ���������������.
��� �������� ������� ����� ������������ � ���� html ���� � �� ������� ����������� �������
����� - �������� ���������. instance() �������� ������������ � ��������.

module - ������ ��������, �� �������� �������������� ���������� (�������� '�������� ����')
message - ���������� ������ ��� ��������������, �� ���� ���� ���� ���������� (�������� '�� ������ ����')
params - ���������� ���������� (�������� '����: p_engine.dat') 
*/

//=====================================================================================//
//                                   class reporter                                    //
//=====================================================================================//
class MLL_EXPORT reporter
{
private:

	class isolation;
	std::auto_ptr<isolation> m_isolation;		///< ������ ���������� ����������

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

	///	�������� ���������
	static void set_parameters(const std::string& file_name, bool auto_start);
	/// �������� ������ � �������� ���
	static void flush();

	/// �������� ��� ����� � ��������.
	static void set_report_name(const std::string &report_name);
	/// ���������� ��� ����� � ��������.
	static const std::string &get_report_name();

	/// ����������, ����� �� ������������� ���������� ������� � ������ ������� ��������� � �������.
	static void set_auto_start(bool enable);
	/// ����������, ����� �� ������������� ���������� ������� � ������ ������� ��������� � �������.
	static bool get_auto_start();

	/// ���������/��������� ������.
	static void enable_report(bool enable);
	/// ����������, �������� �� ������.
	static bool is_report_enabled();

private:
	static void do_add_report(report_kind, const std::string &, const std::string &, const std::string &);

	///	�������� ��������� ������
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

/// �������� � ����� ��������������
#define MLL_REPORT_WARNING(module,message,parameter)	__MLL_DO_REPORT(::mll::debug::reporter::rk_warning,module,message,parameter)
/// �������� � ����� ������
#define MLL_REPORT_ERROR(module,message,parameter)		__MLL_DO_REPORT(::mll::debug::reporter::rk_error,module,message,parameter)
/// �������� � ����� ���������
#define MLL_REPORT_MESSAGE(module,message,parameter)	__MLL_DO_REPORT(::mll::debug::reporter::rk_message,module,message,parameter)

#define MLL_REPORT(module,message,parameter) MLL_REPORT_ERROR(module,message,parameter)

}	//	namespace debug

}	//	namespace mll
