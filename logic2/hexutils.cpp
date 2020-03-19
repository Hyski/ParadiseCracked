//
//Методы для hex'вых утилит
//
#pragma warning(disable:4786)

#include "logicdefs.h"

#pragma hdrstop

#include "hexutils.h"

//карта для маленького hex'a
const ipnt2_t HexUtils::m_mapper1[2][6] = 
{
    { 
        ipnt2_t( 1, 0), ipnt2_t( 0, 1), ipnt2_t(-1, 1), 
        ipnt2_t(-1, 0), ipnt2_t(-1,-1), ipnt2_t(0, -1) 
    },
    { 
        ipnt2_t( 1, 0), ipnt2_t( 1, 1), ipnt2_t( 0, 1), 
        ipnt2_t(-1, 0), ipnt2_t( 0,-1), ipnt2_t( 1,-1)
    }
};

//карта для большого hex'а
const ipnt2_t HexUtils::m_mapper2[2][12] =
{
    {
        ipnt2_t( 2, 0), ipnt2_t( 1, 1), ipnt2_t( 1, 2),
        ipnt2_t( 0, 2), ipnt2_t(-1, 2), ipnt2_t(-2, 1),
        ipnt2_t(-2, 0), ipnt2_t(-2,-1), ipnt2_t(-1,-2),
        ipnt2_t( 0,-2), ipnt2_t( 1,-2), ipnt2_t( 1,-1)
    },
    { 
        ipnt2_t( 2, 0), ipnt2_t( 2, 1), ipnt2_t( 1, 2),
        ipnt2_t( 0, 2), ipnt2_t(-1, 2), ipnt2_t(-1, 1),
        ipnt2_t(-2, 0), ipnt2_t(-1,-1), ipnt2_t(-1,-2),
        ipnt2_t( 0,-2), ipnt2_t( 1,-2), ipnt2_t( 2,-1) 
    }
};

/*
void HexUtils::SetHexSize(float size)
{
    m_hex_size   = size;
    m_hex_size_2 = m_hex_size / 2;

    m_hex_size_4 = m_hex_size / 4;
    //m_hex_size_8 = m_hex_size / 8;

    //m_cos30_size_4 = m_hex_size_4 * cos_30;
    //m_cos30_size_2 = m_hex_size_2 * cos_30;

    m_radius  = m_hex_size / COS30m2;
    //m_y_delta = m_hex_size * cos_30;
    m_side_2  = m_hex_size_2 * TAN30;
}
*/