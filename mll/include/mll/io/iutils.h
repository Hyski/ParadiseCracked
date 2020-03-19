#if !defined(__IUTILS_H_INCLUDED_5992692519605117__)
#define __IUTILS_H_INCLUDED_5992692519605117__

#include <mll/_export_rules.h>
#include <mll/_choose_lib.h>
#include <mll/debug/exception.h>

#include <istream>
#include <stdexcept>

namespace mll
{
namespace io
{

//=====================================================================================//
//                                  class _i_verifier                                  //
//=====================================================================================//
class _i_verifier
{
	const char *m_str;
	friend MLL_EXPORT std::istream &operator>>(std::istream &, const _i_verifier &);

public:
	_i_verifier(const char *str) : m_str(str) {}
};

MLL_EXPORT std::istream &operator>>(std::istream &stream, const _i_verifier &vf);
inline ibinstream &operator>>(ibinstream &stream, const _i_verifier &vf) { return stream; }

// Манипулятор verify сверяет строку из потока с контрольной. Является атомарным, т. е.
//   в случае совпадения строки tellg() будет после строки, а если строка не совпадает
//   или возникла проблема с потоком, то поток устанавливается в исходное состояние
inline _i_verifier verify(const char *str) { return _i_verifier(str); }

typedef debug::exception_template<struct _verify_not_same_error_tag> verify_not_same_error;
typedef debug::exception_template<struct _verify_stream_failure_tag> verify_stream_failure;

} // namespace 'io'
} // namespace 'mll'

#endif // !defined(__IUTILS_H_INCLUDED_5992692519605117__)