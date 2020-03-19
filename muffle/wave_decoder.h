#if !defined(__WAVE_DECODER_H_INCLUDED_6338595382213078__)
#define __WAVE_DECODER_H_INCLUDED_6338595382213078__

#include "decoder.h"

//=====================================================================================//
//                                  class WaveDecoder                                  //
//=====================================================================================//
class WaveDecoder : public Decoder
{
	Stream m_stream;
	ibinstream &m_in;
	Format m_format;
	unsigned m_length;
	std::ios::pos_type m_startPos;
	unsigned m_currentPos;

	void readChunkId(unsigned);
	unsigned readChunk(unsigned);

public:
	WaveDecoder(Stream stream);
	virtual ~WaveDecoder();

	// Возвращает формат потока
	virtual Format getFormat() const;
	// Декодирует кусок данных в буфер
	virtual unsigned decode(Buffer &);
	// Перемещает позицию декодирования в начало
	virtual void reset();
	// Возвращает текущую позицию
	virtual unsigned getCurrentPos() const;
	// Возвращает длину потока
	virtual unsigned getLength();
	// Возвращает длину потока, если есть способ ее узнать
	virtual unsigned getLengthAnyways();

	virtual std::string getInfo();
};

#endif // !defined(__WAVE_DECODER_H_INCLUDED_6338595382213078__)