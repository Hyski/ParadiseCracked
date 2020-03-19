#if !defined(__DYNAMIC_DECODE_STREAM_QUERY_H_INCLUDED_5610686414973456__)
#define __DYNAMIC_DECODE_STREAM_QUERY_H_INCLUDED_5610686414973456__

#include "DecodeThread.h"

class DynamicDecodeStream;

//=====================================================================================//
//              class DynamicDecodeStreamQuery : public DecodeThreadQuery              //
//=====================================================================================//
class DynamicDecodeStreamQuery : public DecodeThreadQuery
{
	DynamicDecodeStream * const m_stream;
	unsigned m_part;

public:
	DynamicDecodeStreamQuery(DynamicDecodeStream *, unsigned part);
	virtual ~DynamicDecodeStreamQuery();

	virtual void start();
	virtual void finish();
};


#endif // !defined(__DYNAMIC_DECODE_STREAM_QUERY_H_INCLUDED_5610686414973456__)