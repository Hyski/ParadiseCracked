//
// ������ ������� ����������
//

#include "precomp.h"

#include <algorithm>
#include <iterator>

#include "SaveLoad.h"

/*
NOTE:

��������� �����:

    ������
    
        char  str[8]  ��������� �������������  "MSAV"

        unsigned   -  ����� ������    
  
        unsigned   -  ������ �������

        unsigned   -  �������� �� ������ ����� �� ������ index record
        
        unsigned   -  reserved (��� CRC)

        data chunk
    
        * * *

        data chunk
    
        index record

        * * *

        index record

    �����


data chunk ���:

        size_t   chunk size  -  ������ ����� ������ 

        char     data [chunk size]

index record ���:

        c_string ( � ����� �� ����� )

        unsigned - �������� �� ������ ����� �����. data chunk
*/

Storage::Storage(AbstractFile* file, storage_mode mode):
    m_file(file), m_mode(mode)    
{
    if(m_mode == SM_LOAD){
    
        //��������� header
        m_file->Seek(0, SEEK_SET);
        m_file->Read(&m_hdr, sizeof(header));


        //��������� index
        long  off;
        char  ch;
        std::string str;
        
        m_file->Seek(m_hdr.m_index_off, SEEK_SET);

        while( m_hdr.m_index_size -- ){

            //��������� ������
            str.clear();
            do{                
                
                m_file->Read(&ch, sizeof(char));
                if(ch) str += ch;

            }while(ch);

            //��������� ��������
            m_file->Read(&off, sizeof(off));
           
            //������������ ������
            m_index[str] = off;
        }   
    }

    if(m_mode == SM_SAVE){
        //�������� ����� ��� header
        m_file->Seek(sizeof(header), SEEK_SET);
    }
}

Storage::~Storage()
{
    if(m_mode == SM_SAVE){

        //���������� header
        m_hdr.m_index_off  = m_file->GetPos();
        m_hdr.m_index_size = m_index.size();
       
        //������� header
        m_file->Seek(0, SEEK_SET);
        m_file->Write(&m_hdr, sizeof(header));

        //��������� � ����� � ���. ���� ������ ������
        m_file->Seek(m_hdr.m_index_off, SEEK_SET);
        
        //����� ������        
        str_index_t::iterator itor = m_index.begin();
        while(itor != m_index.end()){

            m_file->Write((void*)itor->first.c_str(), itor->first.size()+1);
            m_file->Write(&itor->second, sizeof(long));

            itor++;
        }
    }
}

void Storage::SaveData(const char* cell_name, const data_buff_t& data)
{
    std::string cell = cell_name;

    if(!IsSaving() || m_index.end() != m_index.find(cell))
        throw CASUS("Storage: ��������� ������� ����������!");
    
    //���������� � ������
    m_index[cell] = m_file->GetPos();
    
    //����� ������
    size_t size = data.size();
    m_file->Write(&size, sizeof(size));

    data_buff_t::value_type val;

    //����� �����
    for(size_t i = 0; i < size; i++){
        val = data[i];
        m_file->Write(&val, sizeof(val));
    }
}

void Storage::LoadData(const char* cell_name, data_buff_t& data)
{
    std::string cell = cell_name;
    str_index_t::iterator itor = m_index.find(cell);

    if(IsSaving() || m_index.end() == itor)
        throw CASUS("Storage: ��������� ������� ��������!");
     
    //���������� ��������� �� ������
    m_file->Seek(itor->second, SEEK_SET);

    //������� ���-�� ������
    size_t size;
    m_file->Read(&size, sizeof(size));
    
    //������� ������
    data_buff_t::value_type val;

    data.clear();
    data.reserve(size);
    
    while(size--){
        m_file->Read(&val, sizeof(val));
        data.push_back(val);
    }
}

SavSlot::SavSlot(Storage* st, const char* name, size_t size):
     m_strg(st), m_name(name), m_fsave(st->IsSaving()),m_cur(0)
{     
    //���� �������� �� ���������� ������
    if(!m_fsave)
        m_strg->LoadData(m_name.c_str(), m_data);
    else
        m_data.reserve(size); 
}

SavSlot::~SavSlot()
{
    //���� save ��������� ������
    if(m_fsave) m_strg->SaveData(m_name.c_str(), m_data);
}
    
void SavSlot::Save(void* ptr, size_t size)
{   
    if(!m_fsave) throw CASUS("SavCell: ������ �� ����� ��������!");

    unsigned char* arr = static_cast<unsigned char*>(ptr);
    std::copy(arr, arr+size, std::back_inserter(m_data));
}

void SavSlot::Load(void* ptr, size_t size)
{
    if(m_fsave) throw CASUS("SavCell: ������ ������ �� ����� ����������!");
//Grom
		if(m_data.size()<m_cur+size)
			throw CASUS("LoadCell: ������ ������ ������!");

    unsigned char* arr = static_cast<unsigned char*>(ptr);
    std::copy(m_data.begin() + m_cur, m_data.begin() + m_cur + size, arr);
    m_cur += size; 
}

/////////////////////////////////////////////////////////////////////////
//////////////////////    class Storage::header    //////////////////////
/////////////////////////////////////////////////////////////////////////
extern unsigned g_SaveVersion;
Storage::header::header() : m_version(g_SaveVersion),m_index_size(0),m_index_off(0),
							m_reserved(0)
{ 
	strcpy(m_id_str,"MSAV"); 
}