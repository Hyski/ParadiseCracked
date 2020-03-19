#if !defined(__OGG_DECODER_H_INCLUDED_3769856193657767__)
#define __OGG_DECODER_H_INCLUDED_3769856193657767__

#include "decoder.h"
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>

//=====================================================================================//
//                          class OggDecoder : public Decoder                          //
//=====================================================================================//
class OggDecoder : public Decoder
{
	Stream m_stream;
	ibinstream &m_in;
	Format m_format;
	unsigned m_length;
	OggVorbis_File m_file;
	unsigned m_currPos;
	int m_currentSection;

public:
	OggDecoder(Stream stream);
	virtual ~OggDecoder();

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

#endif // !defined(__OGG_DECODER_H_INCLUDED_3769856193657767__)