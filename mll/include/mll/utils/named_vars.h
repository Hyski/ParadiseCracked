#if !defined(__NAMEDVARS46216874_INCLUDED__)
#define __NAMEDVARS46216874_INCLUDED__

#include <mll/algebra/vector3.h>
#include <mll/algebra/point3.h>
#include <mll/algebra/point2.h>
#include <mll/algebra/vector2.h>
#include <mll/io/binstream.h>
#include <mll/debug/exception.h>
#include <list>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace mll
{
namespace utils
{

/*=====================================================================================*\
 *                                      class var                                      * 
\*=====================================================================================*/
class var
{
	std::string m_name;
	std::string m_value;

public:
	var() {}
	explicit var(const std::string &name) : m_name(name) {}
	explicit var(const char *name) : m_name(name) {}
	var(const std::string &name, const std::string &val) : m_name(name), m_value(val) {}
	var(const char *name, const char *val) : m_name(name), m_value(val) {}

	var &operator =(const std::string &);
	var &operator =(const char *);
	var &operator =(const algebra::point3 &);
	var &operator =(const algebra::point2 &);
	var &operator =(const algebra::vector3 &);
	var &operator =(const algebra::vector2 &);
	var &operator =(float);
	var &operator =(int);

	const char *get_string() const { return m_value.c_str(); }

	algebra::point3 get_point3() const;
	algebra::point2 get_point2() const;
	algebra::vector3 get_vector3() const;
	algebra::vector2 get_vector2() const;
	float get_float() const;
	int get_int() const;

//	friend bool operator<(const var &v1, const var &v2);
	friend bool operator==(const var &v1, const var &v2);
	friend bool operator!=(const var &v1, const var &v2);

	const std::string &name() const { return m_name; }
	const std::string &value() const { return m_value; }
};

//=====================================================================================//
//                                     struct less                                     //
//=====================================================================================//
struct less
{
	bool operator()(const var &lhs, const var &rhs) const { return lhs.name() < rhs.name(); }
	bool operator()(const var &lhs, const std::string &rhs) const { return lhs.name() < rhs; }
	bool operator()(const std::string &lhs, const var &rhs) const { return lhs < rhs.name(); }
};

/*=====================================================================================*\
 *                                  class named_vars                                   * 
 *  Блок именованных переменных                                                        * 
\*=====================================================================================*/
class named_vars
{
	typedef std::list<var> vars_container;

public:
	typedef vars_container::iterator iterator;
	typedef vars_container::const_iterator const_iterator;

public:
	const_iterator begin() const { return m_vars.begin(); }
	const_iterator end() const { return m_vars.end(); }

	iterator begin() { return m_vars.begin(); }
	iterator end() { return m_vars.end(); }

//	void save(std::ostream &o)const {};
//	void load(std::ostream &i)      {};

	bool exists(const std::string &name) const;
	bool erase(const std::string &name);
	void clear() { m_vars.clear(); }

	var& operator[](const std::string &name);
	const var &operator[](const std::string &name) const;

	unsigned size() const { return m_vars.size(); }

	const char *get_default(const std::string &, const char *) const;
	std::string get_default(const std::string &, const std::string &) const;
	algebra::point3 get_default(const std::string &, const algebra::point3 &) const;
	algebra::point2 get_default(const std::string &, const algebra::point2 &) const;
	algebra::vector3 get_default(const std::string &, const algebra::vector3 &) const;
	algebra::vector2 get_default(const std::string &, const algebra::vector2 &) const;
	float get_default(const std::string &, float) const;
	int get_default(const std::string &, int) const;

	bool operator==(const named_vars &rhs) const { return std::equal(begin(),end(),rhs.begin()); }

private:
	vars_container m_vars;
};

typedef debug::exception_template<struct var_not_found_tag> var_not_found;

//=====================================================================================//
//                               Операторы ввода-вывода                                //
//=====================================================================================//
MLL_EXPORT std::ostream &operator<<(std::ostream &, const named_vars &);
MLL_EXPORT std::istream &operator>>(std::istream &, named_vars &);
MLL_EXPORT io::obinstream &operator<<(io::obinstream &, const named_vars &);
MLL_EXPORT io::ibinstream &operator>>(io::ibinstream &, named_vars &);

//=====================================================================================//
//                       inline bool named_vars::exists() const                        //
//=====================================================================================//
inline bool named_vars::exists(const std::string &name) const
{
	return std::binary_search(begin(),end(),name,less());
}

//=====================================================================================//
//                           inline bool named_vars::erase()                           //
//=====================================================================================//
inline bool named_vars::erase(const std::string &name)
{
	std::pair<iterator,iterator> it = std::equal_range(begin(),end(),name,less());

	if(it.first != it.second)
	{
		m_vars.erase(it.first);
		return true;
	}

	return false;
}

//=====================================================================================//
//                        inline var& named_vars::operator[]()                         //
//=====================================================================================//
inline var& named_vars::operator[](const std::string &name)
{
	std::pair<iterator,iterator> it = std::equal_range(begin(),end(),name,less());

	if(it.first != it.second)
	{
		return *it.first;
	}
	else
	{
		return *m_vars.insert(it.first,var(name));
	}
}

/*=====================================================================================*\
 *                  inline const var& named_vars::operator[]() const                   * 
\*=====================================================================================*/
inline const var& named_vars::operator[](const std::string &name) const
{
	std::pair<const_iterator,const_iterator> it = std::equal_range(begin(),end(),name,less());

	if(it.first != it.second)
	{
		return *it.first;
	}
	else
	{
		throw var_not_found("Не найдена переменная " + name);
	}
}

//=====================================================================================//
//                              named_vars::get_default()                              //
//=====================================================================================//
inline const char *named_vars::get_default(const std::string &v, const char *d) const
{
	std::pair<const_iterator,const_iterator> it = std::equal_range(begin(),end(),v,less());
	return it.first != it.second ? it.first->get_string() : d;
}

//=====================================================================================//
//                              named_vars::get_default()                              //
//=====================================================================================//
inline std::string named_vars::get_default(const std::string &v, const std::string &d) const
{
	std::pair<const_iterator,const_iterator> it = std::equal_range(begin(),end(),v,less());
	return it.first != it.second ? it.first->value() : d;
}

//=====================================================================================//
//                              named_vars::get_default()                              //
//=====================================================================================//
inline algebra::point3 named_vars::get_default(const std::string &v, const algebra::point3 &d) const
{
	std::pair<const_iterator,const_iterator> it = std::equal_range(begin(),end(),v,less());
	return it.first != it.second ? it.first->get_point3() : d;
}

//=====================================================================================//
//                              named_vars::get_default()                              //
//=====================================================================================//
inline algebra::point2 named_vars::get_default(const std::string &v, const algebra::point2 &d) const
{
	std::pair<const_iterator,const_iterator> it = std::equal_range(begin(),end(),v,less());
	return it.first != it.second ? it.first->get_point2() : d;
}

//=====================================================================================//
//                              named_vars::get_default()                              //
//=====================================================================================//
inline algebra::vector3 named_vars::get_default(const std::string &v, const algebra::vector3 &d) const
{
	std::pair<const_iterator,const_iterator> it = std::equal_range(begin(),end(),v,less());
	return it.first != it.second ? it.first->get_vector3() : d;
}

//=====================================================================================//
//                              named_vars::get_default()                              //
//=====================================================================================//
inline algebra::vector2 named_vars::get_default(const std::string &v, const algebra::vector2 &d) const
{
	std::pair<const_iterator,const_iterator> it = std::equal_range(begin(),end(),v,less());
	return it.first != it.second ? it.first->get_vector2() : d;
}

//=====================================================================================//
//                              named_vars::get_default()                              //
//=====================================================================================//
inline float named_vars::get_default(const std::string &v, float d) const
{
	std::pair<const_iterator,const_iterator> it = std::equal_range(begin(),end(),v,less());
	return it.first != it.second ? it.first->get_float() : d;
}

//=====================================================================================//
//                              named_vars::get_default()                              //
//=====================================================================================//
inline int named_vars::get_default(const std::string &v, int d) const
{
	std::pair<const_iterator,const_iterator> it = std::equal_range(begin(),end(),v,less());
	return it.first != it.second ? it.first->get_int() : d;
}

/*=====================================================================================*\
 *                            inline var &var::operator =()                            * 
\*=====================================================================================*/
inline var &var::operator =(const std::string &str)
{
	m_value = str;
	return *this;
}

/*=====================================================================================*\
 *                            inline var &var::operator =()                            * 
\*=====================================================================================*/
inline var &var::operator =(const char *str)
{
	m_value = str;
	return *this;
}

/*=====================================================================================*\
 *                            inline var &var::operator =()                            * 
\*=====================================================================================*/
inline var &var::operator =(const algebra::point3 &pt)
{
	std::ostringstream str;
	str << pt;
	m_value = str.str();
	return *this;
}

/*=====================================================================================*\
 *                            inline var &var::operator =()                            * 
\*=====================================================================================*/
inline var &var::operator =(const algebra::point2 &pt)
{
	std::ostringstream str;
	str << pt;
	m_value = str.str();
	return *this;
}

/*=====================================================================================*\
 *                            inline var &var::operator =()                            * 
\*=====================================================================================*/
inline var &var::operator =(const algebra::vector3 &pt)
{
	std::ostringstream str;
	str << pt;
	m_value = str.str();
	return *this;
}

/*=====================================================================================*\
 *                                  var::operator =()                                  * 
\*=====================================================================================*/
inline var &var::operator =(const algebra::vector2 &pt)
{
	std::ostringstream str;
	str << pt;
	m_value = str.str();
	return *this;
}


/*=====================================================================================*\
 *                            inline var &var::operator =()                            * 
\*=====================================================================================*/
inline var &var::operator =(float pt)
{
	std::ostringstream str;
	str << std::setprecision(16) << pt;
	m_value = str.str();
	return *this;
}

/*=====================================================================================*\
 *                            inline var &var::operator =()                            * 
\*=====================================================================================*/
inline var &var::operator =(int pt)
{
	std::ostringstream str;
	str << pt;
	m_value = str.str();
	return *this;
}

/*=====================================================================================*\
 *                   inline algebra::point3 var::get_point3() const                    * 
\*=====================================================================================*/
inline algebra::point3 var::get_point3() const
{
	std::istringstream str(m_value);
	algebra::point3 r;
	str >> r;
	return r;
}

/*=====================================================================================*\
 *                   inline algebra::point2 var::get_point2() const                    * 
\*=====================================================================================*/
inline algebra::point2 var::get_point2() const
{
	std::istringstream str(m_value);
	algebra::point2 r;
	str >> r;
	return r;
}

/*=====================================================================================*\
 *                  inline algebra::vector3 var::get_vector3() const                   * 
\*=====================================================================================*/
inline algebra::vector3 var::get_vector3() const
{
	std::istringstream str(m_value);
	algebra::vector3 r;
	str >> r;
	return r;
}

/*=====================================================================================*\
 *                  inline algebra::vector2 var::get_vector2() const                   * 
\*=====================================================================================*/
inline algebra::vector2 var::get_vector2() const
{
	std::istringstream str(m_value);
	algebra::vector2 r;
	str >> r;
	return r;
}

/*=====================================================================================*\
 *                         inline float var::get_float() const                         * 
\*=====================================================================================*/
inline float var::get_float() const
{
	std::istringstream str(m_value);
	float r;
	str >> r;
	return r;
}

/*=====================================================================================*\
 *                           inline int var::get_int() const                           * 
\*=====================================================================================*/
inline int var::get_int() const
{
	std::istringstream str(m_value);
	int r;
	str >> r;
	return r;
}

/*=====================================================================================*\
 *                               inline bool operator<()                               * 
\*=====================================================================================*/
//inline bool operator<(const var &v1, const var &v2)
//{
//	return v1.m_name < v2.m_name;
//}

/*=====================================================================================*\
 *                              inline bool operator==()                               * 
\*=====================================================================================*/
inline bool operator==(const var &v1, const var &v2)
{
	return v1.name() == v2.name() && v1.value() == v2.value();
}

/*=====================================================================================*\
 *                              inline bool operator!=()                               * 
\*=====================================================================================*/
inline bool operator!=(const var &v1, const var &v2)
{
	return !(v1 == v2);
}

} // namespace 'utils'
} // namespace 'mll'

#endif
