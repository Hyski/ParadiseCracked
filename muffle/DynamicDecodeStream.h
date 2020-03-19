#if !defined(__DYNAMIC_DECODE_STREAM_H_INCLUDED_6847767170298212__)
#define __DYNAMIC_DECODE_STREAM_H_INCLUDED_6847767170298212__

#include "Buffer.h"
#include "NotifyReceiver.h"
#include "DecodeStream.h"
#include "DynamicSoundBuffer.h"

class Decoder;
class Script;

//=====================================================================================//
//           class DynamicDecodeStream : private Buffer, public DecodeStream           //
//=====================================================================================//
class DynamicDecodeStream : private Buffer, public DecodeStream
{
	Module * const m_module;
	const Script &m_script;
	std::auto_ptr<Decoder> m_decoder;
	std::auto_ptr<DynamicSoundBuffer> m_buffer;

	WndHandle m_firstHalf;
	WndHandle m_secondHalf;

	typedef NotifyAdaptor<DynamicDecodeStream> Adaptor_t;
	Adaptor_t m_decodeAdaptor;
	Adaptor_t m_stopAdaptor;

	bool m_stopActivated;

	void onDecode();
	void onStop();

	unsigned m_currentFrame;
	unsigned m_samplesLeft;

public:
	DynamicDecodeStream(Module *, const Script &, std::auto_ptr<Decoder>);
	virtual ~DynamicDecodeStream();

	virtual Format getFormat() const;
	virtual unsigned getAvailSize() const;
	virtual void feed(const short *data, unsigned count);

	virtual void play();
	virtual void stop();
	virtual SoundBuffer *buffer() { return m_buffer.get(); }

	Decoder *getDecoder() const { return m_decoder.get(); }
	DynamicSoundBuffer *getBuffer() const { return m_buffer.get(); }
	const Script &getScript() const { return m_script; }

	void activateStop();
};


#endif // !defined(__DYNAMIC_DECODE_STREAM_H_INCLUDED_6847767170298212__)