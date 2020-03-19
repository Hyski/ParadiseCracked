#include "precomp.h"
#include "optslot.h"
#include "vfile.h"
void OptSlot::Save(FILE *f)
{
	int i=m_Data.size();
	fwrite(&i,1,sizeof(int),f);
	for(i=0;i<m_Data.size();i++)
		fwrite(&m_Data[i],1,1,f);
}
void OptSlot::Save(SavSlot &sl)
{
	int i=m_Data.size();
	sl<<i;
	for(i=0;i<m_Data.size();i++)
		sl<<(char)m_Data[i];
}
void OptSlot::Save(std::ostringstream &str)
{
	int i=m_Data.size();
	str<</*std::hex<<*/i<<" ";
	for(i=0;i<m_Data.size();i++)
		str<<(int)m_Data[i]<<" ";
	str<<"\n";
}
void OptSlot::Load(VFile *f)
{
	int i; unsigned char j;
	f->Read(&i,sizeof(int));
	unsigned char *ch=new unsigned char [i];
	unsigned char *chp=ch;
	f->Read(ch,i);
	m_Data.reserve(i);
	while(i--)
		{
		m_Data.push_back(*chp++);
		}
	delete ch;
}
void OptSlot::Load(SavSlot &sl)
{
	int i; char j;
	sl>>i;
	unsigned char *ch=new unsigned char [i];
	unsigned char *chp=ch;
	sl.Load(ch,i);
	m_Data.reserve(i);
	while(i--)
	{
		m_Data.push_back(*chp++);
	}
	delete ch;
}
void OptSlot::Load(std::istringstream &str)
{
	int i,k; unsigned char j;
	str>>/*std::hex>>*/i;
	while(i--)
	{
		str.ignore(1)>>k;
		j=k;
		m_Data.push_back(j);
	}
	str.ignore(strlen("\n"));
}
