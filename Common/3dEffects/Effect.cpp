/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������ �������� � ����� ��������� ��������
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "Effect.h"
#include "../../sound/ISound.h"

#if defined (EFFECTS_DEBUG) //for log
CLog LOG_CLASS;
#endif // for log

extern float AmbientVol;
extern float EffectsVol;

//
// ����� FixedSound - ��� �������, ������� ����������� ����������� ����
//
FixedSound::FixedSound(const std::string& Name, const point3& Target)
{
	static const ISndScript *script = 0;
	if (!script) script = ISound::instance()->getScript("amb_onetime");

	std::string filename("sounds/");
	filename += Name + ".wav";
	ISndEmitter *emitter = ISound::instance()->createEmitter(script,filename.c_str());
	emitter->setPosition(Target);
	emitter->play();
	emitter->Release();
}


//
// ����� ActiveSound - ��� �������, ������� ����������� ��������� ����
//

// ���� ���������, ��� ��� Update ������ �������� ���������� � ��������
void ActiveSound::Update(const point3& Position, const point3& Velocity)
{
	pos = Position;
	vel = Velocity;
	emitter->setPosition(pos);
	emitter->setVelocity(vel);
}

ActiveSound::ActiveSound(const std::string& Name)
{
	static const ISndScript *script = 0;
	if (!script) script = ISound::instance()->getScript("looping");

	std::string filename("sounds/");
	filename += Name + ".wav";
	emitter = ISound::instance()->createEmitter(script,filename.c_str());
	emitter->play();
}

ActiveSound::~ActiveSound()
{
	emitter->stop();
	emitter->Release();
}

///////////////////////////////////////////////////////////////////////////////
//
// ������������ ���� ��� ������ ��������
//
///////////////////////////////////////////////////////////////////////////////
namespace EffectUtility
{
	// �������� ��������� ����� � ��������� �� 0 �� 1 (float)
	float EffectUtility::get0F()
	{
		return (static_cast<float>(rand()))*0.000030518509f;
	}
	// �������� ��������� ����� � ��������� �� 0 �� f (float)
	float EffectUtility::get0F(float f)
	{
		return (static_cast<float>(rand()))*0.000030518509f*f;
	}

	
	// �������� ��������� ����� � ��������� �� -1 �� 1
	float EffectUtility::get_FF()
	{
		return (1 - 2.0f*(static_cast<float>(rand()))*0.000030518509f);
	}
	// �������� ��������� ����� � ��������� �� -f �� f
	float EffectUtility::get_FF(float f)
	{
		return (1 - 2.0f*(static_cast<float>(rand()))*0.000030518509f)*f;
	}
};

