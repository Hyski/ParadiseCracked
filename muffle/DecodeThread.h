#if !defined(__DECODE_THREAD_H_INCLUDED_7074269960690902__)
#define __DECODE_THREAD_H_INCLUDED_7074269960690902__

#include <list>

class Buffer;
class Decoder;

//=====================================================================================//
//                               class DecodeThreadQuery                               //
//=====================================================================================//
class DecodeThreadQuery
{
	Buffer * const m_buffer;
	Decoder * const m_decoder;

public:
	DecodeThreadQuery(Buffer *buf, Decoder *dec) : m_buffer(buf), m_decoder(dec) {}
	virtual ~DecodeThreadQuery() {}

	virtual void start() = 0;
	virtual void finish() = 0;

	Buffer *buffer() const { return m_buffer; }
	Decoder *decoder() const { return m_decoder; }
};

//=====================================================================================//
//                                 class DecodeThread                                  //
//=====================================================================================//
class DecodeThread
{
	CriticalSection m_queueGuard;

	WndHandle m_exitThread;
	WndHandle m_queueUpdated;

	typedef std::allocator<DecodeThreadQuery*> Alloc_t;
	typedef std::list<DecodeThreadQuery*, Alloc_t> Queries_t;
	Queries_t m_queue;

	HANDLE m_thread;

	static void __cdecl threadStarter(DecodeThread *);
	void threadFunc();

public:
	DecodeThread();
	~DecodeThread();

	void addQuery(DecodeThreadQuery *);
	void removeQuery(Decoder *);
};

#endif // !defined(__DECODE_THREAD_H_INCLUDED_7074269960690902__)