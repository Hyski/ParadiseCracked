#include "precomp.h"
#include "DecodeMgr.h"
#include "wave_decoder.h"
#include "ogg_decoder.h"

//=====================================================================================//
//                         Decoder *DecodeMgr::createDecoder()                         //
//=====================================================================================//
std::auto_ptr<Decoder> DecodeMgr::createDecoder(Stream stream)
{
	unsigned bytes;
	std::ios::pos_type pos = stream.stream().tellg();
	stream.bin() >> bytes;
	stream.stream().seekg(pos);

	if(bytes == 0x46464952) // "RIFF"
	{
		return std::auto_ptr<Decoder>(new WaveDecoder(stream));
	}
	else if(bytes == 0x5367674F) // "OggS"
	{
		return std::auto_ptr<Decoder>(new OggDecoder(stream));
	}

	throw cannot_create_decoder("Unknown sound file format");
}