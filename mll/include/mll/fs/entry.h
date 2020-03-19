#if !defined(__ENTRY_H_INCLUDED_8093570542662288__)
#define __ENTRY_H_INCLUDED_8093570542662288__

#pragma once

#include "_export_rules.h"
#include "_choose_lib.h"

#pragma warning( disable : 4251)

namespace mll
{

namespace fs
{
	
class critical_section;

class FILE_SYSTEM_EXPORT entry
{
	critical_section *m_csect;
	
public:
	entry(critical_section *csect);
	~entry();
};

}

}

#endif