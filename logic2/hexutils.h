//
// утилиты для работы с hex'ми
//

#ifndef _PUNCH_HEX_UTILS_H_
#define _PUNCH_HEX_UTILS_H_

//отключить warning о преобр. типов
#pragma warning(disable: 4244)

//
//свойства одного hex'са
//
namespace HexTraits{
    const int num_sides    = 6;
    const int border1_size = 6;
    const int border2_size = 12;
    
    const float size    = 1.0f;           //размер
    const float size_d2 = size/2.0f;     
    const float size_d4 = size/4.0f;
    const float radius  = size/COS30m2;   //радиус окр-ти описанной вокруг hex'са
    const float side_d2 = (size_d2)*TAN30;//1/2 длины стороны hex'са
};

//
//утилиты для работы с hex'вой сетки
//
class HexUtils{
public:

    //перевод координат из screen'овых в hex'вые
    static ipnt2_t scr2hex(point3 pnt);

    //Получить фронт в различных направлениях
    static void GetFrontPnts0(ipnt2_t pos, int dir, ipnt2_t* pnt);
    static void GetFrontPnts1(ipnt2_t pos, int dir, ipnt2_t pnts[]);
    static void GetFrontPnts2(ipnt2_t pos, int dir, ipnt2_t pnts[]);

    //Получить граничные hex'ы для разных размеров
    static void GetBorderPnts1(ipnt2_t pos, ipnt2_t pnts[]);
    static void GetBorderPnts2(ipnt2_t pos, ipnt2_t pnts[]);

    //взят обратное направление
    static int  GetReverseDir(int dir);

private:

    static bool IsLower1(float x, float y);
    static bool IsLower2(float x, float y);

private:

    static const ipnt2_t m_mapper1[2][6];
    static const ipnt2_t m_mapper2[2][12];
};

//
//~~~~~~~~~~~inlines~~~~~~~~~~~~~~
//

inline int HexUtils::GetReverseDir(int dir)
{
    dir += 3;
    return dir %= 6;
}

inline bool HexUtils::IsLower1(float x, float y)
{
    x = fmod(x, HexTraits::size);  //отбросим периоды
  
    if(x < HexTraits::size_d2)        
        return y > (-TAN30 * (x - HexTraits::size_d2)); //первая половинка
    else
        return y > ( TAN30 * (x - HexTraits::size_d2));

    return false;
}
    
inline bool HexUtils::IsLower2(float x, float y)
{
    //нормализуем координаты
    y -= HexTraits::radius + HexTraits::side_d2;
    x  = fmod(x, HexTraits::size);
    
    if( x < HexTraits::size_d2)
        return y > ( TAN30 * x); 
    else
        return y > (-TAN30 * (x - HexTraits::size));  

    return false;    
}

inline ipnt2_t HexUtils::scr2hex(point3 scr_pnt)
{
    //скорректируем начало координат
    scr_pnt.y += HexTraits::radius;
    scr_pnt.x += HexTraits::size_d2;

    ipnt2_t  res;
    float    period = 2*HexTraits::radius + 2*HexTraits::side_d2;

    //учтем периоды
    res.y = floor(scr_pnt.y / period);

    //найдем остаток после отбрасывания периодов
    float rem = scr_pnt.y - period * res.y;

    //в одном периоде помешаются 2-ва hex'са
    res.y <<= 1; //res.y *= 2;
    
    //рассмотрим остаток
    do{
        if(rem < HexTraits::radius - HexTraits::side_d2){
           if(!IsLower1(scr_pnt.x, rem)) res.y --;
           break;
        }

        if(rem < HexTraits::radius + HexTraits::side_d2) 
            break;

        if(rem < 2*HexTraits::radius){
            if(IsLower2(scr_pnt.x, rem)) res.y ++;
            break;
        }

        res.y++;
        
    }while(false);

    //учтем смещение нечетных рядов
    if(res.y & 0x1) scr_pnt.x -= HexTraits::size_d2;

    //вычислим x 
    res.x = floor(scr_pnt.x / HexTraits::size);

    return res;
}

inline void HexUtils::GetFrontPnts0(ipnt2_t pos, int dir, ipnt2_t* pnt)
{
    *pnt = pos + m_mapper1[pos.y & 0x01][dir];
}

inline void HexUtils::GetFrontPnts1(ipnt2_t pos, int dir, ipnt2_t pnts[])
{
    int  _y = pos.y&0x01,
         i1 = (dir - 1) < 0 ? 5 : dir - 1,
         i2 = (dir + 1) > 5 ? 0 : dir + 1;

    pnts[0] = pos + m_mapper1[_y][i1]; 
    pnts[1] = pos + m_mapper1[_y][dir]; 
    pnts[2] = pos + m_mapper1[_y][i2]; 
}

inline void HexUtils::GetFrontPnts2(ipnt2_t pos, int dir, ipnt2_t pnts[])
{ 
    int _y = pos.y & 0x01,
        i3 = 2*dir,
        i2 = (i3 - 1) < 0  ? 11 : i3 - 1,
        i4 = i3 + 1, 
        i1 = (i3 - 2) < 0  ? 10 : i3 - 2,
        i5 = (i3 + 2) > 11 ? 0  : i3 + 2;

    pnts[0] = pos + m_mapper2[_y][i1]; 
    pnts[1] = pos + m_mapper2[_y][i2];
    pnts[2] = pos + m_mapper2[_y][i3];
    pnts[3] = pos + m_mapper2[_y][i4];
    pnts[4] = pos + m_mapper2[_y][i5]; 
}

inline void HexUtils::GetBorderPnts1(ipnt2_t pos, ipnt2_t pnts[])
{
    int _y = pos.y & 0x01;
    
    pnts[0] = pos + m_mapper1[_y][0];
    pnts[1] = pos + m_mapper1[_y][1];
    pnts[2] = pos + m_mapper1[_y][2];
    pnts[3] = pos + m_mapper1[_y][3];
    pnts[4] = pos + m_mapper1[_y][4];
    pnts[5] = pos + m_mapper1[_y][5];
}

inline void HexUtils::GetBorderPnts2(ipnt2_t pos, ipnt2_t pnts[])
{
    int _y = pos.y & 0x01;
    
    pnts[0] = pos + m_mapper2[_y][0];
    pnts[1] = pos + m_mapper2[_y][1];
    pnts[2] = pos + m_mapper2[_y][2];
    pnts[3] = pos + m_mapper2[_y][3];
    pnts[4] = pos + m_mapper2[_y][4];
    pnts[5] = pos + m_mapper2[_y][5];
    pnts[6] = pos + m_mapper2[_y][6];
    pnts[7] = pos + m_mapper2[_y][7];
    pnts[8] = pos + m_mapper2[_y][8];
    pnts[9] = pos + m_mapper2[_y][9];
    pnts[10] = pos + m_mapper2[_y][10];
    pnts[11] = pos + m_mapper2[_y][11];
}

#endif // _PUNCH_HEX_UTILS_H_
