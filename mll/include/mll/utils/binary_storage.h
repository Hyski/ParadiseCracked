#if !defined(__BINARY_STORAGE_INCLUDED__)
#define __BINARY_STORAGE_INCLUDED__

#include <algorithm>
#include <istream>
#include <ostream>
#include <list>

#include <mll/utils/lcrn_tree.h>
#include <mll/io/binstream.h>

namespace mll
{

namespace utils
{

template<typename ID>
class _binary_storage_path;

template<typename ID>
class _binary_storage_node;

class _binary_storage_version;
class _binary_storage_node_data;


template<typename ID>
struct _binary_storage_traits
{
	typedef ID id;
	typedef _binary_storage_path<id> path;
	typedef _binary_storage_version version;
	typedef typename lcrn_tree<_binary_storage_node<id> >::iterator iterator;

protected:

	typedef _binary_storage_node_data node_data_t;
	typedef _binary_storage_node<id> node;
	typedef lcrn_tree<node> bs_tree_t;

};

//=====================================================================================//
//                                class binary_storage                                 //
//=====================================================================================//
template<typename ID>
class binary_storage : public _binary_storage_traits<ID>
{
private:

	bs_tree_t m_bsTree;							//	дерево				
	version m_version;							//	версия

public:
	explicit binary_storage(mll::io::ibinstream& istream);
	binary_storage(const version& v);
	~binary_storage();
public:
	///	прочитать из потока
	void load(mll::io::ibinstream& istream);
	///	записать в поток
	void save(mll::io::obinstream& ostream);

public:
	///	создать корневой узел
	iterator create_root(const id& _id);
	///	получить узел по пути
	iterator get_node(const path& p);
	///	получить итератор на корень
	iterator begin(void);
	///	получить итератор за последним элементом
	iterator end(void);

public:
	///	удалить узел
	void erase(iterator it);
	///	очистить дерево
	void clear(void);

public:
	///	получить версию 
	const version& get_version(void) const;
	///	получить версию 
	version& get_version(void);

};

//=====================================================================================//
//                            class _binary_storage_version                            //
//=====================================================================================//
class _binary_storage_version
{
private:
	unsigned int m_major;					//	различие мажоров означает несовместимость версий
	unsigned int m_minor;					//	фиксирует инкрементальные изменения (версии совместимы снизу вверх)
public:
	_binary_storage_version() : m_major(0),m_minor(0) {}
	_binary_storage_version(mll::io::ibinstream& istream) : m_major(0),m_minor(0) { load(istream); }
	_binary_storage_version(const _binary_storage_version& v) : m_major(v.m_major),m_minor(v.m_minor) {}
	_binary_storage_version(unsigned int major,unsigned int minor) : m_major(major),m_minor(minor) {}
public:
	///	оператор копирования
	_binary_storage_version& operator=(const _binary_storage_version& v)
	{
		m_major = v.m_major;
		m_minor = v.m_minor;

		return *this;
	}
	///	сравнивть на совместимость с другой версией
	bool is_compatible(const _binary_storage_version& v) const
	{
		return (m_major==v.m_major) && (m_minor>=v.m_minor);
	}
	///	сохранить в поток
	void save(mll::io::obinstream& ostream) const 
	{
		ostream << m_major << m_minor;
	}
	///	загрузить из потока
	void load(mll::io::ibinstream& istream)
	{
		istream >> m_major >> m_minor;
	}
	///	получить мажор версии
	unsigned int major(void) const
	{
		return m_major;
	}
	///	получить минор версии
	unsigned int minor(void) const
	{
		return m_minor;
	}

};

//=====================================================================================//
//                              inline bool operator==()                               //
//=====================================================================================//
inline bool operator==(const _binary_storage_version& a,const _binary_storage_version& b)
{
	return (a.major() == b.major()) && (a.minor() == b.minor());
}

//=====================================================================================//
//                               inline bool operator<()                               //
//=====================================================================================//
inline bool operator<(const _binary_storage_version& a,const _binary_storage_version& b)
{
	if(a.major() < b.major()) return true;
	if(a.major() > b.major()) return false;

	return a.minor() < b.minor();
}

//=====================================================================================//
//                            class _binary_storage_node;                              //
//=====================================================================================//
template<typename ID>
class _binary_storage_node : public _binary_storage_traits<ID>
{
private:

	bs_tree_t& m_bsTree;
	node_data_t m_data;
	iterator m_self;
	id m_id;

public:
	_binary_storage_node(const id& _id,bs_tree_t& bs_tree) : m_id(_id),
														     m_bsTree(bs_tree)
														     {}
	_binary_storage_node(const _binary_storage_node& n) : m_id(n.m_id),
														  m_self(n.m_self),
														  m_bsTree(n.m_bsTree)
														  {}
	~_binary_storage_node() {}
public:
	//	оператор копирования
	_binary_storage_node& operator=(const _binary_storage_node& n)
	{
		if(&n != this)
		{
			m_id = n.m_id;
			m_self = n.m_self;
			m_bsTree = n.m_bsTree;
		}

		return *this
	}
	//	создать дочерний узел (возвращает итератор на созданный узел)
	iterator create_node(const id& _id)
	{
		iterator it = m_bsTree.insert_son(m_self,node(_id,m_bsTree));
		return it->m_self = it;
	}
	//	сохранить дерево, начиная с данного узла
	void save(mll::io::obinstream& out)
	{
		unsigned int counter = 0;
		iterator it;
		//	подсчитываем кол-во детей
		for(it=first_child();it!=end();it.brother()) counter++;
		//	сохраняем идентификатор узла
		out << m_id;
		//	сохраняем данные
		m_data.save(out);
		//	сохраняем кол-во детей
		out << counter;
		//	сохраняем детей
		for(it=first_child();it!=end();it.brother())
			it->save(out);
	}
	//	прочитать дерево, начиная с данного узла
	void load(mll::io::ibinstream& in)
	{
		unsigned int counter = 0;
		//	считываем данные узла
		m_data.load(in);
		//	считываем кол-во детей
		in >> counter;
		//	создаем дочерние узлы
		for(;counter>0;counter--)
		{
			//	считываем идентификатор узла
			id _id;
			in >> _id;
			//	добавляем и считываем узел
			create_node(_id)->load(in);
		}
	}
	//	получить идентификатор узла
	const id& get_id() const
	{
		return m_id;
	}
	//	получить данные из узла
	const node_data_t& get_data() const
	{
		return m_data;
	}
	//	получить данные из узла
	node_data_t& get_data(void)
	{
		return m_data;
	}
	//	получить итератор на первого ребенка
	iterator first_child(void)
	{
		return m_self.get_son();
	}
	//	получить итератор за последним ребенка
	iterator end(void)
	{
		return iterator();
	}


	friend binary_storage<ID>;
};

class _binary_storage_node_data
{
private:
	char* m_data;
	unsigned int m_size;
public:
	_binary_storage_node_data() : m_data(0),m_size(0) {}
	_binary_storage_node_data(const _binary_storage_node_data& nd) : m_data(0),m_size(0)
	{
		this->operator=(nd);
	}
	~_binary_storage_node_data() {	delete[] m_data; }
public:
	//	оператор копирования
	_binary_storage_node_data& operator=(const _binary_storage_node_data& nd)
	{
		if(&nd != this) set_data(nd.m_data,nd.m_size);
		return *this;
	}

	//	занести данные
	const void* set_data(const void* data,unsigned int size)
	{
		delete[] m_data; m_data = 0; m_size = 0;

		if(data && size)
		{
			m_size = size;
			m_data = new char[m_size];
			memcpy(m_data,data,m_size);
		}

		return m_data;
	}
	//	получить данные
	const void* get_data(void) const { return m_data; }
	//	получить размер данных
	unsigned int size(void) const { return m_size; }
	//	сохранить данные
	void save(mll::io::obinstream& out)
	{
		//	сохраняем размер данных узла
		out << m_size;
		//	сохраняем данные хранящиеся в узле
		out.stream().write(m_data,m_size);
	}
	//	считываем данные
	void load(mll::io::ibinstream& in)
	{
		delete[] m_data; m_data = 0; m_size = 0;
		//	сохраняем размер данных узла
		in >> m_size;
		if(m_size)
		{
			m_data = new char[m_size];
			//	сохраняем данные хранящиеся в узле
			in.stream().read(m_data,m_size);
		}
	}
};

//=====================================================================================//
//                                     class _path                                     //
//=====================================================================================//
template<typename ID>
class _binary_storage_path : public _binary_storage_traits<ID>
{
private:

	typedef std::list<id> path_list_t;
	path_list_t m_path;

public:
	_binary_storage_path() {}
	_binary_storage_path(const _binary_storage_path& p) : m_path(p.m_path) {}
	_binary_storage_path(const id& _id) 
	{ 
		m_path.push_back(_id); 
	}
public:
	//	оператор копирования
	_binary_storage_path& operator=(const _binary_storage_path& p)
	{ 
		m_path = p.m_path;
		return *this;
	}
	//	добавить кусочек пути до узла
	_binary_storage_path& append(const id& _id) 
	{ 
		m_path.push_back(_id); 
		return *this; 
	}
	//	оператор сложения двух путей
	_binary_storage_path& operator+=(const _binary_storage_path& p) 
	{ 
		std::copy(p.m_path.begin(),p.m_path.end(),std::back_inserter(m_path));
		return *this; 
	}
	//	оператор сравнения
	bool operator==(const _binary_storage_path& p)
	{ 
		return m_path == p.m_path;
	}
	//	оператор нервенства
	bool operator!=(const _binary_storage_path& p)
	{ 
		return m_path != p.m_path;
	}

	friend binary_storage<ID>;
};

//=====================================================================================//
//                     inline _binary_storage_path<ID> operator+()                     //
//=====================================================================================//
template<typename ID>	//	оператор сложения двух путей
inline _binary_storage_path<ID> operator+(const _binary_storage_path<ID>& a,const _binary_storage_path<ID>& b)
{
	_binary_storage_path<ID> res(a);
	return res += b;
}

//=====================================================================================//
//                                class binary_storage                                 //
//=====================================================================================//
template<typename ID>
inline binary_storage<ID>::binary_storage(mll::io::ibinstream& istream)
{
	load(istream);
}

template<typename ID>
inline binary_storage<ID>::binary_storage(const version& v) : m_version(v)
{
}

template<typename ID>
inline binary_storage<ID>::~binary_storage()
{
}

template<typename ID>
inline void binary_storage<ID>::load(mll::io::ibinstream& in)
{
	//	очищаем дерево
	m_bsTree.clear();
	//	считываем версию
	m_version.load(in);
	//	считываем флаг состояния (наличия дерева)
	unsigned int state_flag = 0;
	in >> state_flag;
	if(state_flag)
	{//	создаем корневой узел
		//	считываем идентификатор узла
		id _id;
		in >> _id;
		//	создаем узел и считываем дерево
		create_root(_id)->load(in);
	}
} 

template<typename ID>
inline void binary_storage<ID>::save(mll::io::obinstream& out)
{
	//	сохраняем версию
	m_version.save(out);
	//	сохраняем флаг наличия дерева (дерево может отсутствовать - мы должны это знать) 
	unsigned int state_flag = m_bsTree.begin()==m_bsTree.end()?0:1;
	out << state_flag;
	if(state_flag)
	{//	сохраняем дерево ( <узел><кол-во дочерних узлов><дочерние узлы> )
		m_bsTree.begin()->save(out);
	}
}

template<typename ID>
inline typename binary_storage<ID>::iterator binary_storage<ID>::create_root(const id& _id)
{
	if(m_bsTree.size()) return m_bsTree.begin();
	iterator it = m_bsTree.insert_son(m_bsTree.end(),node(_id,m_bsTree)); 
	return it->m_self = it;
}

template<typename ID>
inline typename binary_storage<ID>::iterator binary_storage<ID>::get_node(const path& p)
{
	if(!p.m_path.size() || m_bsTree.empty()) return m_bsTree.end();

	iterator itt = m_bsTree.begin();
	path::path_list_t::const_iterator itl = p.m_path.begin();

	if(!(itt->get_id() == *itl)) return m_bsTree.end();

	for(++itl;itl!=p.m_path.end();++itl)
	{
		for(itt.son();itt!=m_bsTree.end();itt.brother())
		{
			if(itt->get_id() == *itl) break;
		}
		if(itt==m_bsTree.end()) return m_bsTree.end();
	}

	return itt;
}
//	получить итератор на корень
template<typename ID>
inline typename binary_storage<ID>::iterator binary_storage<ID>::begin(void)
{
	return m_bsTree.begin();
}
//	получить итератор за последним элементом
template<typename ID>
inline typename binary_storage<ID>::iterator binary_storage<ID>::end(void)
{
	return m_bsTree.end();
}
//	удалить узел
template<typename ID>
inline void binary_storage<ID>::erase(iterator it)
{
	m_bsTree.erase(it);
}
//	очистить дерево
template<typename ID>
inline void binary_storage<ID>::clear(void)
{
	return m_bsTree.clear();
}
//	получить версию 
template<typename ID>
inline const typename binary_storage<ID>::version& binary_storage<ID>::get_version(void) const
{
	return m_version;
}
//	получить версию 
template<typename ID>
inline typename binary_storage<ID>::version& binary_storage<ID>::get_version(void)
{
	return m_version;
}


}	//	namespace utils

}	//	namespace mll

#endif