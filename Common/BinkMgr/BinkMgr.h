/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   02.03.2001

************************************************************************/
#ifndef _BINK_MGR_H_
#define _BINK_MGR_H_

class BinkSurface;

class BinkMgr
{
private:
	struct Deleter 
	{
	public:
		BinkMgr *m_pInstance;
	public:
		Deleter(){m_pInstance = 0;}
		~Deleter(){if(m_pInstance) delete m_pInstance;}
	};
	friend Deleter;
	static Deleter m_Deleter;
private:
	class Reference;
	std::map<std::string,Reference*> m_mBinkSurfaces;		//	карта поверхностей
	std::map<std::string,Reference*>::iterator m_Updated;
private:
	BinkMgr();
	virtual ~BinkMgr();
public:
	//	распаковываем все открытые бики
	void Update(void);
	//	распаковка конкретного бика
	void Update(const char *pName);
public:	//	интерфейсы
	//	получить поверхность по идентификатору бинка
	LPDIRECTDRAWSURFACE7 Surface(const char *pName);
	//	вернуть кол-во бинков в менеджере
	inline int Quantity(void) const;
	//	уничтожить поверхность и закрыть бинки по идентификатору
	void Release(const char *pName);
	//	уничтожить все поверхности
	void Release(void);
private:
	void LoadBink(const char *pName);
	void SetColorKey(LPDIRECTDRAWSURFACE7 surface);
public:
	static BinkMgr *Instance(void);
};

inline BinkMgr *BinkMgr::Instance(void)
{
	if(!m_Deleter.m_pInstance) m_Deleter.m_pInstance = new BinkMgr();
	return m_Deleter.m_pInstance;
}

inline int BinkMgr::Quantity(void) const
{
	return m_mBinkSurfaces.size();
}

//***************************************************************//
//**************** class BinkMgr::Reference *********************//
class BinkMgr::Reference
{
private:
	BinkSurface* m_Value;
	int m_Counter;	
public:	
	Reference();
	Reference(BinkSurface* value);
	virtual	~Reference();
public:	
	BinkSurface* AddRef(void){m_Counter++;return m_Value;}
	BinkSurface* Value(void){return m_Value;}
	int Release(void){if(m_Counter){m_Counter--;} return m_Counter;}
	int RefCount(void){return m_Counter;}
};


#endif	//_BINK_MGR_H_