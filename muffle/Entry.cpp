#include "precomp.h"
#include "Entry.h"
#include "CriticalSection.h"

//=====================================================================================//
//                                   Entry::Entry()                                    //
//=====================================================================================//
Entry::Entry(CriticalSection &sect)
:	m_section(sect)
{
	m_section.enter();
}

//=====================================================================================//
//                                   Entry::~Entry()                                   //
//=====================================================================================//
Entry::~Entry()
{
	m_section.leave();
}