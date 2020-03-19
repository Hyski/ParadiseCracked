// KeyAnimation.cpp: implementation of the KeyAnimation class.
//
//////////////////////////////////////////////////////////////////////
#include "precomp.h"
#include "KeyAnimation.h"
#include "../common/saveload/saveload.h"
#include "../common/utils/optslot.h"
#include <sstream>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
bool KeyAnimation::Save(OptSlot &slot)
	{
STACK_GUARD("KeyAnimation::Save");
  slot<<Angles.size();
  slot<<Trans.size();
  //запишем углы поворота
  RotKeys::const_iterator ang;
  for(ang=Angles.begin();ang!=Angles.end();ang++)
    {
    slot<<ang->first<<ang->second.x<<ang->second.y<<ang->second.z<<ang->second.w;
    }
  //запишем  данные о переносе
  TransKeys::const_iterator tr;
  for(tr=Trans.begin();tr!=Trans.end();tr++)
    {
    slot<<tr->first<<tr->second.x<<tr->second.y<<tr->second.z;
    }
	return true;
	}
bool KeyAnimation::Load(OptSlot &slot)
	{
STACK_GUARD("KeyAnimation::Load");
    int asize,tsize;
    slot>>asize>>tsize;
    float x,y,z,w,t;
    //прочитаем углы поворота
    Angles.clear();
    Trans.clear();
    int i;
    for(i=0;i<asize;i++)
      {
      slot>>t>>x>>y>>z>>w;
      Angles[t]=Quaternion(w,x,y,z);
      }
    //прочитаем  данные о переносе
    for(i=0;i<tsize;i++)
      {
      slot>>t>>x>>y>>z;
      Trans[t]=point3(x,y,z);
      }
		Decimate(0);
		return true;
	}

KeyAnimation::KeyAnimation()
{

}

KeyAnimation::~KeyAnimation()
{

}
void KeyAnimation::GetTrans(point3 *Offs,float Time) const
  {
STACK_GUARD("KeyAnimation::GetTrans");
  TransKeys::const_iterator st,et;
  et=Trans.upper_bound(Time);
  if(et==Trans.end())
    {//не нужно интерполяции - да, прошли те времена
    *Offs=Trans.rbegin()->second;
    }
  else
    {
    st=et;
    if(et==Trans.begin())
      {//Не нужно интерполяции - время анимации еще не пришло
      *Offs=et->second;
      }
    else
      {
      //интерполяция
      st--;
      float in;
      in=(float)(Time-st->first)/(et->first-st->first);
      Interpolate(Offs, et->second, st->second,in);
      //*Trans=et->second*in+st->second*(1-in);
      }
    }
  }
void KeyAnimation::GetKeyTimes(std::set<float> *times)
	{
STACK_GUARD("KeyAnimation::GetKeyTimes");
	TransKeys::iterator tit=Trans.begin();       //ключи для смещения объекта
	RotKeys::iterator rit=Angles.begin();  //ключи для поворота объекта
	for(;tit!=Trans.end();tit++)
		times->insert(tit->first);
	for(;rit!=Angles.end();rit++)
		times->insert(rit->first);
	}
void KeyAnimation::GetAngle(Quaternion *Quat,float Time) const
  {
STACK_GUARD("KeyAnimation::GetAngle");
    RotKeys::const_iterator sq,eq;
  eq=Angles.upper_bound(Time);

    if(eq==Angles.end())
      {//не нужно интерполяции - да, прошли те времена
      *Quat=Angles.rbegin()->second;
      }
    else
      {
      sq=eq;
      if(eq==Angles.begin())
        {//Не нужно интерполяции - время анимации еще не пришло
        *Quat=eq->second;
        }
      else
        {
        //интерполяция
        sq--;
        float in;
        in=(float)(Time-sq->first)/(eq->first-sq->first);
        *Quat=Quat->Slerp(in,sq->second,eq->second);
        }
      }
  }
void KeyAnimation::SetKey(float Time, const point3 &Offs)
  {
STACK_GUARD("KeyAnimation::SetKey(offs)");
  Trans[Time]=Offs;
  }
void KeyAnimation::SetKey(float Time, const Quaternion &Quat)
  {
STACK_GUARD("KeyAnimation::SetKey(quat)");
  Angles[Time]=Quat;
  }
void KeyAnimation::SetKey(float Time, const Quaternion &Quat, const point3 &Offs)
  {
STACK_GUARD("KeyAnimation::SetKey(offs&quat)");
  Trans[Time]=Offs;
  Angles[Time]=Quat;
  }
void KeyAnimation::Load(FILE *f)
  {
STACK_GUARD("KeyAnimation::Load");
  int asize,tsize;
  fscanf(f,"Angles Keys:%d\n",&asize);
  fscanf(f,"Trans Keys:%d\n",&tsize);
  int i;
  float x,y,z,w,t;
  //прочитаем углы поворота
  for(i=0;i<asize;i++)
    {
    fscanf(f,"%f[%f %f %f %f] ",&t,&x,&y,&z,&w);
    Angles[t]=Quaternion(w,x,y,z);
    }
  fscanf(f,"\n");

  //прочитаем  данные о переносе
  for(i=0;i<tsize;i++)
    {
    fscanf(f,"%f[%f %f %f] ",&t,&x,&y,&z);
    Trans[t]=point3(x,y,z);
    }
  fscanf(f,"\n");
  //char a;
  //fread(&a,1,sizeof(char),f);
  }
void KeyAnimation::Save(FILE *f) const
  {
STACK_GUARD("KeyAnimation::Save");
  fprintf(f,"Angles Keys:%d\n",Angles.size());
  fprintf(f,"Trans Keys:%d\n",Trans.size());
  //запишем углы поворота
  RotKeys::const_iterator ang;
  for(ang=Angles.begin();ang!=Angles.end();ang++)
    {
    fprintf(f,"%f[%f %f %f %f] ",ang->first,ang->second.x,ang->second.y,ang->second.z,ang->second.w);
    }
  fprintf(f,"\n");

  //запишем  данные о переносе
  TransKeys::const_iterator tr;
  for(tr=Trans.begin();tr!=Trans.end();tr++)
    {
    fprintf(f,"%f[%f %f %f] ",tr->first,tr->second.x,tr->second.y,tr->second.z);
    }
  fprintf(f,"\n");

  }
void KeyAnimation::SaveBin(FILE *f) const
  {
STACK_GUARD("KeyAnimation::SaveBin");
#define _SAVE(a) fwrite(&a,1,sizeof(a),f);
  int asize=Angles.size();
  int tsize=Trans.size();
  _SAVE(asize);
  _SAVE(tsize);
  //запишем углы поворота
  RotKeys::const_iterator ang;
  for(ang=Angles.begin();ang!=Angles.end();ang++)
    {
    _SAVE(ang->first);
    _SAVE(ang->second);
    }
  //запишем  данные о переносе
  TransKeys::const_iterator tr;
  for(tr=Trans.begin();tr!=Trans.end();tr++)
    {
    _SAVE(tr->first);
    _SAVE(tr->second);
    }
#undef _SAVE
  }
void KeyAnimation::LoadBin(VFile *f)
  {
STACK_GUARD("KeyAnimation::LoadBin");
  #define _LOAD(a) f->Read(&a,sizeof(a));
  int asize,tsize;
  _LOAD(asize);
  _LOAD(tsize);
  int i;
  float t;
  Quaternion lq;
  point3 lp;
  //прочитаем углы поворота
  for(i=0;i<asize;i++)
    {
    _LOAD(t);
    _LOAD(lq);
    Angles[t]=lq;
    }
  //прочитаем  данные о переносе
  for(i=0;i<tsize;i++)
    {
    _LOAD(t);
    _LOAD(lp)
    Trans[t]=lp;
    }
#undef _LOAD
  }
bool KeyAnimation::MakeSaveLoad(SavSlot &slot)
  {
STACK_GUARD("KeyAnimation::MakeSaveLoad");
  if(slot.IsSaving())
    {
  slot<<Angles.size();
  slot<<Trans.size();
  //запишем углы поворота
  RotKeys::const_iterator ang;
  for(ang=Angles.begin();ang!=Angles.end();ang++)
    {
    slot<<ang->first<<ang->second.x<<ang->second.y<<ang->second.z<<ang->second.w;
    }
  //запишем  данные о переносе
  TransKeys::const_iterator tr;
  for(tr=Trans.begin();tr!=Trans.end();tr++)
    {
    slot<<tr->first<<tr->second.x<<tr->second.y<<tr->second.z;
    }
    }
  else
    {
    int asize,tsize;
    slot>>asize>>tsize;
    float x,y,z,w,t;
    //прочитаем углы поворота
    Angles.clear();
    Trans.clear();
    int i;
    for(i=0;i<asize;i++)
      {
      slot>>t>>x>>y>>z>>w;
      Angles[t]=Quaternion(w,x,y,z);
      }
    //прочитаем  данные о переносе
    for(i=0;i<tsize;i++)
      {
      slot>>t>>x>>y>>z;
      Trans[t]=point3(x,y,z);
      }
    } 
  return true;
  }
void KeyAnimation::Load(std::istringstream &stream)
  {
STACK_GUARD("KeyAnimation::Load");
  int asize,tsize;
  (stream.ignore(strlen("Angles Keys:"))>>asize).ignore(strlen("\n"));
  (stream.ignore(strlen("Trans Keys:"))>>tsize).ignore(strlen("\n"));
  int i;
  float x,y,z,w,t;
  //прочитаем углы поворота
  for(i=0;i<asize;i++)
    {
    ((stream>>t).ignore(1)>>x>>y>>z>>w).ignore(1);
    Angles[t]=Quaternion(w,x,y,z);
    }
  stream.ignore(strlen("\n"));

  //прочитаем  данные о переносе
  for(i=0;i<tsize;i++)
    {
    ((stream>>t).ignore(1)>>x>>y>>z).ignore(1);
    Trans[t]=point3(x,y,z);
    }
  stream.ignore(strlen("\n"));
  }
void KeyAnimation::Save(std::ostringstream &stream) const
  {
STACK_GUARD("KeyAnimation::Save");
  stream<<"Angles Keys:"<<Angles.size()<<"\n";
  stream<<"Trans Keys:"<<Trans.size()<<"\n";
  //запишем углы поворота
  RotKeys::const_iterator ang;
  for(ang=Angles.begin();ang!=Angles.end();ang++)
    {
    stream<<ang->first<<"["<<ang->second.x<<" "
      <<ang->second.y<<" "<<ang->second.z<<" "
      <<ang->second.w<<"]";
    }
  stream<<"\n";

  //запишем  данные о переносе
  TransKeys::const_iterator tr;
  for(tr=Trans.begin();tr!=Trans.end();tr++)
    {
    stream<<tr->first<<"["
      <<tr->second.x<<" "
      <<tr->second.y<<" "
      <<tr->second.z<<"]";
    }
  stream<<"\n";
  }
bool _eq(const Quaternion &a,const Quaternion &b)
	{
	return a.x==b.x&&a.y==b.y&&a.z==b.z&&a.w==b.w;
	}
void 	KeyAnimation::Decimate(float /*_err*/)
	{
STACK_GUARD("KeyAnimation::Decimate");
	//int size=Angles.size()+Trans.size();
	{
  RotKeys::iterator ae=Angles.end(), ang=Angles.begin(), ang_1=ae, ang_2=ae;
	for(int i=0; i<2 && ang!=ae ; i++, ang_2=ang_1, ang_1=ang, ang++) {/**/};

  for(; ang!=ae; ang++)
    {
			 if(_eq(ang->second,ang_2->second)&&_eq(ang_1->second,ang_2->second))
				 {
				 Angles.erase(ang_1);
				 ae=Angles.end();
				 ang_1=ang;
				 continue;
				 }
			 ang_2=ang_1;
			 ang_1=ang;
		}
	}
	{
  TransKeys::iterator te=Trans.end(), tr=Trans.begin(), tr_1=te, tr_2=te;
	for(int i=0; i<2 && tr!=te ; i++, tr_2=tr_1, tr_1=tr, tr++) {/**/};

  for(; tr!=te; tr++)
    {
			 if(tr->second==tr_2->second && tr_1->second==tr_2->second)
				 {
				 Trans.erase(tr_1);
				 te=Trans.end();
				 tr_1=tr;
				 continue;
				 }
			 tr_2=tr_1;
			 tr_1=tr;
		}
	}


//	int size2=size-(Angles.size()+Trans.size());
	return;
	}

