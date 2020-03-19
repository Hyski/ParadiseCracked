/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: классы эффектов и класс менеджера эффектов
                                                                                
                                                                                
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
// класс FixedSound - для эффекта, который проигрывает неподвижный звук
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
// класс ActiveSound - для эффекта, который проигрывает подвижный звук
//

// звук подвижный, так что Update должна изменить координаты и скорость
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
// пространство имен для утилит эффектов
//
///////////////////////////////////////////////////////////////////////////////
namespace EffectUtility
{
	// получить случайное число в диапазоне от 0 до 1 (float)
	float EffectUtility::get0F()
	{
		return (static_cast<float>(rand()))*0.000030518509f;
	}
	// получить случайное число в диапазоне от 0 до f (float)
	float EffectUtility::get0F(float f)
	{
		return (static_cast<float>(rand()))*0.000030518509f*f;
	}

	
	// получить случайное число в диапазоне от -1 до 1
	float EffectUtility::get_FF()
	{
		return (1 - 2.0f*(static_cast<float>(rand()))*0.000030518509f);
	}
	// получить случайное число в диапазоне от -f до f
	float EffectUtility::get_FF(float f)
	{
		return (1 - 2.0f*(static_cast<float>(rand()))*0.000030518509f)*f;
	}
};

