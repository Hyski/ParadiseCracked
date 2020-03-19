#if !defined(__D3D7_INCLUDED__)
#define __D3D7_INCLUDED__

#include <mll/algebra/matrix3.h>
#include <d3dtypes.h>

namespace mll
{

namespace algebra
{

//=====================================================================================//
//                           inline ::D3DMATRIX D3DMATRIX()                            //
//=====================================================================================//
inline ::D3DMATRIX D3DMATRIX(const matrix3 &m)
{
	return ::D3DMATRIX(m._00,m._01,m._02,m._03,
					   m._10,m._11,m._12,m._13,
					   m._20,m._21,m._22,m._23,
					   m._30,m._31,m._32,m._33);
}

//=====================================================================================//
//                           inline ::D3DVECTOR D3DVECTOR()                            //
//=====================================================================================//
inline ::D3DVECTOR D3DVECTOR(const vector3 &v)
{
	return ::D3DVECTOR(v.x,v.y,v.z);
}

//=====================================================================================//
//                            inline matrix3 make_matrix()                             //
//=====================================================================================//
inline matrix3 make_matrix(const ::D3DMATRIX &m)
{
	return matrix3( m._11, m._12, m._13, m._14,
					m._21, m._22, m._23, m._24,
					m._31, m._32, m._33, m._34,
					m._41, m._42, m._43, m._44 );
}

} // namespace 'algebra'

} // namespace 'mll'

#endif