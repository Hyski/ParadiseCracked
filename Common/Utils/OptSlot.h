#ifndef _OPTSLOT_HEADER_
#define _OPTSLOT_HEADER_
class VFile;
class OptSlot
{
protected:
		std::vector<unsigned char> m_Data;
		int m_CurPos;
public:
	void Save(FILE *f);
	void Save(SavSlot &sl);
	void Save(std::ostringstream &str);
	
	void Load(VFile *f);
	void Load(SavSlot &sl);
	void Load(std::istringstream &str);
	
	void Rewind(){m_CurPos=0;};
	void operator =(const OptSlot &s){m_Data=s.m_Data;};
	OptSlot(const OptSlot &s):m_CurPos(0){operator=(s);};
	OptSlot():m_CurPos(0){};
	template<class T> friend OptSlot& operator <<(OptSlot &s,const T &d);
	template<class T> friend OptSlot& operator >>(OptSlot &s,T &d);
};
	template<class T> inline OptSlot& operator <<(OptSlot &s,const T &d)
	{
		for(int i=0;i<sizeof(d);i++) s.m_Data.push_back(*((unsigned char*)&d+i));
		return s;
	}
	template<class T> inline OptSlot& operator >>(OptSlot &s,T &d)
	{
		for(int i=0;i<sizeof(d);i++) 
		{
			unsigned char c=s.m_Data[s.m_CurPos+i];
			*((unsigned char*)&d+i)=c;
		}
		s.m_CurPos+=sizeof(d);
		return s;
	}
template<> inline  OptSlot& operator <<(OptSlot &s,const std::string &d)
{
	int i=d.size();
	s<<i;
	for(i=0;i<d.size();i++) s<<d[i];
	return s;
}
template<> inline OptSlot& operator >>(OptSlot &s,std::string &d)
{
	int i;
	s>>i;
	d.reserve(d.size()+i);
	while(i--)
		{
		unsigned char t;
		s>>t;
		d.push_back(t);
		}
	return s;
}


#endif