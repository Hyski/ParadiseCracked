#include "precomp.h"
#include "muffle.h"
#include "kernel.h"
#include "scriptmgr.h"
#include "decodemgr.h"
#include "decoder.h"
#include "Emitter.h"
#include "NotifyThread.h"
#include "DecodeThread.h"
#include "Script.h"
#include "VolumeMgr.h"
#include "PrimaryBuffer.h"
#include "ThemeSession.h"
#include "FileMgmt.h"
#include "ThemeManager.h"
#include "SoundBuffer.h"

namespace
{
	Muffle *muffle = 0;
}

//=====================================================================================//
//                                  Muffle::Muffle()                                   //
//=====================================================================================//
Muffle::Muffle(SndServices *services)
:	m_wnd((HWND)services->m_Wnd),
	m_kernel(new Kernel(DSDEVID_DefaultPlayback,m_wnd)),
	m_notifyThread(new NotifyThread),
	m_fileMgmt(new FileMgmt(m_notifyThread.get(),services->m_FileSystem)),
	m_scriptMgr(new ScriptMgr(services->m_FileSystem)),
	m_decodeMgr(new DecodeMgr),
	m_decodeThread(new DecodeThread),
	m_volumeMgr(new VolumeMgr),
	m_themeManager(new ThemeManager(&m_module)),
	m_services(services)
{
	muffle = this;

	m_module.m_muffle = this;
	m_module.m_decodeMgr = m_decodeMgr.get();
	m_module.m_dsound = &m_kernel->dsound();
	m_module.m_primary = &m_kernel->primary();
	m_module.m_scriptMgr = m_scriptMgr.get();
	m_module.m_notifyThread = m_notifyThread.get();
	m_module.m_decodeThread = m_decodeThread.get();
	m_module.m_volumeMgr = m_volumeMgr.get();
	m_module.m_fileSystem = services->m_FileSystem;
	m_module.m_fileMgmt = m_fileMgmt.get();
	m_module.m_themeManager = m_themeManager.get();

	m_volumes[cMaster] = m_volumeMgr->addVolume();
	m_volumes[cThemes] = m_volumeMgr->addVolume(m_volumes[cMaster].get());
	m_volumes[cSpeech] = m_volumeMgr->addVolume(m_volumes[cMaster].get());
	m_volumes[cEffects] = m_volumeMgr->addVolume(m_volumes[cMaster].get());
	m_volumes[cMenu] = m_volumeMgr->addVolume(m_volumes[cMaster].get());
	m_volumes[cAmbient] = m_volumeMgr->addVolume(m_volumes[cMaster].get());
	m_volumes[cDebug] = m_volumeMgr->addVolume(m_volumes[cMaster].get());

	setCamera(snd_vector(0.0f,0.0f,1.0f),snd_vector(0.0f,1.0f,0.0f),
		snd_vector(1.0f,0.0f,0.0f), snd_vector(0.0f,0.0f,0.0f));

	SAFE_CALL(m_kernel->primary().getListener()->SetRolloffFactor(
		5.0f,//1.0f/0.5f,
		DS3D_IMMEDIATE));
}

//=====================================================================================//
//                                  Muffle::~Muffle()                                  //
//=====================================================================================//
Muffle::~Muffle()
{
	//MLL_MAKE_LOG("sound_shutdown.log","shutdown started.");

	  /*
	¬Õ»Ã¿Õ»≈: Grom did this shit.
	  while(Emitter::getEmittersCount()) 
		Sleep(0);
		*/

	m_themeManager.reset();
	Emitter::ClearAll();

	int cnt=0;
	while(Emitter::getEmittersCount() && (cnt<100) ) 
	{
		Sleep(40);// ∆‰ÂÏ Á‡ÚÛı‡ÌËˇ ÏÛÁ˚ÍË
		++cnt;
	}

	if(Emitter::getEmittersCount())
		Emitter::ClearAll();
	/*
	m_fileMgmt.release();
	m_notifyThread.release();

	for(int i=0; i<cTerminator; i++)
		m_volumes[i].release();
	
	m_volumeMgr.release();
	m_decodeThread.release();
	m_decodeMgr.release();
	m_scriptMgr.release();
	m_kernel.release();*/

	muffle = 0;
	//MLL_MAKE_LOG("sound_shutdown.log","shutdown finished.");
}

//=====================================================================================//
//                             ISound *ISound::instance()                              //
//=====================================================================================//
ISound *ISound::instance()
{
	return muffle;
}

//=====================================================================================//
//                             bool ISound::initInstance()                             //
//=====================================================================================//
void ISound::initInstance(SndServices *svc)
{
	if(muffle != 0) throw sound_error("Sound already initialized");
	new Muffle(svc);
}

//=====================================================================================//
//                             void ISound::shutInstance()                             //
//=====================================================================================//
void ISound::shutInstance()
{
	if(muffle == 0) throw sound_error("Sound hasn't been initialized");
	delete muffle;
}

//=====================================================================================//
//                            bool ISound::isInitialized()                             //
//=====================================================================================//
bool ISound::isInitialized()
{
	return muffle != 0;
}

//=====================================================================================//
//                        ISndEmitter *Muffle::createEmitter()                         //
//=====================================================================================//
ISndEmitter *Muffle::createEmitter(const char *scriptName)
{
	return new Emitter(&m_module,m_module.getScriptMgr()->get(scriptName));
}

//=====================================================================================//
//                        ISndEmitter *Muffle::createEmitter()                         //
//=====================================================================================//
ISndEmitter *Muffle::createEmitter(const ISndScript *script)
{
	return new Emitter(&m_module,*static_cast<const Script*>(script));
}

//=====================================================================================//
//                        ISndEmitter *Muffle::createEmitter()                         //
//=====================================================================================//
ISndEmitter *Muffle::createEmitter(const ISndScript *script, const char *wave)
{
	return new Emitter(&m_module,*static_cast<const Script*>(script),wave);
}

//=====================================================================================//
//                              void Muffle::emitSound()                               //
//=====================================================================================//
void Muffle::emitSound(const ISndScript *script, const char *wave)
{
	Emitter *emitter = new Emitter(&m_module,*static_cast<const Script*>(script),wave);
	emitter->play();
	emitter->setSelfDestruct();
}

//=====================================================================================//
//                              void Muffle::emitSound()                               //
//=====================================================================================//
void Muffle::emitSound(const ISndScript *script)
{
	Emitter *emitter = new Emitter(&m_module,*static_cast<const Script*>(script));
	emitter->play();
	emitter->setSelfDestruct();
}

//=====================================================================================//
//                              void Muffle::emitSound()                               //
//=====================================================================================//
void Muffle::emitSound(const char *scriptName)
{
	Emitter *emitter = new Emitter(&m_module,m_module.getScriptMgr()->get(scriptName));
	emitter->play();
	emitter->setSelfDestruct();
}

//=====================================================================================//
//                           ISndScript *Muffle::getScript()                           //
//=====================================================================================//
const ISndScript *Muffle::getScript(const char *script)
{
	return &m_module.getScriptMgr()->get(script);
}

//=====================================================================================//
//                              void Muffle::setVolume()                               //
//=====================================================================================//
void Muffle::setVolume(Channel c, float vol)
{
	getVolumeObject(c)->change(vol);
}

//=====================================================================================//
//                             void Muffle::muteChannel()                              //
//=====================================================================================//
void Muffle::muteChannel(Channel c, bool mut)
{
	getVolumeObject(c)->mute(mut);
}

//=====================================================================================//
//                              void Muffle::setCamera()                               //
//=====================================================================================//
void Muffle::setCamera(const snd_vector &dir,
					   const snd_vector &up,
					   const snd_vector &right,
					   const snd_vector &origin)
{
	IDirectSound3DListener8 *listener = m_kernel->primary().getListener();
	m_dir = snd_vector(-dir.x,-dir.y,dir.z);
	m_up = snd_vector(0.0f,0.0f,1.0f);//up;
	m_right = right;
	m_origin = snd_vector(origin.x,origin.y,origin.z);

	listener->SetPosition(m_origin.x, m_origin.y, m_origin.z, DS3D_DEFERRED);
	listener->SetOrientation(m_dir.x, m_dir.y, m_dir.z, m_up.x, m_up.y, m_up.z, DS3D_DEFERRED);
	listener->CommitDeferredSettings();
}

//=====================================================================================//
//                      ISndThemeSession *Muffle::beginSession()                       //
//=====================================================================================//
ISndThemeSession *Muffle::beginSession()
{
	ThemeSession *session = new ThemeSession(&m_module);
	m_themeManager->addSession(session);
	return session;
}

#include "Buffer.h"

//=====================================================================================//
//                                void Muffle::manage()                                //
//=====================================================================================//
void Muffle::manage()
{
#if defined(__MUFFLE_TEST__)
	{
		std::ostringstream sstr;
		sstr << "Files: " << m_module.getFileMgmt()->getFileCount() << " occupying "
			<< m_module.getFileMgmt()->getTotalSize()/1024 << "K memory";
		m_services->dbg_printf(200,300,sstr.str().c_str());
	}
	{
		std::ostringstream sstr;
		sstr << "Emitters count: " << Emitter::getEmittersCount();
		m_services->dbg_printf(200,320,sstr.str().c_str());
	}
	{
		std::ostringstream sstr;
		sstr << "Event count: " << m_module.getNotifyThread()->getEventCount();
		m_services->dbg_printf(200,340,sstr.str().c_str());
	}
	{
		std::ostringstream sstr;
		sstr << "NotifyCluster count: " << m_module.getNotifyThread()->getThreadCount();
		m_services->dbg_printf(200,360,sstr.str().c_str());
	}
	{
		std::ostringstream sstr;
		sstr << "Decoders count: " << Decoder::getInstanceCount();
		m_services->dbg_printf(200,380,sstr.str().c_str());
	}
	{
		std::ostringstream sstr;
		sstr << "Buffers count: " << Buffer::getInstanceCount();
		m_services->dbg_printf(200,400,sstr.str().c_str());
	}
	{
		std::ostringstream sstr;
		sstr << "SoundBuffer: soft(" << SoundBuffer::softCount() << ") hard(" << SoundBuffer::hardCount() << ")";
		m_services->dbg_printf(200,420,sstr.str().c_str());
	}
	{
		std::ostringstream sstr;
		sstr << "BuildDate: " << __DATE__" "__TIME__;
		m_services->dbg_printf(200,440,sstr.str().c_str());
	}
#endif
}