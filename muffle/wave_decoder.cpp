#include "precomp.h"
#include "wave_decoder.h"
#include "buffer.h"

namespace
{
#define MKCHUNKNAME(D,C,B,A)	((unsigned(A)<<24)+(unsigned(B)<<16)+(unsigned(C)<<8)+(unsigned(D)))
	const unsigned RIFFChunkName = MKCHUNKNAME('R','I','F','F');
	const unsigned WAVEChunkName = MKCHUNKNAME('W','A','V','E');
	const unsigned fmtChunkName  = MKCHUNKNAME('f','m','t',' ');
	const unsigned dataChunkName = MKCHUNKNAME('d','a','t','a');
#undef MKCHUNKNAME
};

//=====================================================================================//
//                           void WaveDecoder::readChunkId()                           //
//=====================================================================================//
void WaveDecoder::readChunkId(unsigned mustBe)
{
	unsigned ckId;
	m_in >> ckId;
	if(ckId != mustBe) throw sound_error("Failed to read RIFF chunk");
}

//=====================================================================================//
//                          unsigned WaveDecoder::readChunk()                          //
//=====================================================================================//
unsigned WaveDecoder::readChunk(unsigned mustBe)
{
	readChunkId(mustBe);

	unsigned size;
	m_in >> size;

	return size;
}

//=====================================================================================//
//                             WaveDecoder::WaveDecoder()                              //
//=====================================================================================//
WaveDecoder::WaveDecoder(Stream stream)
:	m_stream(stream),
	m_in(m_stream.bin())
{
	readChunk(RIFFChunkName);
	readChunkId(WAVEChunkName);

	{
		unsigned size = readChunk(fmtChunkName);

		if(sizeof(WAVEFORMAT) > size) throw sound_error("Unknown fmt chunk");

		std::vector<char> buffer(size);
		m_in >> mll::io::array(buffer.begin(),buffer.end());

		WAVEFORMAT *wfmt = reinterpret_cast<WAVEFORMAT*>(&buffer[0]);
		if(wfmt->wFormatTag != WAVE_FORMAT_PCM)
		{
			throw sound_error("Unknown RIFF data format");
		}
		if(wfmt->nBlockAlign/wfmt->nChannels != sizeof(short))
		{
			throw sound_error("Unsupported bits per sample count");
		}

		m_format.rate = wfmt->nSamplesPerSec;
		m_format.channels = wfmt->nChannels;
	}

	{
		unsigned size = 0;
		while(!size)
		{
			try
			{
				size = readChunk(dataChunkName);
			}
			catch(const std::exception &)
			{
				unsigned tmp;
				m_in >> tmp;
				m_in.stream().seekg(tmp,std::ios::cur);
			}
		}
		m_startPos = m_in.stream().tellg();
		m_length = size / ( sizeof(short) * m_format.channels );
	}

	m_currentPos = 0;
}

//=====================================================================================//
//                             WaveDecoder::~WaveDecoder()                             //
//=====================================================================================//
WaveDecoder::~WaveDecoder()
{
}

//=====================================================================================//
//                        Format WaveDecoder::getFormat() const                        //
//=====================================================================================//
Format WaveDecoder::getFormat() const
{
	return m_format;
}

//=====================================================================================//
//                           unsigned WaveDecoder::decode()                            //
//=====================================================================================//
unsigned WaveDecoder::decode(Buffer &buffer)
{
	unsigned size = std::min(buffer.getAvailSize(),m_length-m_currentPos);
	if(!size) return size;

	unsigned block = m_format.channels*sizeof(short);

	std::vector<char> tmp(size*block);
	m_in.stream().read(&tmp[0],tmp.size());
	m_currentPos = (m_in.stream().tellg()-m_startPos)/block;
	buffer.feed(reinterpret_cast<short *>(&tmp[0]),size);
	return size;
}

//=====================================================================================//
//                              void WaveDecoder::reset()                              //
//=====================================================================================//
void WaveDecoder::reset()
{
	m_currentPos = 0;
	m_in.stream().seekg(m_startPos);
}

//=====================================================================================//
//                     unsigned WaveDecoder::getCurrentPos() const                     //
//=====================================================================================//
unsigned WaveDecoder::getCurrentPos() const
{
	return m_currentPos;
}

//=====================================================================================//
//                          unsigned WaveDecoder::getLength()                          //
//=====================================================================================//
unsigned WaveDecoder::getLength()
{
	return m_length;
}

//=====================================================================================//
//                      unsigned WaveDecoder::getLengthAnyways()                       //
//=====================================================================================//
unsigned WaveDecoder::getLengthAnyways()
{
	return m_length;
}

//=====================================================================================//
//                         std::string WaveDecoder::getInfo()                          //
//=====================================================================================//
std::string WaveDecoder::getInfo()
{
	return m_stream.name();
}