#include "precomp.h"
#include "script.h"
#include "volume.h"

//=====================================================================================//
//                                  Script::Script()                                   //
//=====================================================================================//
Script::Script()
{
}

//=====================================================================================//
//                                  Script::Script()                                   //
//=====================================================================================//
Script::Script(const std::string &name, ISound::Channel channel)
:	m_name(name),
	m_channel(channel)
{
	m_minAngle = 360.0f;
	m_maxAngle = 360.0f;
	m_outVol = 1.0f;
	m_min = 5.0f;
	m_max = 100.0f;

	switch(channel)
	{
		case ISound::cThemes:
			m_3d = false;
			m_repeat = true;
			break;
		case ISound::cSpeech:
			m_3d = false;
			m_repeat = false;
			break;
		case ISound::cEffects:
			m_3d = true;
			m_repeat = false;
			break;
		case ISound::cMenu:
			m_3d = true;
			m_repeat = false;
			break;
		case ISound::cAmbient:
			m_3d = true;
			m_repeat = true;
			break;
		case ISound::cDebug:
			m_3d = true;
			m_repeat = false;
			break;
	}
}

//=====================================================================================//
//                                  Script::Script()                                   //
//=====================================================================================//
Script::Script(const Script &script)
:	m_name(script.m_name),
	m_channel(script.m_channel),
	m_file(script.m_file),
	m_repeat(script.m_repeat),
	m_3d(script.m_3d),
	m_min(script.m_min),
	m_max(script.m_max),
	m_minAngle(script.m_minAngle),
	m_maxAngle(script.m_maxAngle),
	m_outVol(script.m_outVol)
{
}

//=====================================================================================//
//                                  Script::~Script()                                  //
//=====================================================================================//
Script::~Script()
{
}

//=====================================================================================//
//                             void Script::setFileName()                              //
//=====================================================================================//
void Script::setFileName(const std::string &name)
{
	m_file = name;
}

//=====================================================================================//
//                              void Script::setRepeat()                               //
//=====================================================================================//
void Script::setRepeat(bool repeat)
{
	m_repeat = repeat;
}

//=====================================================================================//
//                                void Script::set3d()                                 //
//=====================================================================================//
void Script::set3d(bool b3d)
{
	m_3d = b3d;
}

//=====================================================================================//
//                             void Script::setDistances()                             //
//=====================================================================================//
void Script::setDistances(float min, float max)
{
	m_min = min;
	m_max = max;
}

//=====================================================================================//
//                               void Script::setCone()                                //
//=====================================================================================//
void Script::setCone(float minAngle, float maxAngle, float outVol)
{
	m_minAngle = minAngle;
	m_maxAngle = maxAngle;
	m_outVol = outVol;
}

//=====================================================================================//
//                             std::ostream &operator<<()                              //
//=====================================================================================//
std::ostream &operator<<(std::ostream &stream, const Script &script)
{
	stream << "Информация о скрипте [" << script.m_name << "]:\n";
	stream << "\tфайл: \"" << script.m_file << "\"\n";
	stream << "\tколичество повторов: ";
	if(script.m_repeat)
	{
		stream << "бесконечно";
	}
	else
	{
		stream << script.m_repeat;
	}
	stream << "\n\t3d: " << ((script.m_3d)?"включено":"выключено") << "\n";
	if(script.m_3d)
	{
		stream << "\tрасстояния: (" << script.m_min << "," << script.m_max << ")\n";
		stream << "\tконус: (" << script.m_minAngle << "," << script.m_maxAngle
			<< script.m_outVol << ")\n";
	}
	return stream;
}