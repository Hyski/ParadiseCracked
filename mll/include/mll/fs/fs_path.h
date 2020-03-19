#if !defined(__FS_PATH_H_INCLUDED_5992692519605117__)
#define __FS_PATH_H_INCLUDED_5992692519605117__


#pragma warning( disable : 4251)


#include "_export_rules.h"
#include "_choose_lib.h"

#include <iterator>
#include <string>
#include <list>


namespace mll
{

namespace fs
{


//=====================================================================================//
//                                    class fs_path                                    //
//=====================================================================================//
/*	—лужебный класс, призван заменить строку-путь.
	ѕри конструировании разбивает строку по слэшам и заносит
	в список имена каталогов. ћожет "превращатьс€" обратно в строку.
*/
class FILE_SYSTEM_EXPORT fs_path
{
public:
	class const_iterator;
	typedef std::list<int> separators_t;		///<	позиции, по которым в m_data наход€тс€ слеши (считаетс€, чтоо в конце строки тоже есть слеш - дл€ удобства)
	typedef std::string data_t;

private:	
	data_t m_data;	
	separators_t m_separators;

public:
	fs_path();
	fs_path(const std::string& data);
	fs_path(const char* data);
	fs_path(const fs_path& another);
	~fs_path() {}

public:
	const_iterator begin() const;
	const_iterator end() const;
	std::string front() const;
	std::string back() const;
	void pop_front();
	void pop_back();
	int length() const;
	void clear();
	const std::string& to_str() const;

public:
	fs_path& operator+=(const fs_path& another);
	fs_path& operator=(const fs_path& another);

private:
	void analyzeRawData();
};


//=====================================================================================//
//                                fs_path operator+();                                 //
//=====================================================================================//
fs_path operator+(const fs_path& first, const fs_path& second);


//=====================================================================================//
//                            class fs_path::const_iterator                            //
//=====================================================================================//
class FILE_SYSTEM_EXPORT fs_path::const_iterator: public std::iterator<std::bidirectional_iterator_tag, std::string>
{
private:
	friend fs_path;
	std::string m_selecting;
	const fs_path::data_t* m_data;
	const fs_path::separators_t* m_separators;
	int m_dataIndex;										///<	"from" point in controlled sequence
	fs_path::separators_t::const_iterator m_sepIt;		///<	next separator

public:
	const_iterator();
	const_iterator(const const_iterator& another);
	~const_iterator();

public:
	bool operator==(const const_iterator& another) const { return ((m_dataIndex == another.m_dataIndex) && (m_sepIt == another.m_sepIt)); }
	bool operator!=(const const_iterator& another) const { return ((m_dataIndex != another.m_dataIndex) || (m_sepIt != another.m_sepIt)); }

	const_iterator& operator++();
	const_iterator operator++(int);
	const_iterator& operator--();
	const_iterator operator--(int);

	const std::string& operator*() const;
	const std::string* operator->() const;

	const_iterator& operator=(const const_iterator& another);
};

}

}

#endif // !defined(__FS_PATH_H_INCLUDED_5992692519605117__)