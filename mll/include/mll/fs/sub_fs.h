#if !defined(__SUB_FS_H_INCLUDED_8016431795074442__)
#define __SUB_FS_H_INCLUDED_8016431795074442__


#include "fs_path.h"

#include <ostream>
#include <istream>
#include <memory>
#include <time.h>
#include <boost/shared_ptr.hpp>

namespace mll
{

namespace fs
{


class sub_fs_iter;


//=====================================================================================//
//                                    class sub_fs                                     //
//=====================================================================================//
class sub_fs
{
public:
	class priority_handler;

	struct sized_data
	{
		boost::shared_ptr<void> data;
		size_t length;
	};

public:
	virtual ~sub_fs() = 0 {}
	///	��������� ������-��������� �������
	virtual void set_priority_handler(std::auto_ptr<priority_handler> handler) = 0;
	///	�������� ���� �� ������� ���������� (��������, zip-����� etc), �� ������ �� �����
	virtual const std::string& disk_path() const = 0;
	/// ����������� �������� ����� ������ ����������
	virtual bool system_readonly() const = 0;
	/// ���������� �� ���� � ��������� ������
	virtual bool file_exists(const fs_path &name) const = 0;
	/// ������� �������� ������ ���������� �� ���������� ����
	virtual std::auto_ptr<sub_fs_iter> new_iterator(const fs_path &) const = 0;
	/// ������ ����� � ��������� ������ � ������
	virtual unsigned file_size(const fs_path &name) const = 0;
	/// ����������� ������ � ���� � ��������� ������
	virtual bool is_readonly(const fs_path &name) const = 0;
	/// ���� ���������� ��������� ����� � ��������� ������
	virtual time_t date(const fs_path &name) const = 0;
	/// �������� �� ���� � ��������� ������ �����������
	virtual bool is_dir(const fs_path &name) const = 0;
	/// ������� ��������� �� ����� istream ��� ����� � ��������� ������
	virtual std::auto_ptr<std::istream> r_open_file(const fs_path &path,
		bool seekable = true) const = 0;
	/// ������� ��������� �� ����� ostream ��� ����� � ��������� ������
	/** ���� ������ �������� append, ����� ������ ��������� 
		�� ����� �����.*/
	virtual std::auto_ptr<std::ostream> w_open_file(const fs_path &path,
		bool append, bool seekable = true) const = 0;
	/// ������� ���� � ��������� ������
	virtual void delete_file(const fs_path &name) const = 0;
	///	������� ����������
	virtual void make_dir(const fs_path &path) const = 0;
	/// ��������� ���������� �����.
	virtual sized_data read_file(const fs_path &path) const = 0;

public:		//	up_to_subsystem_priority
	virtual int priority() const = 0;
};


//=====================================================================================//
//                  class FILE_SYSTEM_EXPORT sub_fs::priority_handler                  //
//=====================================================================================//
class sub_fs::priority_handler
{
public:
	virtual ~priority_handler() {}
	virtual int priority() const = 0;
};

}

}

#endif // !defined(__SUB_FS_H_INCLUDED_8016431795074442__)