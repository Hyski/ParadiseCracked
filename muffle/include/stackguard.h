#ifndef _STACKGUARD_HEADER_
#define _STACKGUARD_HEADER_

#if defined(_DEBUG) || defined(_HOME_VERSION)	|| defined(STACK_GUARD_ON)
#define STACK_GUARD(a) StackGuard _ST_G_(a);
#else
#define STACK_GUARD(a) ;
#endif

class StackGuard
	{
	public:
		StackGuard::StackGuard(const char *FuncName)
			:m_FuncName(FuncName)
			{
			AddName(FuncName);
			}
		StackGuard::~StackGuard()
			{
			RemName(m_FuncName);
			}


	public:
		static std::string GetAllNames();
		static void AddName(const char *FuncName){Names.push_back(FuncName);};
		static void RemName(const char *FuncName)
			{
			if(Names.size())
				{
				if(Names.back()==FuncName) Names.pop_back();
				else throw CASUS("error stack guarding");
				}
			};
	private:
		static std::vector<const char *> Names;
	private:
			const char *m_FuncName;
	};




#endif