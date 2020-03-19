#if !defined(__STATIC_DECODE_STREAM_H_INCLUDED_9263837130445031__)
#define __STATIC_DECODE_STREAM_H_INCLUDED_9263837130445031__

#include "DecodeStream.h"
#include "StaticSoundBuffer.h"

class Script;
class Decoder;

//=====================================================================================//
//                   class StaticDecodeStream : public DecodeStream                    //
//=====================================================================================//
class StaticDecodeStream : public DecodeStream
{
	Module * const m_module;
	const Script &m_script;
	std::auto_ptr<StaticSoundBuffer> m_buffer;

public:
	StaticDecodeStream(Module *, const Script &, std::auto_ptr<Decoder>);
	virtual ~StaticDecodeStream();

	virtual void play();
	virtual void stop();
	virtual SoundBuffer *buffer() { return m_buffer.get(); }
	virtual Decoder *getDecoder() const { return 0; }
};

#endif // !defined(__STATIC_DECODE_STREAM_H_INCLUDED_9263837130445031__)