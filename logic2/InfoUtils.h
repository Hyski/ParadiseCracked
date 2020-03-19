//
// классы для работы с описаниями и энцклопедией
//

#ifndef _PUNCH_INFOUTILS_H_
#define _PUNCH_INFOUTILS_H_

//
// хранилище описаний
//
template<class InfoClass, class InfoType, class InfoFactory>
class InfoArchive{
public:

    typedef InfoType  info_type;
    typedef InfoClass info_class;

private:

    struct node_s{

        node_s*    m_next;
        info_class* m_info;
        
        int m_refs;
    
        node_s(node_s* next, info_class* info, int refs) : 
            m_refs(refs), m_next(next), m_info(info){}

        ~node_s() { delete m_info; }
    };

    node_s*     m_first;
    InfoFactory m_factory;

    static InfoArchive m_instance;

private:

    node_s* Find(info_type type, const rid_t& rid)
    {
        for(node_s* ptr = m_first; ptr; ptr = ptr->m_next)
            if(     ptr->m_info->GetType() == type
                &&  ptr->m_info->GetRID() == rid)
                return ptr;
            
        return 0;
    }

public:

    InfoArchive() : m_first(0) {} 

    ~InfoArchive()
    {
        while(node_s* ptr = m_first){
            m_first = m_first->m_next;
            delete ptr;
        }
    }

    //singleton
    static InfoArchive* GetInst(){ return &m_instance; }

    //получить ссылку на описание
    info_class* CreateRef(info_type type, const rid_t& rid)
    {
        if(node_s* ptr = Find(type, rid)){
            ptr->m_refs ++;
            return ptr->m_info;
        }
        
        m_first = new node_s(m_first, m_factory.Create(type, rid), 1);
        return  m_first->m_info;        
    }

    //отпустить ссылку на описание
    void DeleteRef(const info_class* info)
    {
        node_s* previous = 0;
        node_s* current  = m_first;
        
        while(current){
            
            if(current->m_info == info){
                
                current->m_refs --;
                
                if(current->m_refs > 0)
                    return;
                
                if(previous)
                    previous->m_next = current->m_next;
                else
                    m_first = current->m_next;
                
                delete current;
                return;
            }
            
            previous = current;
            current  = current->m_next;
        }
    }
};

template<class InfoClass, class InfoType, class InfoFactory>
InfoArchive<InfoClass, InfoType, InfoFactory> InfoArchive<InfoClass, InfoType, InfoFactory>::m_instance; 

//
// энциклопедия описаний
//
template<class Archive>
class InfoHandbook{
public:

    typedef Archive info_archive;
    typedef typename Archive::info_type  info_type;
    typedef typename Archive::info_class info_class;

private:

    struct node_s {
        rid_t     m_rid;
        info_type m_type;
        unsigned  m_flags;

        node_s*   m_next;

        node_s(node_s* next, info_type type, const rid_t& rid, unsigned flags) :
            m_type(type), m_next(next), m_rid(rid), m_flags(flags) {}
    };

    node_s* m_first;

    static InfoHandbook m_instance;

private:

    node_s* FindNode(info_type type, const rid_t& rid)
    {
        for(node_s* ptr = m_first; ptr; ptr = ptr->m_next)
            if(ptr->m_type == type && ptr->m_rid == rid)
                return ptr;
            
        return 0;
    }

public:

    InfoHandbook() : m_first(0) {}

    ~InfoHandbook() { Reset(); }

    static InfoHandbook* GetInst(){ return &m_instance; }

    //очистить энциклопедию
    void Reset()
    {
        while(node_s* ptr = m_first){
            m_first = m_first->m_next;
            delete ptr;
        }
    }

    enum flag_type{
        F_NEW_REC = 1 << 0, //признак новой записи
    };

    //поместить описание в энциклопедию
    void Push(const info_class* info, unsigned flags = 0)
    {
        //если подобная запись уже есть
        if(FindNode(info->GetType(), info->GetRID()))
            return;       
        
        m_first = new node_s(m_first, info->GetType(), info->GetRID(), flags);
    }
    
    void Push(info_type type, const rid_t& rid, unsigned flags = 0)
    {
        //попробуем загрузить эту информацию
        info_class* ref = info_archive::GetInst()->CreateRef(type, rid);

        Push(ref, flags);

        info_archive::GetInst()->DeleteRef(ref);
    }
    
    //сохранить содержимое энциклопедии
    void MakeSaveLoad(SavSlot& st)
    {
        if(st.IsSaving()){
            
            node_s* current = m_first;
            
            st << (current != 0);
            
            while(current){
                
                st << current->m_rid;
                st << current->m_flags;
                st << static_cast<int>(current->m_type);
                
                current = current->m_next;
                st << (current != 0);
            }
            
        }else{
            
            Reset();
            
            rid_t rid;
            bool  fnext;
            int   type;
            unsigned flags;
            
            st >> fnext;
            
            while(fnext){
                
                st >> rid;
                st >> flags;
                st >> type; 
                
                m_first = new node_s(m_first, static_cast<info_type>(type), rid, flags);
                
                st >> fnext;
            }
        }
    }

    friend class Iterator;

    //итератор для прохода по списку описаний
    class Iterator{
    public:

        Iterator(const Iterator& itor) :
          m_info(0), m_first(itor.m_first), m_flags(itor.m_flags), m_mask(itor.m_mask){}           
        
        Iterator(unsigned mask = 0, unsigned flags = 0, node_s* first = 0) :
            m_first(first), m_mask(mask), m_info(0), m_flags(flags)
        { if(m_first && !IsSuitable(m_first)) operator++(); }

        ~Iterator(){ ReleaseReference(); }
          
        Iterator& operator ++()
        {
            //отпустим ссылку перед перемоткой списка
            ReleaseReference();

            for(m_first = m_first ? m_first->m_next : m_first; m_first; m_first = m_first->m_next)
                if(IsSuitable(m_first)) return *this;        

            return *this;
        }
          
        Iterator  operator ++(int)
        { Iterator tmp = *this; operator++(); return tmp;}
          
        //операторы для удобства работы
        const info_class* operator->()
        { 
            if(!m_info) m_info = info_archive::GetInst()->CreateRef(m_first->m_type, m_first->m_rid);
            return m_info;
        }

        const info_class& operator*() { return *operator ->(); }

        Iterator& operator = (const Iterator& itor)
        {
            //отпустим ссылку
            ReleaseReference();

            m_mask = itor.m_mask;
            m_first = itor.m_first;

            return *this;
        }
          
        //сравнение на !=
        friend bool operator != (const Iterator& i1, const Iterator& i2)
        { return i1.m_first != i2.m_first; }

        friend bool operator == (const Iterator& i1, const Iterator& i2)
        { return i1.m_first == i2.m_first; }
          
    private:
        
        bool IsSuitable(const node_s* node) const
        { 
            return     (m_mask == 0  || node->m_type & m_mask)
                    && (m_flags == 0 || node->m_flags & m_flags);
        }

        void ReleaseReference()
        {
            if(m_info){
                info_archive::GetInst()->DeleteRef(m_info);
                m_info = 0;            
            }
        }
        
    private:

        info_class* m_info; 
        unsigned    m_mask;
        unsigned    m_flags;
        node_s*     m_first;
    };
    
    typedef Iterator iterator;

    iterator end() { return iterator(0, 0, 0); }
    iterator begin(unsigned mask, unsigned flags = 0) { return iterator(mask, flags, m_first); }

    //сбросить флаги записи
    void DropFlags(info_type type, const rid_t& rid, unsigned flags)
    {
        if(node_s* ptr = FindNode(type, rid)) ptr->m_flags &= ~flags;
    }

    //установить флаги записи
    void RaiseFlags(info_type type, const rid_t& rid, unsigned flags)
    {
        if(node_s* ptr = FindNode(type, rid))  ptr->m_flags |= flags;
    }
};

template<class Archive>
InfoHandbook<Archive> InfoHandbook<Archive>::m_instance;

#endif // _PUNCH_INFOUTILS_H_