#if !defined(__DECODE_MGR_H_INCLUDED_9196595280472335__)
#define __DECODE_MGR_H_INCLUDED_9196595280472335__

class Decoder;

//=====================================================================================//
//                                   class DecodeMgr                                   //
//=====================================================================================//
class DecodeMgr : private noncopyable
{
public:
	std::auto_ptr<Decoder> createDecoder(Stream stream);
};

#endif // !defined(__DECODE_MGR_H_INCLUDED_9196595280472335__)