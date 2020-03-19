#if !defined(__ENTRY_H_INCLUDED_6525749268230297__)
#define __ENTRY_H_INCLUDED_6525749268230297__

class CriticalSection;

//=====================================================================================//
//                                     class Entry                                     //
//=====================================================================================//
class Entry
{
	CriticalSection &m_section;
public:
	Entry(CriticalSection &sect);
	~Entry();
};

#endif // !defined(__ENTRY_H_INCLUDED_6525749268230297__)