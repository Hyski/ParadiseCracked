#if !defined(__MUFFLE_H_INCLUDED_8093570542662288__)
#define __MUFFLE_H_INCLUDED_8093570542662288__

#include "volume.h"

class Kernel;
class ScriptMgr;
class DecodeMgr;
class NotifyThread;
class DecodeThread;
class VolumeMgr;
class Volume;

//=====================================================================================//
//                                    class Muffle                                     //
//=====================================================================================//
class Muffle : public ISound, private noncopyable
{
	HWND m_wnd;
	ci_VFileSystem *m_fs;
	std::auto_ptr<Kernel> m_kernel;
	std::auto_ptr<NotifyThread> m_notifyThread;
	std::auto_ptr<FileMgmt> m_fileMgmt;
	std::auto_ptr<ScriptMgr> m_scriptMgr;
	std::auto_ptr<DecodeMgr> m_decodeMgr;
	std::auto_ptr<DecodeThread> m_decodeThread;
	std::auto_ptr<VolumeMgr> m_volumeMgr;
	std::auto_ptr<ThemeManager> m_themeManager;

	SndServices *m_services;
	Module m_module; // ���������� ��� ���� ��������

	std::auto_ptr<Volume> m_volumes[cTerminator];

	snd_vector m_dir, m_up, m_right, m_origin;

public:
	Muffle(SndServices *services);
	~Muffle();

	virtual void manage();

	// ������ ����������� ������
	virtual ISndThemeSession *beginSession();

	// ������� �������� �����
	virtual ISndEmitter *createEmitter(const ISndScript *, const char *waveFile);
	virtual ISndEmitter *createEmitter(const ISndScript *);
	virtual ISndEmitter *createEmitter(const char *scriptName);

	// ��������� ����. ����� ������� ����� � ������� ���� ������� 
	// ��� ����� ����������������� ������. ���� ���� ��������, �� ��
	// ����� ���� ���������� ������ �� ����� ���������� ������� �
	// ������� ������� clean()
	virtual void emitSound(const ISndScript *, const char *waveFile);
	virtual void emitSound(const ISndScript *);
	virtual void emitSound(const char *scriptName);

	// ���������� ��������� �� ������
	virtual const ISndScript *getScript(const char *scriptName);

	// ���������� ��������� ����� ��� ������������� ������
	virtual void setVolume(Channel, float volume);

	// �������� ��������� ����� ��� ������������� ������
	virtual float getVolume(Channel c) const { return getVolumeObject(c)->get(); }

	// ��������/��������� ������������ �����
	virtual void muteChannel(Channel c, bool mut);

	// �������� ������� ������
	virtual void setCamera(const snd_vector &dir,
						   const snd_vector &up,
						   const snd_vector &right,
						   const snd_vector &origin);

	Volume *getVolumeObject(Channel c) const { return m_volumes[c].get(); }
};

#endif // !defined(__MUFFLE_H_INCLUDED_8093570542662288__)