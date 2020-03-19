#include "precomp.h"
#include "securom.h"
#include "../logic2/logicdefs.h"
#include "../logic2/observer.h"
#include "../logic2/spawn.h"
#include "../logic2/dirtylinks.h"




class SecuROMTrigger
{
public:
	enum IDENTIFICATION { /*REG_01, REG_02, REG_03, STK_01, STK_02, STK_03, */STK_04};
	enum CHECK_ACTION {CD_00, CD_01, CD_02, AL_01, AL_02, EX_01, EX_02};
	enum RESPONSE {RET_01, RET_02, RET_03, EVT_01, EVT_02/*, RGY_01*/};

	inline SecuROMTrigger(IDENTIFICATION id, CHECK_ACTION ca, RESPONSE resp,
		int TVN1, int TVN2, int TVN3
		)
		:tvn1(TVN1), tvn2(TVN2), tvn3(TVN3),
		m_id(id), m_ca(ca), m_resp(resp)
	{
	}

	~SecuROMTrigger()
	{
		if(m_ev.get())
			CloseHandle(*m_ev);

	}

	
	inline void Prepare()
	{
		if(m_resp != EVT_01 && m_resp != EVT_02) return;

		std::ostringstream o;
		o<<"EV_"<<std::setw(4)<<std::setfill('0')<<tvn3;

		m_ev.reset(new HANDLE(
			CreateEvent(NULL, TRUE, (m_resp==EVT_01)?FALSE:TRUE, o.str().c_str())
			)
			);
	}
	inline bool Event()
	{
		if(m_resp != EVT_01 && m_resp != EVT_02) return false;

		std::ostringstream o;
		o<<"EV_"<<std::setw(4)<<std::setfill('0')<<tvn3;
		HANDLE h = OpenEvent(EVENT_ALL_ACCESS, FALSE, o.str().c_str()); 
		if(h==NULL) return false;
		DWORD ret = WaitForSingleObject(h,30);
		CloseHandle(h);
		if(ret == WAIT_OBJECT_0)
			return true;
		return false;
	}
	inline int Get1(){return tvn1;};
	inline int Get2(){return tvn2;};
	inline int Get3(){return tvn3;};
	inline void SetVal(int val){m_res.reset(new int(val));};

	inline bool Check()
	{
		if(m_resp==RET_01 && m_res.get() && *m_res==0) return true;
		if(m_resp==RET_02 && m_res.get() && *m_res==1) return true;
		if(m_resp==RET_03 && m_res.get() && *m_res==tvn3) return true;

		if(!m_res.get()) return true;

		if(m_resp == EVT_01 || m_resp == EVT_02)
		{
		std::ostringstream o;
		o<<"EV_"<<std::setw(4)<<std::setfill('0')<<tvn3;
		HANDLE h = OpenEvent(EVENT_ALL_ACCESS, FALSE, o.str().c_str()); 
		if(h==NULL) return false;
		DWORD ret = WaitForSingleObject(h,30);
		if((ret == WAIT_OBJECT_0 && m_resp==EVT_01) || (ret==WAIT_TIMEOUT && m_resp==EVT_02))
		{
			if(m_resp==EVT_01)
				ResetEvent(h);
			else 
				SetEvent(h);
			CloseHandle(h);
			m_res.reset();
			return true;
		}
		CloseHandle(h);
		return false;
		}
		return false;
		

	}

private:
	int				tvn1,tvn2,tvn3;
	IDENTIFICATION  m_id;
	CHECK_ACTION	m_ca;
	RESPONSE		m_resp;

	std::auto_ptr<int> m_res;
	std::auto_ptr<HANDLE> m_ev;

};
#define SecuRomActivationStart(trigger) {\
		int _tvn1(trigger.Get1()),_tvn2(trigger.Get2()),_tvn3(trigger.Get3());\
			_asm push _tvn3\
			_asm push _tvn2\
			_asm {push _tvn1};\
}

#define SecuRomActivationEnd(trigger) {\
		int res;\
		_asm	pop edx\
		_asm 	pop ecx\
		_asm 	pop ebx\
		_asm 	{mov res,eax};\
		trigger.SetVal(res);\
}
















/*
	SecuROMTrigger tr(SecuROMTrigger::STK_04, SecuROMTrigger::CD_02, SecuROMTrigger::EVT_01,
						1234,5678,876);
	tr.Prepare();
	SecuRomActivationStart(tr);
	_asm
	{
		mov eax,8976
	}
	SecuRomActivationEnd(tr);

	bool ret = tr.Check();
*/

std::auto_ptr<TriggerHolder> Holder;

#if !defined(USE_SECUROM_TRIGGERS)
class TriggerHolderImp: public TriggerHolder
{
public:
	TriggerHolderImp(){};
	void Init(){};
	void Shut()
	{
		Holder.reset();
	};
};



#else


//CLog ll;
#define log /##/ll["security.log"]
#define MY_THROW throw (*m_casus)
class TriggerHolderImp: public TriggerHolder,
                        private SpawnObserver
{
private:
	std::auto_ptr<SecuROMTrigger> CD_checker;
	std::auto_ptr<SecuROMTrigger> Checker1;
	std::auto_ptr<SecuROMTrigger> Checker2;
	std::auto_ptr<CommonCasus> m_casus;
public:
	TriggerHolderImp()
	{
		log("starting..\n");
		CD_checker.reset(new SecuROMTrigger(SecuROMTrigger::STK_04, SecuROMTrigger::CD_02, SecuROMTrigger::RET_03,5163,35,8462));
		Checker1.reset(new SecuROMTrigger(SecuROMTrigger::STK_04, SecuROMTrigger::AL_02, SecuROMTrigger::EVT_01,385,17,253));
		Checker2.reset(new SecuROMTrigger(SecuROMTrigger::STK_04, SecuROMTrigger::EX_02, SecuROMTrigger::EVT_02,	6582,28,15));
		m_casus.reset(new CASUS(DirtyLinks::GetStrRes("mainmenu_insert_cd")));
		log("preparing triggers..\n");
		
		CD_checker->Prepare();
		Checker1->Prepare();
		log("checker1 Event:%s",Checker1->Event()?"true":"false");
		Checker2->Prepare();
		log("checker2 Event:%s",Checker2->Event()?"true":"false");
	}
	void Init()
	{
		//Подпишемся на всякие события...
		log("init..\n");
	    Spawner::GetInst()->Attach(this, ET_PREPARE_SPAWN);
	    Spawner::GetInst()->Attach(this, ET_EXIT_LEVEL);
	    Spawner::GetInst()->Attach(this, ET_PHASE_CHANGE);
	    Spawner::GetInst()->Attach(this, ET_FINISH_SPAWN);
		
		log("init..done\n");
		

	}
	
	void Shut() 
	{
		log("closing..\n");
		//отпишемся от событий....
		Spawner::GetInst()->Detach(this);
		Holder.reset();
		log("closed.\n");
	};

private:
	void Update(SpawnObserver::subject_t subj, SpawnObserver::event_t event, SpawnObserver::info_t info)
	{   
		log("updating...");

		switch(event){
		case ET_EXIT_LEVEL:
			log("EXIT_LEVEL\n");
			break;
			
		case ET_PREPARE_SPAWN:
			log("PREPARE_LEVEL\n");
			SecuRomActivationStart((*CD_checker));
			Sleep(12);
			SecuRomActivationEnd((*CD_checker));
			log("checked CD_01\n");
			if(!CD_checker->Check())
			{
				log("check failed!\n");
				MY_THROW;
				break;
			}
				log("check ok.\n");
			break;

		case ET_PHASE_CHANGE:
			{
			log("PHASE_CHANGE\n");
				switch(rand()%2)
				{
				case 0:
					log("checking Checker01\n");
					SecuRomActivationStart((*Checker1));
					GetActiveWindow();
					SecuRomActivationEnd((*Checker1));
					log("Event:%s",Checker1->Event()?"true":"false");
					if(!Checker1->Check()) 
						log("]]] check failed!\n");
					;
					break;
				case 1:
					log("checking Checker02\n");
					SecuRomActivationStart((*Checker2));
					IsBadCodePtr((FARPROC)GetInst);
					SecuRomActivationEnd((*Checker2));
					log("Event:%s",Checker2->Event()?"true":"false");
					if(!Checker2->Check()) 
						log("]]] check failed!\n");
					;
					break;
				}
			}
			break;
		case ET_FINISH_SPAWN:
			log("FINISH_RESPAWN\n");
			if(!Checker1->Check())
			{
				log("checker1 failed\n");
			MY_THROW;
			}

			if(!Checker2->Check())
			{
				log("checker2 failed!\n");
				MY_THROW;
			}
			break;
		}
	}
	
	
};
#endif

TriggerHolder* TriggerHolder::GetInst()
{
	if(!Holder.get())
	{
		Holder.reset(new TriggerHolderImp);
	}
	return Holder.get();
}


