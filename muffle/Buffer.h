#if !defined(__BUFFER_H_INCLUDED_2748740437371592__)
#define __BUFFER_H_INCLUDED_2748740437371592__

#include "format.h"

//=====================================================================================//
//                                    class Buffer                                     //
//=====================================================================================//
class Buffer : private noncopyable, public CountedInstances<Buffer>
{
public:
	// Возвращает формат данных буфера
	virtual Format getFormat() const = 0;
	// Возвращает размер доступного буфера
	virtual unsigned getAvailSize() const = 0;
	// Отправляет данные в буфер. count - количество полных сэмплов
	// т. е. если count разделить на rate, то получится длительность куска данных в сек.
	virtual void feed(const short *data, unsigned count) = 0;
};

#endif // !defined(__BUFFER_H_INCLUDED_2748740437371592__)