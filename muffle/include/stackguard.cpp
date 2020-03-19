#include "precomp.h"
#include "stackguard.h"
std::vector<const char *> StackGuard::Names; 

std::string StackGuard::GetAllNames()
	{
	std::string names;
	static const int LINEWIDTH=40;
	int nlines=LINEWIDTH;
	for(int i=0;i<Names.size();i++)
		{
		if(names.size()) names+=" => ";
		names+=std::string(Names[i]);
		if(names.size()>nlines)
			{
			names+="\n";
			nlines+=LINEWIDTH;
			}
		}
	return names;
	}


