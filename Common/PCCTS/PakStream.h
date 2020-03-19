//
// класс для чтения скриптов из пака
//

#ifndef _PUNCH_PAKSTREAM_H_
#define _PUNCH_PAKSTREAM_H_

//простенький класс - обертка для чтения из пака
class DLGPakStream: public DLGInputStream{
public:
    DLGPakStream(const char* file): 
        m_name(file), m_vfile(DataMgr::Load(file)), m_eof(false)
    {
    }

    ~DLGPakStream()
    {
        if(m_vfile) DataMgr::Release(m_name.c_str());
    }

    bool IsError() const {return m_vfile == 0;}

    int nextChar()
    {
        if(m_eof) return EOF; 
        
        if(int ret = m_vfile->ReadChar())
            return ret;
        else
            m_eof = true;

		return EOF;
    }

private:
    std::string m_name;
    VFile*      m_vfile;
    bool        m_eof;
};

#endif // _PUNCH_PAKSTREAM_H_