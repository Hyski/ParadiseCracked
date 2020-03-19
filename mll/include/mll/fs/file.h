#if !defined(__FILE_H_INCLUDED_8093570542662288__)
#define __FILE_H_INCLUDED_8093570542662288__


#pragma warning( disable : 4251)


#include "file_id.h"

#include <ostream>
#include <istream>


namespace mll
{

namespace fs
{


//=====================================================================================//
//                                     class o_file                                    //
//=====================================================================================//

/** \class o_file
	\brief Класс - "файл для записи".
*/
/**	Класс, содержащий уникальный идентификатор 
	и указатель на поток ostream.
*/

class o_file
{
	file_id m_id;
	std::auto_ptr<std::ostream> m_stream;
	
public:
	o_file(const file_id &id, std::auto_ptr<std::ostream> stream)
		: m_id(id), m_stream(stream) {}
	~o_file() {}
	/// Возвращает ссылку на уникальный идентификатор
	const file_id &id() const { return m_id; }
	/// Возвращает ссылку на поток
	std::ostream &stream() const { return *m_stream; }
};

//=====================================================================================//
//                                     class i_file                                    //
//=====================================================================================//

/** \class i_file
	\brief Класс - "файл для чтения".
*/
/**	Класс, содержащий уникальный идентификатор 
	и указатель на поток istream.
*/

class i_file
{
	file_id m_id;
	std::auto_ptr<std::istream> m_stream;
	
public:
	i_file(const file_id &id, std::auto_ptr<std::istream> stream)
		: m_id(id), m_stream(stream) {}
	~i_file() {}
	/// Возвращает ссылку на уникальный идентификатор
	const file_id &id() const { return m_id; }
	/// Возвращает ссылку на поток
	std::istream &stream() const { return *m_stream; }
};

}

}

#endif // !defined(__FILE_H_INCLUDED_8093570542662288__)