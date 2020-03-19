#if !defined(__PRINTF_H_INCLUDED_8093570542662288__)
#define __PRINTF_H_INCLUDED_8093570542662288__

#include <mll/_choose_lib.h>
#include <mll/_export_rules.h>
#include <mll/patterns/nocopy.h>

#include <ostream>
#include <stdarg.h>

namespace mll
{
namespace io
{

std::string MLL_EXPORT printf(const char *, ...);
std::string MLL_EXPORT vprintf(const char *, va_list);

} // namespace 'io'
} // namespace 'mll'

#endif // !defined(__PRINTF_H_INCLUDED_8093570542662288__)