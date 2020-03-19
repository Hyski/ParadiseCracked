#if !defined(__DECODE_STREAM_H_INCLUDED_5800534574304575__)
#define __DECODE_STREAM_H_INCLUDED_5800534574304575__

class SoundBuffer;
class Decoder;

//=====================================================================================//
//                                 class DecodeStream                                  //
//=====================================================================================//
class DecodeStream : private noncopyable
{
public:
	virtual ~DecodeStream() {}
	virtual void play() = 0;
	virtual void stop() = 0;
	virtual SoundBuffer *buffer() = 0;
	virtual Decoder *getDecoder() const = 0;
};

#endif // !defined(__DECODE_STREAM_H_INCLUDED_5800534574304575__)