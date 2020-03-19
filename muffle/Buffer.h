#if !defined(__BUFFER_H_INCLUDED_2748740437371592__)
#define __BUFFER_H_INCLUDED_2748740437371592__

#include "format.h"

//=====================================================================================//
//                                    class Buffer                                     //
//=====================================================================================//
class Buffer : private noncopyable, public CountedInstances<Buffer>
{
public:
	// ���������� ������ ������ ������
	virtual Format getFormat() const = 0;
	// ���������� ������ ���������� ������
	virtual unsigned getAvailSize() const = 0;
	// ���������� ������ � �����. count - ���������� ������ �������
	// �. �. ���� count ��������� �� rate, �� ��������� ������������ ����� ������ � ���.
	virtual void feed(const short *data, unsigned count) = 0;
};

#endif // !defined(__BUFFER_H_INCLUDED_2748740437371592__)