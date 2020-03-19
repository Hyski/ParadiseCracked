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

#endif // !defined(__WAVE_DECODER_H_INCLUDED_6338595382213078__)