#if !defined(__REAL_FILE_H_INCLUDED_7552370787048739__)
#define __REAL_FILE_H_INCLUDED_7552370787048739__

class FileMgmt;

//=====================================================================================//
//                                   class RealFile                                    //
//=====================================================================================//
class RealFile : private noncopyable
{
	unsigned m_test;
	std::string m_name;
	long m_refCtr;
	unsigned m_length;
	unsigned m_lastUseTime;
	ci_VFile *m_file;

public:
	RealFile(ci_VFileSystem *, const std::string &, unsigned test);
	~RealFile();

	void addRef();
	void release();

	const char *getData();
	unsigned getLength();
	const std::string &name() const { return m_name; }

	std::auto_ptr<std::istream> getStream();

	unsigned getRefCounter() const { return m_refCtr; }
	unsigned getLastUseTime() const { return m_lastUseTime; }
};

#endif // !defined(__REAL_FILE_H_INCLUDED_7552370787048739__)