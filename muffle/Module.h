#if !defined(__MODULE_H_INCLUDED_5299984892160361__)
#define __MODULE_H_INCLUDED_5299984892160361__

class DirectSound;
class PrimaryBuffer;
class ScriptMgr;
class DecodeMgr;
class NotifyThread;
class DecodeThread;
class VolumeMgr;
class Muffle;
class FileMgmt;
class ThemeManager;

//=====================================================================================//
//                                    struct Module                                    //
//=====================================================================================//
class Module
{
	friend class Muffle;

	Muffle * m_muffle;
	NotifyThread * m_notifyThread;
	FileMgmt * m_fileMgmt;
	DirectSound * m_dsound;
	PrimaryBuffer * m_primary;
	ScriptMgr * m_scriptMgr;
	DecodeMgr * m_decodeMgr;
	DecodeThread * m_decodeThread;
	VolumeMgr * m_volumeMgr;
	ci_VFileSystem * m_fileSystem;
	ThemeManager * m_themeManager;

public:
	DirectSound * getDirectSound() const { return m_dsound; }
	PrimaryBuffer * getPrimaryBuffer() const { return m_primary; }
	ScriptMgr * getScriptMgr() const { return m_scriptMgr; }
	DecodeMgr * getDecodeMgr() const { return m_decodeMgr; }
	NotifyThread * getNotifyThread() const { return m_notifyThread; }
	DecodeThread * getDecodeThread() const { return m_decodeThread; }
	VolumeMgr * getVolumeMgr() const { return m_volumeMgr; }
	Muffle * getMuffle() const { return m_muffle; }
	FileMgmt * getFileMgmt() const { return m_fileMgmt; }
	ci_VFileSystem * getFileSystem() const { return m_fileSystem; }
	ThemeManager *getThemeManager() const { return m_themeManager; }
};

#endif // !defined(__MODULE_H_INCLUDED_5299984892160361__)