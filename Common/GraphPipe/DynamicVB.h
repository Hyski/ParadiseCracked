#ifndef _DYNAMICVB_H_
#define _DYNAMICVB_H_

#include <d3d.h>
#include <assert.h>
//Grom:
//Added opportunity:
//                     Lock With supposed number of vertices
//  VertexType* pVertex = (VertexType*)pDynamicVB->LockSupposed( theVertexCount, &theStartVertex );
//  ....
//  pDynamicVB->UnlockSupposed(NumVerts);
/////////////////////////////
// D. Sim Dietrich Jr.
// sim.dietrich@nvidia.com
//////////////////////
//
// Usage :
//
// Create a seperate DynamicVB object for each FVF type to hold dynamic vertex data.
//
// /* NOTE: DX7 on DX7 runtime has issues that prevent > 1 dynamic VB to achieve optimal performance
//   This imples that you ONLY WANT ONE DYNAMIC VB, so just use the largest FVF size you need
//   It should be trivial to compare the tradeoff in your app of potentially using a larger FVF
//   size than necessary compared to having multiple dynamic VBs... */
//
// Create the VB like so :
//
// DynamicVB* pDynamicVB = new DynamicVB( pD3DDevice,
//										  MyFVF_Flags,
//										  theNumberOfVertices,
//                                        theVertexSize );
//																		  
// For each Frame
//
//  Call FlushAtFrameStart() once
//
//  pDynamicVB->FlushAtFrameStart();
//
//  Call Lock on the vertex buffer with the number of vertices you intend to draw and a dword pointer
//   to hold the beginning vertex index
//
//  Fill in the vertex buffer using the returned pointer, like so :
//
//  unsigned int theIndex = 0;
//  unsigned int theStartVertex = 0;
//
//  VertexType* pVertex = (VertexType*)pDynamicVB->Lock( theVertexCount, &theStartVertex );
//
//  /* NOTE: The startvertex is only for passing into d3d - you DO NOT NEED to
//      offset where you write into the locked vb.  The lock call returns a pointer to
//      the correct section of the vertex buffer for you.
//
//  int theIndex = 0;
//
//  // Fill in Vertex Data
//  pVertex[ theIndex++ ] = newVertexData; 
//
//  // Unlock
//  pDynamicVB->Unlock();
//
//  // Now Draw
//  pD3D->DIPVB( D3DPT_TRIANGLELIST, pDynamicVB->GetInterface(), theStartVertex, theVertexCount, ... );
//

class DynamicVB 
{
	private :

		LPDIRECT3DVERTEXBUFFER7 mpVB;
		//Grom
    int VertsRendered;
    int m_VertsWritten;
		unsigned int mVertexCount;
		unsigned int mFrame;
		unsigned int mPosition;

		unsigned int mVertexSize;

		bool		 mbLocked;
		bool		 mbFlush;

		enum LOCK_FLAGS
		{
			LOCKFLAGS_FLUSH  = DDLOCK_NOSYSLOCK | DDLOCK_WRITEONLY | DDLOCK_DISCARDCONTENTS,
			LOCKFLAGS_APPEND = DDLOCK_NOSYSLOCK | DDLOCK_WRITEONLY | DDLOCK_NOOVERWRITE
		};
		enum SUPPOSE_STATE
		{
		  NOSUPPOSE,SUPPOSE_START
		  };

	public :

		DynamicVB( const LPDIRECT3D7 pD3D, const DWORD& theFVF, const unsigned int& theVertexCount, const unsigned int& theVertexSize )
		{
		  m_VertsWritten=NOSUPPOSE;
			mpVB = 0;
			mPosition = 0;

			mbFlush = true;

			mbLocked = false;

			mVertexCount = theVertexCount;

			mVertexSize = theVertexSize;

			D3DVERTEXBUFFERDESC aDesc;
			memset( &aDesc, 0x00, sizeof( aDesc ) );
			aDesc.dwSize = sizeof( aDesc );
			extern bool _OldRender;
			aDesc.dwCaps = D3DVBCAPS_WRITEONLY /*| D3DVBCAPS_SYSTEMMEMORY*/;
			//if(_OldRender) aDesc.dwCaps |= D3DVBCAPS_SYSTEMMEMORY;
			aDesc.dwFVF = theFVF;
			aDesc.dwNumVertices = mVertexCount;

			HRESULT hr = pD3D->CreateVertexBuffer( &aDesc, &mpVB, 0 );
			assert( ( hr == D3D_OK ) && ( mpVB ) );
		}

		LPDIRECT3DVERTEXBUFFER7 GetInterface() const { return mpVB; }

		// Use at beginning of frame to force a flush of VB contents on first draw
		void FlushAtFrameStart() {VertsRendered=0; mbFlush = true; }

		void* Lock( const unsigned int& theLockCount, unsigned int* pStartVertex )
		{
			*pStartVertex = 0;
			void* pLockedData = 0;

			// Ensure there is enough space in the VB for this data
			if ( theLockCount > mVertexCount ) { assert( false ); return 0; }

			if ( mpVB )
			{
				DWORD dwFlags = LOCKFLAGS_APPEND;

				// If either user forced us to flush,
				//  or there is not enough space for the vertex data,
				//  then flush the buffer contents
				//
				if ( mbFlush || ( ( theLockCount + mPosition ) >= mVertexCount ) )
				{
					mbFlush = false;
					mPosition = 0;
					dwFlags = LOCKFLAGS_FLUSH;
				}

				DWORD dwSize = 0;
				HRESULT hr = mpVB->Lock( dwFlags, &pLockedData, &dwSize );

				assert( hr == D3D_OK );
				if ( hr == D3D_OK )
				{
					assert( pLockedData != 0 );
					mbLocked = true;
					*pStartVertex = mPosition;
					mPosition += theLockCount;
				}
				else
				{
					return 0;
				}

			}

      VertsRendered+=theLockCount;
			return ( (void*)( ( char*)pLockedData + (*pStartVertex) * mVertexSize ) );
		}

		void UnlockSupposed(int NumVerts)
		{
      assert(m_VertsWritten==SUPPOSE_START);
      VertsRendered+=NumVerts;
			mPosition += NumVerts;
			m_VertsWritten=NOSUPPOSE;
			if ( ( mbLocked ) && ( mpVB ) )
			{
				HRESULT hr = mpVB->Unlock();				
				assert( hr == D3D_OK );
				mbLocked = false;
			}
		}

    int GetNumVertsChopped(){return VertsRendered;}

		void* LockSupposed( const unsigned int& theLockCount, unsigned int* pStartVertex )
		{
		  m_VertsWritten=SUPPOSE_START;
			*pStartVertex = 0;
			void* pLockedData = 0;

			// Ensure there is enough space in the VB for this data
			if ( theLockCount > mVertexCount ) { return 0; }

			if ( mpVB )
			{
				DWORD dwFlags = LOCKFLAGS_APPEND;

				// If either user forced us to flush,
				//  or there is not enough space for the vertex data,
				//  then flush the buffer contents
				//
				if ( mbFlush || ( ( theLockCount + mPosition ) >= mVertexCount ) )
				{
					mbFlush = false;
					mPosition = 0;
					dwFlags = LOCKFLAGS_FLUSH;
				}

				DWORD dwSize = 0;
				HRESULT hr = mpVB->Lock( dwFlags, &pLockedData, &dwSize );

				assert( hr == D3D_OK );
				if ( hr == D3D_OK )
				{
					assert( pLockedData != 0 );
					mbLocked = true;
					*pStartVertex = mPosition;
					//mPosition += theLockCount;
				}
				else return 0;
			}
			

			return ( (void*)( ( char*)pLockedData + (*pStartVertex) * mVertexSize ) );
		}

		void Unlock()
		{
		 // assert(m_VertsWritten==NOSUPPOSE);
			if ( ( mbLocked ) && ( mpVB ) )
			{
				HRESULT hr = mpVB->Unlock();				
				assert( hr == D3D_OK );
				mbLocked = false;
			}
		}

		~DynamicVB()
		{
			Unlock();
			if ( mpVB )
			{
				mpVB->Release();
				mpVB = NULL;
			}
		}
	
};

#endif  _DYNAMICVB_H_
