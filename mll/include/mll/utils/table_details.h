#pragma once

namespace mll
{

namespace utils
{

//=====================================================================================//
//                                  namespace detail                                   //
//=====================================================================================//
namespace detail
{

enum end_of
{
	eoRow,
	eoCell,
};

template<typename T>
struct token
{
	T m_text;
	end_of m_endof;

	token() : m_endof(eoRow) {}
};

template<typename T> struct lexer_traits;

template<> struct lexer_traits<std::string>
{
	static const char quote = '\"';
	static const char cr = '\r';
	static const char lf = '\n';
	static const char tab = '\t';
};

template<> struct lexer_traits<std::wstring>
{
	static const wchar_t quote = L'\"';
	static const wchar_t cr = L'\r';
	static const wchar_t lf = L'\n';
	static const wchar_t tab = L'\t';
};

//=====================================================================================//
//                                 class lexer_helper                                  //
//=====================================================================================//
template<typename T>
class lexer_helper
{
	enum { buffer_shift = 5 };
	enum { buffer_size = 1<<5, buffer_mask = buffer_size-1 };

	mll::io::ibinstream &m_file;

	typename T::traits_type::int_type m_buffer[buffer_size];
	unsigned int m_i;
	unsigned int m_c;

	void raw_consume(unsigned int c = 1)
	{
		assert( m_c >= c );
		m_i += c;
		m_c -= c;
	}

	void precache(unsigned int i = 1)
	{
		for(; i; --i)
		{
			const unsigned int index = (m_i+m_c++)&buffer_mask;
			assert( m_c <= buffer_size );
			typename T::traits_type::char_type c;
			m_file >> c;
			m_buffer[index] = (m_file.good())?T::traits_type::to_int_type(c):T::traits_type::eof();
		}
	}

public:
	lexer_helper(mll::io::ibinstream &file) : m_file(file), m_i(0), m_c(0) {prepare();}

	void prepare();

	typename T::traits_type::int_type lai(unsigned int i = 0)
	{
		if(m_c <= i) precache(i-m_c + 1);
		return m_buffer[(m_i+i)&buffer_mask];
	}

	typename T::traits_type::char_type lac(unsigned int i = 0)
	{
		if(m_c <= i) precache(i-m_c + 1);
		return T::traits_type::to_char_type(m_buffer[(m_i+i)&buffer_mask]);
	}

	void consume(unsigned int c = 1)
	{
		if(c > m_c)
		{
			precache(c-m_c);
		}

		raw_consume(c);
	}


};

//=====================================================================================//
//                      void lexer_helper<std::string>::prepare()                      //
//=====================================================================================//
template<>
inline void lexer_helper<std::string>::prepare()
{
}

//=====================================================================================//
//                     void lexer_helper<std::wstring>::prepare()                      //
//=====================================================================================//
template<>
inline void lexer_helper<std::wstring>::prepare()
{
	unsigned short byte_order;

	m_file >> byte_order;

	if(byte_order != 0xFEFF) throw wrong_byte_order("wrong byte order in a txt file, "
													"possibly, it is not a unicode file");

}

//=====================================================================================//
//                             token _read_simple_token()                              //
//=====================================================================================//
template<typename T>
token<T> _read_simple_token(lexer_helper<T> &file)
{
	token<T> result;
	while(file.lac() != lexer_traits<T>::tab
		&& file.lac() != lexer_traits<T>::cr
		&& file.lac() != lexer_traits<T>::lf)
	{
		if( file.lai() == T::traits_type::eof() ) throw wrong_format("wrong format of txt: unexpected eof");
		result.m_text += file.lac();
		file.consume();
	}
	return result;
}

//=====================================================================================//
//                             token _read_complex_token()                             //
//=====================================================================================//
template<typename T>
token<T> _read_complex_token(lexer_helper<T> &file)
{
	token<T> result;
	file.consume(); // пропустим открывающие кавычки

	while(file.lac() != lexer_traits<T>::quote || file.lac(1) == lexer_traits<T>::quote)
	{
		if(file.lai() == T::traits_type::eof()) throw wrong_format("wrong format of txt: unexpected eof");

		if(file.lac() == lexer_traits<T>::quote)
		{
			file.consume();
			result.m_text += lexer_traits<T>::quote;
		}
		else
		{
			result.m_text += file.lac();
		}

		file.consume();
	}

	file.consume();
	return result;
}

//=====================================================================================//
//                                 token _read_token()                                 //
//=====================================================================================//
template<typename T>
token<T> _read_token(lexer_helper<T> &file)
{
	token<T> result;
	if(file.lai() == T::traits_type::eof()) return result;

	switch(file.lac())
	{
		case lexer_traits<T>::quote:
			result = _read_complex_token(file);
			break;
		default:
			result = _read_simple_token(file);
			break;
	}

	switch(file.lac())
	{
		case lexer_traits<T>::tab:
			result.m_endof = eoCell;
			file.consume();
			break;
		case lexer_traits<T>::cr:
			if(file.lac(1) != lexer_traits<T>::lf) throw wrong_format("wrong format of txt: no \\n after \\r");
			file.consume(2);
			result.m_endof = eoRow;
			break;
		case lexer_traits<T>::lf:
			file.consume(1);
			result.m_endof = eoRow;
			break;
		default:
			throw wrong_format("wrong format of txt: unexpected symbol at end of cell");
	}

	return result;
}

template<typename R>
inline void read_table(mll::io::ibinstream &in, basic_table<R> &tbl)
{
	in.stream() >> std::noskipws;
	tbl.clear();
	unsigned int width = 0;

	lexer_helper<R> lexhlp(in);
	bool end_of_row = true;
	while(lexhlp.lai() != R::traits_type::eof())
	{
		token<R> tok = _read_token(lexhlp);
		if(!in.eof())
		{
			if(end_of_row)
			{
				tbl.m_data.push_back(basic_table<R>::internal_row_type());
				end_of_row = false;
			}
			tbl.m_data.back().push_back(tok.m_text);
			if(tok.m_endof == eoRow) end_of_row = true;
			width = std::max(tbl.m_data.back().size(),width);
		}
	}

	tbl.m_width = width;
	tbl.m_height = tbl.m_data.size();

	tbl.m_rows.reserve(tbl.m_height);
	for(unsigned int i = 0; i != tbl.m_height; ++i)
	{
		tbl.m_data[i].resize(tbl.m_width);
		tbl.m_rows.push_back(basic_row<R>(&tbl,i));
	}
	tbl.m_cols.reserve(tbl.m_width);
	for(unsigned int i = 0; i != tbl.m_width; ++i)
	{
		tbl.m_cols.push_back(basic_col<R>(&tbl,i));
	}
}

}

//=====================================================================================//
//                          mll::io::ibinstream &operator>>()                          //
//=====================================================================================//
template<typename R>
inline mll::io::ibinstream &operator>>(mll::io::ibinstream &in, basic_table<R> &tbl)
{
	detail::read_table(in,tbl);
	return in;
}

//=====================================================================================//
//                             std::istream &operator>>()                              //
//=====================================================================================//
template<typename T>
inline std::istream &operator>>(std::istream &file, basic_table<T> &tbl)
{
	mll::io::ibinstream in(file);
	detail::read_table(in,tbl);
	return file;
}

}

}
