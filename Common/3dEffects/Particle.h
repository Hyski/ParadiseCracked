/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: классы частиц                                                                 
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined (__PARTICLE_H__)
#define __PARTICLE_H__
             
                                        
//
// базовый класс частицы BaseParticle
//
class BaseParticle
{
public:
	// координаты
	point3 coords;
	// цвет
	unsigned int color;
	// размер квадрата
	float size;

	// конструктор по умолчанию
	BaseParticle(): coords(0, 0, 0), color(0), size(0)
	{}
	virtual ~BaseParticle() {}
};

#endif // __PARTICLE_H__