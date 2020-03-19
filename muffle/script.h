#if !defined(__SCRIPT_H_INCLUDED_4526468332152538__)
#define __SCRIPT_H_INCLUDED_4526468332152538__

#include "volume.h"

//=====================================================================================//
//                          class Script : public ISndScript                           //
//=====================================================================================//
class Script : public ISndScript
{
	std::string m_name;
	ISound::Channel m_channel;
	std::auto_ptr<Volume> m_volume;

	std::string m_file;	// имя файла

	bool m_repeat;		// зацикленный
	bool m_3d;			// является ли 3d

	float m_min,m_max;	// минимальная и максимальная дистанции
	float m_minAngle,m_maxAngle,m_outVol;	// параметры конуса

public:
	Script();
	Script(const std::string &name, ISound::Channel channel);
	Script(const Script &);
	~Script();

	void setFileName(const std::string &);
	void setRepeat(bool);
	void set3d(bool);
	void setDistances(float min, float max);
	void setCone(float minAngle, float maxAngle, float outVol);

	const std::string &name() const { return m_name; }

	ISound::Channel channel() const { return m_channel; }
	const std::string &fileName() const { return m_file; }
	bool repeat() const { return m_repeat; }
	bool is3d() const { return m_3d; }
	unsigned channels() const { return m_3d?2:1; }

	float minDist() const { return m_min; }
	float maxDist() const { return m_max; }

	float minConeAngle() const { return m_minAngle; }
	float maxConeAngle() const { return m_maxAngle; }
	float coneOutVolume() const { return m_outVol; }

	friend std::ostream &operator<<(std::ostream &, const Script &);
};

#endif // !defined(__SCRIPT_H_INCLUDED_4526468332152538__)