/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Класс-обертка для буфера вершин Direct3D 7.x
                                                                                
     Author: Grom 
   Creation: 29 июня 2000
***************************************************************/                

#if !defined(AFX_VERTEXBUFFER_H__D18928A0_4DEA_11D4_A0E0_525405F0AA60__INCLUDED_)
#define AFX_VERTEXBUFFER_H__D18928A0_4DEA_11D4_A0E0_525405F0AA60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class VertexBuffer  
  {
  protected:
    LPDIRECT3DVERTEXBUFFER7 Buffer; //Собственно, сам буфер
    IDirect3DDevice7 *D3DDev;//Устройства, на которых создан буфер
    IDirect3D7 *D3D;

    void *Data;//Данные (указатель на данные, когда буфер заблокирован)
    bool Locked; //флаг означающий, что буфер в данный момент в состоянии Lock

    D3DVERTEXBUFFERDESC BufDesc; //Описание буфера
  public:
    int Size;  //количество вершин в буфере
    int FillSize;  //количество заполненных вершин в буфере
    int Pitch;//Размер одной вершины в байтах
  public:
    VertexBuffer(IDirect3DDevice7 *_D3DDev, IDirect3D7 *D3D);
    virtual ~VertexBuffer();

    void* Lock();
    void Unlock();
    void SetDesc(unsigned FVF);
    void SetSize(int size); //Установление нового размера буфера
    void *GetData(){return Data;}
    void Flush(bool Last=true);
    void Flush(unsigned short*Idxs, int IdxsNum, bool Last);
  };
inline void VertexBuffer::Flush(bool Last)
  {
  if(!Buffer) return ;
  if(!FillSize) return;
  HRESULT h=D3DDev->DrawPrimitiveVB(D3DPT_TRIANGLELIST,Buffer,0,FillSize,0*D3DDP_WAIT);
  if(Last)
  FillSize=0;
  }
inline void VertexBuffer::Flush(unsigned short *Idxs, int IdxsNum, bool Last)
  {
  if(!Buffer) return ;
  if(!FillSize) return;

  HRESULT h=D3DDev->DrawIndexedPrimitiveVB
    (D3DPT_TRIANGLELIST,Buffer,0,FillSize,Idxs,IdxsNum,0*D3DDP_WAIT);
  if(h!=DD_OK)
    {
    if(h==D3DERR_INVALIDPRIMITIVETYPE)   throw CASUS("INVALID PRIMITIVE!");
    if(h==D3DERR_INVALIDVERTEXTYPE)   throw CASUS("D3DERR_INVALIDVERTEXTYPE!");
    if(h==D3DERR_VERTEXBUFFERLOCKED)   throw CASUS("D3DERR_VERTEXBUFFERLOCKED");
    if(h==DDERR_INVALIDOBJECT)   throw CASUS("DDERR_INVALIDOBJECT!");
    if(h==DDERR_INVALIDPARAMS)   throw CASUS("DDERR_INVALIDPARAMS!");
    if(h==DDERR_WASSTILLDRAWING)   throw CASUS("DDERR_WASSTILLDRAWING!");
        }

  /*if(Last)
  FillSize=0; */
  }
inline void VertexBuffer::Unlock()
  {
  if(!Locked) return;

  Data=NULL;
  Buffer->Unlock();

  Locked=false;
  }

inline void* VertexBuffer::Lock()
  {
  static unsigned flags=
    /*DDLOCK_WAIT|*/
    /*DDLOCK_NOSYSLOCK|*/
    DDLOCK_WRITEONLY|
    DDLOCK_DISCARDCONTENTS;

  if(Locked) return Data;

  if(FAILED(Buffer->Lock(flags,&Data,NULL)))
    Data=NULL;

  Locked=true;
  return Data;
  }

#endif // !defined(AFX_VERTEXBUFFER_H__D18928A0_4DEA_11D4_A0E0_525405F0AA60__INCLUDED_)
