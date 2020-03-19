#if !defined(__MASK_REGEX_H_INCLUDED_7552370787048739__)
#define __MASK_REGEX_H_INCLUDED_7552370787048739__


#include "_export_rules.h"
#include "_choose_lib.h"

#include <string>


namespace mll
{

namespace fs
{

//=====================================================================================//
//                                  class mask_regex                                   //
//=====================================================================================//
/*	Служебный класс проверки маски. Использует regex из Boost.
*/
class FILE_SYSTEM_EXPORT mask_regex
{
	std::string result_mask;

public:
	mask_regex(const std::string &);

	bool is_valid(const std::string &);
};

}

}

#endif // !defined(__MASK_REGEX_H_INCLUDED_7552370787048739__)