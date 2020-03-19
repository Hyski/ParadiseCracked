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

	// ���������� ������ ������
	virtual Format getFormat() const = 0;
	// ���������� ����� ������ � �����
	virtual unsigned decode(class Buffer &) = 0;
	// ���������� ������� ������������� � ������
	virtual void reset() = 0;
	// ���������� ������� �������
	virtual unsigned getCurrentPos() const = 0;
	// ���������� ����� ������
	virtual unsigned getLength() = 0;
	// ���������� ����� ������, ���� ���� ������ �� ������
	virtual unsigned getLengthAnyways() = 0;

	virtual std::string getInfo() = 0;
};

#endif // !defined(__DECODER_H_INCLUDED_5412684183372428__)