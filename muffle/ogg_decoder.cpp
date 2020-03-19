#include "precomp.h"
#include "ogg_decoder.h"
#include "Buffer.h"

namespace
{
//=====================================================================================//
//                                 size_t streamRead()                                 //
//=====================================================================================//
	size_t __cdecl streamRead(void *ptr, size_t size, size_t nmemb, Stream *datasource)
	{
		datasource->stream().read(reinterpret_cast<char *>(ptr),size*nmemb);
		if(!datasource->stream().good())
		{
			datasource->stream().clear();
		}
		return datasource->stream().gcount()/size;
	}

//=====================================================================================//
//                                  int streamSeek()                                   //
//=====================================================================================//
	int __cdecl streamSeek(Stream *datasource, __int64 offset, int whence)
	{
		std::ios::seekdir way;
		switch(whence)
		{
			case SEEK_CUR: way = std::ios::cur; break;
			case SEEK_END: way = std::ios::end; break;
			case SEEK_SET: way = std::ios::beg; break;
		}
		datasource->stream().seekg(offset,way);
		assert( datasource->stream().good() );
		if(datasource->stream().good()) return 0;
		return 1;
	}

//=====================================================================================//
//                                  int streamClose()                                  //
//=====================================================================================//
	int __cdecl streamClose(Stream *datasource)
	{
		assert( datasource->stream().good() );
		return 0;
	}

//=====================================================================================//
//                                  long streamTell()                                  //
//=====================================================================================//
	long __cdecl streamTell(Stream *datasource)
	{
		assert( datasource->stream().good() );
		return datasource->stream().tellg();
	}

	ov_callbacks mycbks = 
	{
		(unsigned(__cdecl*)(void*,unsigned,unsigned,void*))streamRead,
		(int (__cdecl *)(void *,__int64,int))streamSeek,
		(int (__cdecl *)(void *))streamClose,
		(long (__cdecl *)(void *))streamTell
	};
}

//=====================================================================================//
//                              OggDecoder::OggDecoder()                               //
//=====================================================================================//
OggDecoder::OggDecoder(Stream stream)
:	m_stream(stream),
	m_in(m_stream.bin()),
	m_currentSection(0)
{
	switch(ov_open_callbacks(&m_stream,&m_file,0,0,mycbks))
	{
		case OV_EREAD : throw sound_error("- A read from media returned an error. ");
		case OV_ENOTVORBIS : throw sound_error("- Bitstream is not Vorbis data. ");
		case OV_EVERSION : throw sound_error("- Vorbis version mismatch. ");
		case OV_EBADHEADER : throw sound_error("- Invalid Vorbis bitstream header. ");
		case OV_EFAULT : throw sound_error("- Internal logic fault; indicates a bug or heap/stack corruption. ");
	}
	vorbis_info *vi = ov_info(&m_file,-1);
	m_format.rate = vi->rate;
	m_format.channels = vi->channels;
	m_length = ov_pcm_total(&m_file,-1);
	m_currPos = 0;
}

//=====================================================================================//
//                              OggDecoder::~OggDecoder()                              //
//=====================================================================================//
OggDecoder::~OggDecoder()
{
	ov_clear(&m_file);
}

//=====================================================================================//
//                        Format OggDecoder::getFormat() const                         //
//=====================================================================================//
Format OggDecoder::getFormat() const
{
	return m_format;
}

//=====================================================================================//
//                            unsigned OggDecoder::decode()                            //
//=====================================================================================//
unsigned OggDecoder::decode(Buffer &buff)
{
	unsigned size = std::min(buff.getAvailSize(),m_length-m_currPos);
	if(!size) return size;

	unsigned block = m_format.channels*sizeof(short);
	unsigned count = 0;

	std::vector<char> tmp(size*block);
	m_currPos += size;
	while(count < size*block)
	{
		long result = ov_read(&m_file,&tmp[0]+count,tmp.size()-count,0,2,1,&m_currentSection);
		if(result > 0) count += result;
	}
	buff.feed(reinterpret_cast<const short *>(&tmp[0]),size);
	return size;
}

//=====================================================================================//
//                              void OggDecoder::reset()                               //
//=====================================================================================//
void OggDecoder::reset()
{
	ov_pcm_seek(&m_file,0);
	m_currPos = 0;
}

//=====================================================================================//
//                     unsigned OggDecoder::getCurrentPos() const                      //
//=====================================================================================//
unsigned OggDecoder::getCurrentPos() const
{
	return m_currPos;
}

//=====================================================================================//
//                          unsigned OggDecoder::getLength()                           //
//=====================================================================================//
unsigned OggDecoder::getLength()
{
	return m_length;
}

//=====================================================================================//
//                       unsigned OggDecoder::getLengthAnyways()                       //
//=====================================================================================//
unsigned OggDecoder::getLengthAnyways()
{
	return m_length;
}

//=====================================================================================//
//                          std::string OggDecoder::getInfo()                          //
//=====================================================================================//
std::string OggDecoder::getInfo()
{
	return m_stream.name();
}