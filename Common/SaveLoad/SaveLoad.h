//
// —истема сохранени€ объектов
//

#ifndef _PUNCH_SAVELOAD_H_
#define _PUNCH_SAVELOAD_H_

//#include "../DataMgr/DataMgr.h" Punch: уже включен в precomp.h

//определение буфера данных
typedef std::vector<unsigned char> data_buff_t;

//
//абстракци€ файла дл€ записи Save'a
//
class AbstractFile{
public:
    //узнать текущее положение указател€ в файле
    virtual long GetPos() = 0;
    //установить указатель файла на опред позицию
    virtual void Seek(long offset, int origin) = 0;
    //запись / чтение из файла
    virtual size_t Write(void* ptr, size_t size) = 0;
    virtual size_t Read(void* ptr, size_t size) = 0;
};

//
// обертка на стандартный файл
//
class StdFile: public AbstractFile{
public:

    StdFile(const char* name, const char* type):
        m_hfile(fopen(name, type)){}

    ~StdFile()
    {
      if(m_hfile) fclose(m_hfile);
    }

    //инициализаци€ файла была успешной?
    bool IsOk() const
    {
        return m_hfile != NULL;
    }

    long GetPos()
    {
        return ftell(m_hfile);
    }
    void Seek(long offset, int origin)
    {
        fseek(m_hfile, offset, origin);
    }
    size_t Write(void* ptr, size_t size)
    {
        return fwrite(ptr, 1, size, m_hfile);
    }
    size_t Read(void* ptr, size_t size)
    {
        return fread(ptr, 1, size, m_hfile);
    }

private:
    FILE* m_hfile;
};

//
// обертка на файл считываемый из пака
//
class VFile;
class PackageFile: public AbstractFile
{
private:
	VFile *m_pVFile;
	std::string m_sFileName;
public:
	PackageFile(const char* name) : m_pVFile(DataMgr::Load(name)){m_sFileName = name;}
	~PackageFile(){DataMgr::Release(m_sFileName.c_str());}
public:
    //инициализаци€ файла была успешной?
    bool IsOk() const
    {
        return m_pVFile->Size() != 0;
    }
    long GetPos()
    {
        return m_pVFile->Tell();
    }
    void Seek(long offset, int origin)
    {
      switch(origin)
        {
        case SEEK_CUR:
          m_pVFile->Seek(offset,VFile::VFILE_CUR);
          break;
        case SEEK_END:
		      m_pVFile->Seek(offset,VFile::VFILE_END);
          break;
        case SEEK_SET:
		      m_pVFile->Seek(offset,VFile::VFILE_SET);
          break;
        }
    }
    size_t Write(void* , size_t )
    {
		return 0;
    }
    size_t Read(void* ptr, size_t size)
    {
        return m_pVFile->Read(ptr,size);
    }
};


//
//абстракци€ дл€ хранилища информации
//
class Storage{
public:

    enum storage_mode{
        SM_LOAD,
        SM_SAVE,
    };

    Storage(AbstractFile* file, storage_mode mode);
    ~Storage();

    //сохранить/загрузить буфер с данными
    void SaveData(const char* cell_name, const data_buff_t& data);
    void LoadData(const char* cell_name, data_buff_t& data);

    //вид текущей операции
    bool IsSaving() const;

    //проверить нормальный ли Save
    bool IsValid();

    //получить версию текщей системы SaveLoad
    unsigned GetVersion() const;

private:

    //запретим копирующий конструктор
    Storage(const Storage& store);
    
private:

    struct header{
        char        m_id_str[8];  //идентиф. строка
        unsigned    m_version;    //верси€ 
        unsigned    m_index_size; //количество размер индекса
        long        m_index_off;  //смещени€ до индекса
        unsigned    m_reserved;

        header();
    };

    typedef std::map<std::string,long> str_index_t;

    AbstractFile* m_file;
    
    storage_mode  m_mode;    
    str_index_t   m_index;
    header        m_hdr; 
};

//
//класс - абстракци€ дл€ €чейки сохранени€
//
class SavSlot{
public:

    SavSlot(Storage* st, const char* name, size_t size = 128);
    ~SavSlot();
    
    //сохранить/загрузить данные в €чейку
    void Save(void* ptr, size_t size);
    void Load(void* ptr, size_t size);
    
    //получить ссылку на Storage этой €чейки
    Storage* GetStore();

    //режим работы €чейки
    bool IsSaving() const;

private:

    Storage*     m_strg;
    std::string  m_name;
    data_buff_t  m_data; 
    bool         m_fsave;
    unsigned     m_cur;
};

//
// ќператоры дл€ записи/чтени€ в Save
//

inline SavSlot& operator << (SavSlot& st, bool val)
{
    st.Save(&val, sizeof(bool));    
    return st;
}

inline SavSlot& operator >> (SavSlot& st, bool& val)
{
    st.Load(&val, sizeof(bool));
    return st;
}

inline SavSlot& operator << (SavSlot& st, unsigned val)
{
    st.Save(&val, sizeof(unsigned));    
    return st;
}

inline SavSlot& operator >> (SavSlot& st, unsigned& val)
{
    st.Load(&val, sizeof(unsigned));
    return st;
}

inline SavSlot& operator << (SavSlot& st, int val)
{
    st.Save(&val, sizeof(int));    
    return st;
}

inline SavSlot& operator >> (SavSlot& st, int& val)
{
    st.Load(&val, sizeof(int));
    return st;
}

inline SavSlot& operator << (SavSlot& st, float val)
{
    st.Save(&val, sizeof(float));
    return st;
}

inline SavSlot& operator >> (SavSlot& st, float& val)
{
    st.Load(&val, sizeof(float));
    return st;
}

inline SavSlot& operator << (SavSlot& st, char val)
{
    st.Save(&val, sizeof(char));
    return st;
}

inline SavSlot& operator >> (SavSlot& st, char& val)
{
    st.Load(&val, sizeof(char));
    return st;
}

inline SavSlot& operator << (SavSlot& st, short val)
{
    st.Save((void*)&val, sizeof(short));
    return st;
}

inline SavSlot& operator >> (SavSlot& st, short& val)
{
    st.Load((void*)&val, sizeof(short));
    return st;
}

inline SavSlot& operator << (SavSlot& st, const std::string& str)
{
    st.Save((void*)str.c_str(), str.size()+1);
    return st;
}

inline SavSlot& operator >> (SavSlot& st, std::string& str)
{
    char ch;    
    str.clear();
    do{
        st >> ch;
        if(ch) str += ch;
    }while(ch);

    return st;
}

//
// ~~~~~~~~~~~~ inline ~~~~~~~~~~~
//

inline bool Storage::IsSaving() const
{
    return m_mode == SM_SAVE;
}

inline unsigned Storage::GetVersion() const
{
    return m_hdr.m_version;
}

inline bool Storage::IsValid()
{
    return true;
}

inline Storage* SavSlot::GetStore()
{
    return m_strg;
}

inline bool SavSlot::IsSaving() const
{
    return m_fsave;
}

#endif // _PUNCH_SAVELOAD_H_
