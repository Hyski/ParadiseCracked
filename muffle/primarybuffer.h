#if !defined(__PRIMARYBUFFER_H_INCLUDED_3051904287262003__)
#define __PRIMARYBUFFER_H_INCLUDED_3051904287262003__

//=====================================================================================//
//                                 class PrimaryBuffer                                 //
//=====================================================================================//
class PrimaryBuffer : private noncopyable
{
	ComPtr<IDirectSoundBuffer> m_buffer;
	ComPtr<IDirectSound3DListener> m_listener;

public:
	PrimaryBuffer(const class DirectSound &);
	~PrimaryBuffer();

	IDirectSoundBuffer *getBuffer() const { return m_buffer; }
	IDirectSound3DListener *getListener() const { return m_listener; }
};

#endif // !defined(__PRIMARYBUFFER_H_INCLUDED_3051904287262003__)