// VertexBuffer.cpp: implementation of the VertexBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "precomp.h"
#include "VertexBuffer.h"
VertexBuffer::VertexBuffer(IDirect3DDevice7 *_D3DDev,IDirect3D7 *_D3D)
               :Locked(false),Buffer(NULL),Data(NULL),
                FillSize(0),Size(0),Pitch(0),
                D3DDev(_D3DDev),D3D(_D3D)
  {
   BufDesc.dwSize=sizeof(BufDesc);
   BufDesc.dwCaps=D3DVBCAPS_WRITEONLY/*|D3DVBCAPS_SYSTEMMEMORY*/;
  }

VertexBuffer::~VertexBuffer()
  {
  Unlock();
  RELEASE(Buffer);
  }


void VertexBuffer::SetDesc(unsigned FVF)
  {
//D3DFVF_XYZ,D3DFVF_XYZRHW,
//D3DFVF_NORMAL,D3DFVF_DIFFUSE,D3DFVF_TEXn,D3DFVF_TEXCOORDSIZE2(n)

  BufDesc.dwFVF=FVF;
  }
void VertexBuffer::SetSize(int size)
  {
  Unlock();
  RELEASE(Buffer);
  Size=size;
  FillSize=0;
  BufDesc.dwNumVertices=Size;
  if(Size)
    {
    HRESULT h=D3D->CreateVertexBuffer(&BufDesc,&Buffer,0);
    if(h!=D3D_OK) throw CASUS("Can't create VB!");
    }
  }
