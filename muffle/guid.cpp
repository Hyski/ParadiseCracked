#include "precomp.h"
#include "guid.h"

//=====================================================================================//
//                             std::ostream &operator<<()                              //
//=====================================================================================//
std::ostream &operator<<(std::ostream &stream, const GUID &guid)
{
	LPOLESTR str;
	SAFE_CALL( StringFromCLSID(guid,&str) );
	std::string tmp(wcslen(str)+1, ' ');
	sprintf(&tmp[0],"%S",str);
	stream << tmp;
	CoTaskMemFree(str);
	return stream;
}