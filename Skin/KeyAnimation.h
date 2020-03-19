// KeyAnimation.h: interface for the KeyAnimation class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KEYANIMATION_H__A11D3D2A_FF88_42BA_AA48_DBCA5C400683__INCLUDED_)
#define AFX_KEYANIMATION_H__A11D3D2A_FF88_42BA_AA48_DBCA5C400683__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class SavSlot;
class OptSlot;
class KeyAnimation  
  {
  protected:
    typedef std::map<float, point3> TransKeys;       //ключи для смещения объекта
    typedef std::map<float, Quaternion> RotKeys;  //ключи для поворота объекта
  protected:
    TransKeys Trans;
    RotKeys   Angles;
  public: //    Construction/Destruction
		KeyAnimation(const KeyAnimation &a){*this=a;}
    KeyAnimation();
    virtual ~KeyAnimation();
  public:
    //получение значения смещения в указанный момент времени
    void GetTrans(point3 *Trans,float Time) const;
    //получение значения поворота в указанный момент времени
    void GetAngle(Quaternion *Quat,float Time) const;
    //установка ключа смещения
    void SetKey(float Time, const point3 &Trans);
    //установка ключа поворота
    void SetKey(float Time, const Quaternion &Quat);
    //установка ключа положения
    void SetKey(float Time, const Quaternion &Quat, const point3 &Trans);
	//время последнего ключа
    float GetLastTime() const
      {
			float a=0,b=0;
			if(Angles.size())	a=Angles.rbegin()->first;
			if(Trans.size())	b=Trans.rbegin()->first;
      return a>b?a:b;
      }
		//возвращает массив расположения ключей в анимации
		//внимание: ключи ДОБАВЛЯЮТСЯ в множество, если нужно, очищайте его сами
		void GetKeyTimes(std::set<float> *times); 
		//выкинуть часть ключей, если не превышена указанна погрешность
		void 	Decimate(float _err);
    virtual void Load(FILE *f);
    virtual void Save(FILE *f) const;
    virtual void SaveBin(FILE *f) const;
    virtual void LoadBin(VFile *f);
    virtual void Load(std::istringstream &stream);
    virtual void Save(std::ostringstream &stream) const;
    bool MakeSaveLoad(SavSlot &slot);
    bool Save(OptSlot &slot);
    bool Load(OptSlot &slot);
		KeyAnimation &operator=(const KeyAnimation &a){Trans=a.Trans;Angles=a.Angles;return *this;}
  };

#endif // !defined(AFX_KEYANIMATION_H__A11D3D2A_FF88_42BA_AA48_DBCA5C400683__INCLUDED_)
