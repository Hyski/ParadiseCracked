#if !defined(__DECODER_H_INCLUDED_5412684183372428__)
#define __DECODER_H_INCLUDED_5412684183372428__

#include "format.h"

//=====================================================================================//
//                                    class Decoder                                    //
//=====================================================================================//
class Decoder : public CountedInstances<Decoder>
{
public:
	enum Length
	{
		lCannotBeDetermined		=	0xFFFFFFFF,
		lLengthyOperation		=	0xFFFFFFFE,
	};

	virtual ~Decoder() {}

	// Возвращает формат потока
	virtual Format getFormat() const = 0;
	// Декодирует кусок данных в буфер
	virtual unsigned decode(class Buffer &) = 0;
	// Перемещает позицию декодирования в начало
	virtual void reset() = 0;
	// Возвращает текущую позицию
	virtual unsigned getCurrentPos() const = 0;
	// Возвращает длину потока
	virtual unsigned getLength() = 0;
	// Возвращает длину потока, если есть способ ее узнать
	virtual unsigned getLengthAnyways() = 0;

	virtual std::string getInfo() = 0;
};

#endif // !defined(__DECODER_H_INCLUDED_5412684183372428__)