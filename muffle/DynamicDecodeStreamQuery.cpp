#include "precomp.h"
#include "DynamicDecodeStreamQuery.h"
#include "DynamicDecodeStream.h"
#include "Script.h"
#include "Decoder.h"

//=====================================================================================//
//                DynamicDecodeStreamQuery::DynamicDecodeStreamQuery()                 //
//=====================================================================================//
DynamicDecodeStreamQuery::DynamicDecodeStreamQuery(DynamicDecodeStream *stream,
												   unsigned part)
:	DecodeThreadQuery(stream->getBuffer(),stream->getDecoder()),
	m_stream(stream),
	m_part(part)
{
}

//=====================================================================================//
//                       void DynamicDecodeStreamQuery::start()                        //
//=====================================================================================//
void DynamicDecodeStreamQuery::start()
{
	m_stream->getBuffer()->lock(m_part);
}

//=====================================================================================//
//                       void DynamicDecodeStreamQuery::finish()                       //
//=====================================================================================//
void DynamicDecodeStreamQuery::finish()
{
	SoundBuffer *buf = m_stream->getBuffer();
	if(buf->getAvailSize() > 0)
	{
		if(!m_stream->getScript().repeat())
		{
			std::vector<short> tmp(buf->getAvailSize()*buf->getFormat().channels);
			std::fill(tmp.begin(),tmp.end(),0);
			m_stream->feed(&tmp[0],buf->getAvailSize());
			m_stream->activateStop();
		}
		else
		{
			m_stream->getDecoder()->reset();
			m_stream->getDecoder()->decode(*buffer());
		}
	}

	m_stream->getBuffer()->unlock();
}

//=====================================================================================//
//                DynamicDecodeStreamQuery::~DynamicDecodeStreamQuery()                //
//=====================================================================================//
DynamicDecodeStreamQuery::~DynamicDecodeStreamQuery()
{
}