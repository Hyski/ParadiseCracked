//=====================================================================================//
//                                                                                     //
//                                  Paradise Cracked                                   //
//                                                                                     //
//                             Copyright by MiST land 2002                             //
//                                                                                     //
//   -------------------------------------------------------------------------------   //
//   Description:                                                                      //
//                                                                                     //
//   -------------------------------------------------------------------------------   //
//   Author: Pavel A.Duvanov                                                           //
//   Date:   29.11.2002                                                                //
//                                                                                     //
//=====================================================================================//
#ifndef __MOD_MGR_H_INCLUDED_5211432865184138__
#define __MOD_MGR_H_INCLUDED_5211432865184138__

//=====================================================================================//
//                                      class Mod                                      //
//=====================================================================================//
class Mod
{
private:

	std::string m_author;			//	имя автора
	std::string m_name;				//	наименование мода
	std::string m_desc;				//	описание мода
	std::string m_id;				//	идентификатор мода
	bool m_isValid;					//	флаг присутсвия мода

public:
	Mod(const std::string& id);
	~Mod();
public:
	//	проверка на присутствие мода
	bool isValid(void)  const;
	//	получить имя автора
	const std::string& author(void) const;
	//	получить наименование мода
	const std::string& name(void) const;
	//	получить описание мода
	const std::string& desc(void) const;
	//	получить идентификатор мода
	const std::string& id(void) const;

};

//	проверка на присутствие мода
inline bool Mod::isValid(void) const
{
	return m_isValid;
}
//	получить имя автора
inline const std::string& Mod::author(void) const
{
	return m_author;
}
//	получить наименование мода
inline const std::string& Mod::name(void) const
{
	return m_name;
}
//	получить описание мода
inline const std::string& Mod::desc(void) const
{
	return m_desc;
}
//	получить идентификатор мода
inline const std::string& Mod::id(void) const
{
	return m_id;
}

//=====================================================================================//
//                                    class ModMgr                                     //
//=====================================================================================//
class ModMgr
{
public:

	static const std::string m_modPath;			//	путь до каталога с модами
	static const std::string m_modInfo;			//	наименование файлика с описанием мода

	typedef std::vector<Mod*> mods_t;

private:

	mods_t m_mods;								//	список доступных модов

public:
	ModMgr();
	~ModMgr();
public:
	//	получить список доступых модов
	const mods_t& mods(void) const;

};

//	получить список доступых модов
inline const ModMgr::mods_t& ModMgr::mods(void) const
{
	return m_mods;
}


#endif //__MOD_MGR_H_INCLUDED_5211432865184138__
