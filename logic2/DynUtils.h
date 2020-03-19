//
// ������� ��� ������������� �������� ��������
//

#ifndef _PUNCH_DYNUTILS_H_
#define _PUNCH_DYNUTILS_H_

namespace DynUtils{

    //
    //������������ �����������
    //
    class DynCtor{
    public:
        
        //������ �������� �������
        typedef void* (*new_func_t)();
        
        DynCtor(new_func_t func, const char* cls) : 
        m_func(func), m_class(cls), m_next(m_first)
        {
            m_first = this;
        }
        
        //����� ����������� �� ���������� �������������� ������
        static DynCtor* FindCtor(const char* cls)
        {
            if(m_cached && !strcmp(m_cached->m_class, cls))
                return m_cached;

            for(m_cached = m_first; m_cached && strcmp(m_cached->m_class, cls); m_cached = m_cached->m_next);
            return m_cached;   
        }
        
        //������ ��������� ������������� ������
        const char* GetClass() const { return m_class; }  
        //������� ������ ������
        void* Create(){return m_func();}
        
    private:
        
        DynCtor*    m_next;
        new_func_t  m_func;
        const char* m_class;        
        
        static DynCtor* m_first;
        static DynCtor* m_cached;
    };
    
    //
    // ������� ����. ����������\�������� �������� �� save'a
    //
    
    //���������� ������������
    template<class Obj>
    inline SavSlot& SaveObj(SavSlot& slot, const Obj* obj)
    {
        slot << std::string(obj->GetDynCtor()->GetClass());
        return slot;  
    }
    
    //�������� ��������� �� ����������� �����������
    template<class Obj>
    inline SavSlot& LoadObj(SavSlot& slot, Obj*& obj)
    {
        std::string class_str;
        slot >> class_str;
        
        DynCtor* ctor = DynCtor::FindCtor(class_str.c_str());
        obj = static_cast<Obj*>(ctor->Create());
        
        return slot;
    }
}

//
// �� ����� ������ ������������ ��� ��������� �������������
//
#define CLASS_STR(class_name)  #class_name

//
// ����� ����������� ������� ������������
//
#define DCTOR_ABS_DEF(class_name)\
virtual DynUtils::DynCtor* GetDynCtor() const = 0;\

//
// ���������� ������� ����������� ��� ������� ������
//
#define DCTOR_DEF(class_name)\
private:\
static void* CreateObj();\
static DynUtils::DynCtor m_dctor;\
public:\
virtual DynUtils::DynCtor* GetDynCtor() const;\

//
// ��������� ������� ����������� ������
//
#define DCTOR_IMP(class_name)\
void* class_name::CreateObj(){return new class_name();}\
DynUtils::DynCtor class_name::m_dctor(class_name::CreateObj, CLASS_STR(class_name));\
DynUtils::DynCtor* class_name::GetDynCtor() const {return &class_name::m_dctor;}\

#endif // _PUNCH_DYNUTILS_H_