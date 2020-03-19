/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: менеджер эффектов с модельками
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                


#if !defined(__MODELMANAGER_H__)
#define __MODELMANAGER_H__

#include "Effect.h"
class GraphPipe;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//
// класс менеджера эффектов с модельками
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ModelManager
{
	typedef std::list<BaseEffect*> EffectList_t;
	EffectList_t m_effects;
public:
	ModelManager();
	~ModelManager();
	// следующий тик
	void NextTick(float time);
	// отрисовка
	void Draw();
	// добавить эффект в список
	void AddEffect(BaseEffect* effect);
	// очистить все эффекты
	void clear();
};


#endif // !defined(__MODELMANAGER_H__)
