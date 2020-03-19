#include "precomp.h"
#include "kernel.h"
#include "dsound.h"
#include "primarybuffer.h"

//=====================================================================================//
//                                  Kernel::Kernel()                                   //
//=====================================================================================//
Kernel::Kernel(const GUID &device, HWND wnd)
:	m_dsound(new DirectSound(device,wnd)),
	m_primary(new PrimaryBuffer(*m_dsound))
{
}

//=====================================================================================//
//                                  Kernel::~Kernel()                                  //
//=====================================================================================//
Kernel::~Kernel()
{
}