#if !defined(__TABLE_H_INCLUDED_8170697998161808__)
#define __TABLE_H_INCLUDED_8170697998161808__

#include <string>
#include <vector>
#include <cassert>
#include <iterator>
#include <algorithm>
#include <functional>
#include <mll/io/binstream.h>
#include <mll/algebra/point2.h>
#include <mll/debug/exception.h>

namespace mll
{

namespace utils
{

MLL_MAKE_EXCEPTION(table_read_error,mll::debug::exception);
MLL_MAKE_EXCEPTION(wrong_format,table_read_error);
MLL_MAKE_EXCEPTION(wrong_byte_order,table_read_error);

template<typename T> class basic_row;
template<typename T> class basic_col;
template<typename T> class basic_table;

//=====================================================================================//
//                                  namespace detail                                   //
//=====================================================================================//
namespace detail
{

template<typename T> class basic_row_iterator;
template<typename T> class basic_col_iterator;
template<typename T> class basic_cell_iterator;

template<typename T>
void read_table(mll::io::ibinstream &, basic_table<T> &);

//=====================================================================================//
//                      template<typename T> struct table_traits                       //
//=====================================================================================//
template<typename T> struct table_traits
{
	typedef basic_row<T> row_type;
	typedef basic_col<T> col_type;

	typedef basic_row_iterator<T> row_iterator;
	typedef basic_col_iterator<T> col_iterator;
	typedef basic_cell_iterator<T> cell_iterator;

	typedef std::vector<T> _M_row_type;
	typedef std::vector<_M_row_type> _M_table_type;

	typedef std::vector<row_type> _M_rows_type;
	typedef std::vector<col_type> _M_cols_type;
};

//=====================================================================================//
//                                 class row_iterator                                  //
//=====================================================================================//
template<typename T>
class basic_row_iterator : public std::iterator< std::random_access_iterator_tag, basic_row<T> >
{
	friend class basic_table<T>;
	typedef basic_row<T> row_type;
	typedef typename table_traits<T>::_M_rows_type::const_iterator iterator_type;

	typedef basic_row_iterator self;

public:
	typedef row_type value_type;
	typedef const row_type &reference_type;

	typedef basic_cell_iterator<T> iterator;

public:
	basic_row_iterator() {}
	basic_row_iterator(const basic_row_iterator &i) : m_it(i.m_it) {}

	bool operator==(const self &it) const { return m_it == it.m_it; }
	bool operator!=(const self &it) const { return m_it != it.m_it; }
	bool operator< (const self &it) const { return m_it  < it.m_it; }
	bool operator> (const self &it) const { return m_it  > it.m_it; }
	bool operator<=(const self &it) const { return m_it <= it.m_it; }
	bool operator>=(const self &it) const { return m_it >= it.m_it; }

	self &operator+=(int n) { m_it += n; return *this; }
	friend self operator+(const self &it, int n) { return it.m_it + n; }
	friend self operator+(int n, const self &it) { return it.m_it + n; }

	self &operator-=(int n) { m_it -= n; return *this; }
	friend self operator-(const self &it, int n) { return it.m_it - n; }
	friend self operator-(int n, const self &it) { return it.m_it - n; }

	friend int operator-(const self &a, const self &b) { return a.m_it - b.m_it; }

	self &operator++() { ++m_it; return *this; }
	self operator++(int) { self a(*this); ++a; return *this; }
	self &operator--() { --m_it; return *this; }
	self operator--(int) { self a(*this); --a; return *this; }

	reference_type operator*() const { return *m_it; }
	const value_type *operator->() const { return &*m_it; }

	reference_type operator[](int i) const { return *(*this + i); }

private:
	basic_row_iterator(iterator_type i) : m_it(i) {}

private:
	iterator_type m_it;
};

//=====================================================================================//
//                              class basic_col_iterator                               //
//=====================================================================================//
template<typename T>
class basic_col_iterator : public std::iterator< std::random_access_iterator_tag, basic_col<T> >
{
	friend class basic_table<T>;
	typedef basic_col<T> col_type;
	typedef typename table_traits<T>::_M_cols_type::const_iterator iterator_type;

	typedef basic_col_iterator self;

public:
	typedef col_type value_type;
	typedef const col_type &reference_type;

	typedef basic_cell_iterator<T> iterator;

public:
	basic_col_iterator() {}
	basic_col_iterator(const self &i) : m_it(i.m_it) {}

	bool operator==(const self &it) const { return m_it == it.m_it; }
	bool operator!=(const self &it) const { return m_it != it.m_it; }
	bool operator< (const self &it) const { return m_it  < it.m_it; }
	bool operator> (const self &it) const { return m_it  > it.m_it; }
	bool operator<=(const self &it) const { return m_it <= it.m_it; }
	bool operator>=(const self &it) const { return m_it >= it.m_it; }

	self &operator+=(int n) { m_it += n; return *this; }
	friend self operator+(const self &it, int n) { return it.m_it + n; }
	friend self operator+(int n, const self &it) { return it.m_it + n; }

	self &operator-=(int n) { m_it -= n; return *this; }
	friend self operator-(const self &it, int n) { return it.m_it - n; }
	friend self operator-(int n, const self &it) { return it.m_it - n; }

	friend int operator-(const self &a, const self &b) { return a.m_it - b.m_it; }

	self &operator++() { ++m_it; return *this; }
	self operator++(int) { self a(*this); ++a; return *this; }
	self &operator--() { --m_it; return *this; }
	self operator--(int) { self a(*this); --a; return *this; }

	reference_type operator*() const { return *m_it; }
	const value_type *operator->() const { return &*m_it; }

	reference_type operator[](int i) const { return *(*this + i); }

private:
	basic_col_iterator(iterator_type i) : m_it(i) {}

private:
	iterator_type m_it;
};

//=====================================================================================//
//                              class basic_cell_iterator                              //
//=====================================================================================//
template<typename T>
class basic_cell_iterator : public std::iterator< std::random_access_iterator_tag, T >
{
	friend class basic_row<T>;
	friend class basic_col<T>;
	friend class basic_table<T>;

	typedef typename table_traits<T>::_M_table_type table_type;
	typedef typename table_type::const_iterator iterator_type;
	typedef basic_cell_iterator self;

public:
	typedef T value_type;
	typedef const T &reference_type;

public:
	enum mode
	{
		row, col
	};

public:
	basic_cell_iterator(mode b = row) : m_index(0), m_mode(b) {}
	basic_cell_iterator(const self &i) : m_it(i.m_it), m_index(i.m_index), m_mode(i.m_mode) {}

	bool operator==(const self &it) const { return m_it == it.m_it && m_index == it.m_index; }
	bool operator!=(const self &it) const { return !(*this == it); }
	bool operator< (const self &it) const { return compare(it); }
	bool operator> (const self &it) const { return it.compare(*this); }
	bool operator<=(const self &it) const { return *this == it || *this < it; }
	bool operator>=(const self &it) const { return *this == it || *this > it; }

	self &operator+=(int n)
	{
		switch(m_mode)
		{
			case row: m_index += n; break;
			case col: m_it += n; break;
		}
		return *this;
	}


	self &operator-=(int n) { m_it += -n; return *this; }

	self &operator++() { *this += 1; return *this; }
	self operator++(int) { self a(*this); ++a; return *this; }
	self &operator--() { *this -= 1; return *this; }
	self operator--(int) { self a(*this); --a; return *this; }

	reference_type operator*() const { return *(m_it->begin()+m_index); }
	const value_type *operator->() const { return &*(m_it->begin()+m_index); }

	reference_type operator[](int i) const { return *(*this + i); }

	int subtract(const self &b) const
	{
		assert(m_mode == b.m_mode);

		switch(m_mode)
		{
			case row: return m_index - b.m_index;
			case col: return m_it - b.m_it;
		}
		return 0;
	}

private:
	basic_cell_iterator(iterator_type i, unsigned int idx, mode b)
		: m_it(i), m_index(idx), m_mode(b) {}

	bool compare(const self &s) const
	{
		if(m_it == s.m_it) return m_index < s.m_index;
		return m_it < s.m_it;
	}

	mode m_mode;
	iterator_type m_it;
	unsigned int m_index;
};

template<typename T>
basic_cell_iterator<T> operator+(const basic_cell_iterator<T> &it, int n)
{
	basic_cell_iterator<T> copy(it); return copy += n;
}

template<typename T>
basic_cell_iterator<T> operator+(int n, const basic_cell_iterator<T> &it)
{
	return it+n;
}

template<typename T>
basic_cell_iterator<T> operator-(const basic_cell_iterator<T> &it, int n)
{
	return it + -n;
}

template<typename T>
basic_cell_iterator<T> operator-(int n, const basic_cell_iterator<T> &it)
{
	return it + -n;
}

template<typename T>
int operator-(const basic_cell_iterator<T> &a, const basic_cell_iterator<T> &b)
{
	return a.subtract(b);
}

}

//=====================================================================================//
//                                   class basic_row                                   //
//=====================================================================================//
/// ѕредставл€ет собой строку таблицы
template<typename T>
class basic_row
{
	friend class basic_table<T>;
	friend void detail::read_table<T>(mll::io::ibinstream &, basic_table<T> &);

	struct tester { void truth() const {} };
	typedef void (tester::*unspecified_bool_type)() const;

public:
	/// итератор по €чейкам строки
	typedef detail::basic_cell_iterator<T> iterator;

public:
	basic_row() : m_table(0), m_index(0xFFFFFFFF) {}

	/// возвращает итератор на первую €чейку строки
	iterator begin() const { return iterator(m_table->m_data.begin()+m_index,0,iterator::row); }
	/// возвращает итератор за последней €чейкой строки
	iterator end() const { return iterator(m_table->m_data.begin()+m_index,m_table->m_width,iterator::row); }

	/// возвращает индекс строки
	unsigned int index() const { return m_index; }

	/// возвращает €чейку строки с заданным индексом
	const T &operator[](int i) const { return *(begin()+i); }

	/// ¬озвращает true, если столбец не существует в таблице
	operator unspecified_bool_type() const
	{
		return m_table ? &tester::truth : 0;
	}

	basic_row &operator=(const basic_row &r)
	{
		if(this != &r)
		{
			m_table = r.m_table;
			m_index = r.m_index;
		}
		return *this;
	}

private:
	basic_row(const basic_table<T> *tbl, unsigned int i) : m_table(tbl), m_index(i) {}
	void rebase(basic_table<T> *t) { m_table = t; }

	const basic_table<T> *m_table;
	unsigned int m_index;
};

//=====================================================================================//
//                                   class basic_col                                   //
//=====================================================================================//
/// ѕредставл€ет собой столбец таблицы
template<typename T>
class basic_col
{
	friend class basic_table<T>;
	friend void detail::read_table<T>(mll::io::ibinstream &, basic_table<T> &);

	struct tester { void truth() const {} };
	typedef void (tester::*unspecified_bool_type)() const;

public:
	/// итератор по €чейкам столбца
	typedef detail::basic_cell_iterator<T> iterator;

public:
	basic_col() : m_table(0), m_index(0xFFFFFFFF) {}

	/// возвращает итератор на первую €чейку столбца
	iterator begin() const { return iterator(m_table->m_data.begin(),m_index,iterator::col); }
	/// возвращает итератор за последней €чейкой столбца
	iterator end() const { return iterator(m_table->m_data.end(),m_index,iterator::col); }

	/// возвращает индекс столбца
	unsigned int index() const { return m_index; }

	/// возвращает €чейку столбца с заданным индексом
	const T &operator[](int i) const { return *(begin()+i); }

	/// ¬озвращает true, если столбец не существует в таблице
	operator unspecified_bool_type() const
	{
		return m_table ? &tester::truth : 0;
	}

	basic_col &operator=(const basic_col &r)
	{
		if(this != &r)
		{
			m_table = r.m_table;
			m_index = r.m_index;
		}
		return *this;
	}

private:
	basic_col(const basic_table<T> *tbl, unsigned int i) : m_table(tbl), m_index(i) {}
	void rebase(basic_table<T> *) {}

	const basic_table<T> *m_table;
	unsigned int m_index;
};

//=====================================================================================//
//                                  class basic_table                                  //
//=====================================================================================//
/// ’ранит страницу строк
template<typename T>
class basic_table
{
	friend class basic_row<T>;
	friend class basic_col<T>;

	typedef typename detail::table_traits<T>::_M_row_type internal_row_type;
	typedef typename detail::table_traits<T>::_M_table_type table_type;
	typedef typename detail::table_traits<T>::_M_cols_type cols_type;
	typedef typename detail::table_traits<T>::_M_rows_type rows_type;

	table_type m_data;
	cols_type m_cols;
	rows_type m_rows;
	size_t m_width, m_height;

	basic_col<T> m_emptyCol;
	basic_row<T> m_emptyRow;

public:
	/// тип хранимых данных
	typedef T value_type;
	/// тип строки
	typedef basic_row<T> row_type;
	/// тип столбца
	typedef basic_col<T> col_type;
	/// итератор по строкам
	typedef typename detail::table_traits<T>::row_iterator row_iterator;
	/// итератор по столбцам
	typedef typename detail::table_traits<T>::col_iterator col_iterator;
	/// итератор по €чейкам
	typedef typename detail::table_traits<T>::cell_iterator iterator;

public:
	basic_table() : m_width(0), m_height(0) {}
	basic_table(const basic_table &t) : m_data(t.m_data), m_width(t.m_width), m_height(t.m_height),
		m_cols(t.m_cols), m_rows(t.m_rows)
	{
		for(unsigned int i = 0; i < m_width; ++i) m_cols[i].rebase(this);
		for(unsigned int i = 0; i < m_height; ++i) m_rows[i].rebase(this);
	}

	const row_type &operator[](int i) const { return m_rows[i]; }
	const T &operator[](const mll::algebra::point2i &i) const { return (*this)[i.y][i.x]; }

	/// очистить таблицу
	void clear()
	{
		*this = basic_table();
	}

	/// возвращает ширину таблицы (количество столбцов)
	size_t width() const { return m_width; }
	/// возвращает высоту таблицы (количество строк)
	size_t height() const { return m_height; }

	basic_table &operator=(const basic_table &t)
	{
		if(&t != this)
		{
			m_data = t.m_data;
			m_cols = t.m_cols;
			m_rows = t.m_rows;
			m_width = t.m_width;
			m_height = t.m_height;
		}
		return *this;
	}

	row_iterator row_begin() const { return row_iterator(m_rows.begin()); }
	row_iterator row_end() const { return row_iterator(m_rows.end()); }

	col_iterator col_begin() const { return col_iterator(m_cols.begin()); }
	col_iterator col_end() const { return col_iterator(m_cols.end()); }

	const row_type &row(int i) const { return m_rows[i]; }
	const row_type &row(const iterator &i) const { return i.m_it != m_data.end() ? m_rows[i.m_it-m_data.begin()] : m_emptyRow; }
	const col_type &col(int i) const { return m_cols[i]; }
	const col_type &col(const iterator &i) const { return i.m_index < m_cols.size() ? m_cols[i.m_index] : m_emptyCol; }

	friend void detail::read_table<T>(mll::io::ibinstream &, basic_table<T> &);
};

typedef basic_table<std::string> table;
typedef basic_table<std::wstring> wtable;

} // namespace 'utils'

} // namespace 'mll'

#include "table_details.h"

#endif // !defined(__TABLE_H_INCLUDED_8170697998161808__)