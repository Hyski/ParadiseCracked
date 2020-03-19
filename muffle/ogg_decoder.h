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

	// ���������� ������ ������
	virtual Format getFormat() const;
	// ���������� ����� ������ � �����
	virtual unsigned decode(Buffer &);
	// ���������� ������� ������������� � ������
	virtual void reset();
	// ���������� ������� �������
	virtual unsigned getCurrentPos() const;
	// ���������� ����� ������
	virtual unsigned getLength();
	// ���������� ����� ������, ���� ���� ������ �� ������
	virtual unsigned getLengthAnyways();

	virtual std::string getInfo();
};

#endif // !defined(__OGG_DECODER_H_INCLUDED_3769856193657767__)