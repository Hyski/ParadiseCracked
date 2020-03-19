/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: менеджер эффектов
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#include "precomp.h"

#include "ModelManager.h"
#include "../GraphPipe/GraphPipe.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//
// класс менеджера эффектов с модельками
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ModelManager::ModelManager()
{
}

ModelManager::~ModelManager()
{
	EffectList_t::iterator i = m_effects.begin();
	EffectList_t::iterator end = m_effects.end();
	while(i != end)
	{
		delete (*i);
		++i;
	}
}

// очистить все эффекты
void ModelManager::clear()
{
	EffectList_t::iterator i = m_effects.begin();
	EffectList_t::iterator end = m_effects.end();
	while(i != end)
	{
		delete (*i);
		++i;
	}
}
	
// следующий тик
void ModelManager::NextTick(float time)
{
	EffectList_t::iterator i = m_effects.begin();
	EffectList_t::iterator end = m_effects.end();
	while(i != end)
	{
		(*i)->NextTick(time);
		if((*i)->IsFinished())
		{
			delete (*i);
			i = m_effects.erase(i);
			continue;
		}
		++i;
	}
}

// отрисовка
void ModelManager::Draw()
{
	EffectList_t::iterator i = m_effects.begin();
	EffectList_t::iterator end = m_effects.end();
	while(i != end)
	{
		(*i)->Update();
		++i;
	}
}

// добавить эффект в список
void ModelManager::AddEffect(BaseEffect* effect)
{
	m_effects.push_back(effect);
}





