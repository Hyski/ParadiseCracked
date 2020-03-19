#pragma warning(disable:4786)

#include "logicdefs.h"
#include "visutils3.h"

//===================================================================

VisMap* VisMap::GetInst()
{
    static VisMap imp;
    return &imp;
}

VisMap::VisMap() : m_relations(0)
{
}

VisMap::~VisMap() 
{
}

void VisMap::Reset()
{   
    if(m_relations){
        
        for(size_t k = 0; k < m_spectators.size() * m_markers.size(); delete m_relations[k++]); 

        delete[] m_relations;
        m_relations = 0;
    }

    m_markers.clear();
    m_spectators.clear();

    m_markers.push_back(0);
    m_spectators.push_back(0);
}

void VisMap::Insert(Marker* marker)
{
    for(size_t k = 1; k < m_markers.size(); k++){
        if(m_markers[k] == 0){

            //вставим маркер в k-ю позицию
            m_markers[k] = marker;
            marker->SetMID(k);
            marker->ZeroRequests();
            marker->Highlight(false);
            
            //создадим связи между маркером и наблюдателями
            for(size_t j = 0; j < m_spectators.size(); j++){

                if(m_spectators[j] == 0)
                    continue;

                if(MSRelation* relation = m_spectators[j]->CreateRelation(marker))
                    m_relations[k + m_markers.size() * j] = relation;
            }

            Update(marker, UE_INS_MARKER);
            return;
        }
    }

    //расширим массив маркеров
    m_markers.push_back(0);

    //расширим карту отношений
    ExpandRelationsTbl(ipnt2_t(m_markers.size() - 1, m_spectators.size()),
                       ipnt2_t(m_markers.size(), m_spectators.size()));

    //попробуем вставить еще раз
    Insert(marker);
}

void VisMap::Remove(Marker* marker)
{
    //найдем маркер 
    if(m_markers[marker->GetMID()] == marker){

        //послеть уведомление
        Update(marker, UE_DEL_MARKER);

        //удалим все связи с карты
        for(size_t k = 0; k < m_spectators.size(); k++){
            
            size_t index = marker->GetMID() + m_markers.size() * k;

            delete m_relations[index];
            m_relations[index] = 0;
        }

        //удалим ссылку на массив
        m_markers[marker->GetMID()] = 0;
        marker->SetMID(0);
    }
}

void VisMap::Insert(Spectator* spectator)
{
    //найдем свободную ячейку в массиве наблюдателей
    for(size_t k = 1; k < m_spectators.size(); k++){

        if(m_spectators[k] == 0){

            spectator->SetSID(k);
            spectator->OnInsert();
            m_spectators[k] = spectator;

            //создадим связи ко всем маркерам
            for(size_t j = 0; j < m_markers.size(); j++){

                if(m_markers[j] == 0)
                    continue;

                if(MSRelation* relation = spectator->CreateRelation(m_markers[j]))
                    m_relations[j + m_markers.size() * k] = relation;                
            }

            Update(spectator, UE_INS_SPECTATOR);
            return;
        }
    }

    //расширим массив наблюдателей
    m_spectators.push_back(0);

    //расширим карту отношений
    ExpandRelationsTbl(ipnt2_t(m_markers.size(), m_spectators.size() - 1),
                       ipnt2_t(m_markers.size(), m_spectators.size()));

    //попробуем вставить еще раз
    Insert(spectator);
}

void VisMap::Remove(Spectator* spectator)
{
    //найдем наблюдателя
    if(m_spectators[spectator->GetSID()] == spectator){

        //послать уведомление
        Update(spectator, UE_DEL_SPECTATOR);

        //удалить вс связи
        for(size_t k = 0; k < m_markers.size(); k++){

            size_t index = k + m_markers.size() * spectator->GetSID();

            delete m_relations[index];
            m_relations[index] = 0;
        }

        m_spectators[spectator->GetSID()] = 0;

        spectator->SetSID(0);
        spectator->OnRemove();
    }
}

void VisMap::Update(Marker* marker, update_event event)
{
    for(size_t k = 0; k < m_spectators.size(); k++){

        if(m_spectators[k] == 0)
            continue;

        size_t index = marker->GetMID() + m_markers.size() * k;            
        if(m_relations[index]) m_relations[index]->Update(marker, m_spectators[k], event);        
    }

    while(marker->HaveRequests()) SwitchMarker(marker);
}

void VisMap::Update(Spectator* spectator, update_event event)
{
    for(size_t k = 0; k < m_markers.size(); k++){

        Marker* marker = m_markers[k];
        if(marker == 0) continue;

        int index = k + m_markers.size() * spectator->GetSID();
        
        if(m_relations[index]){
            m_relations[index]->Update(marker, spectator, event);        
            while(marker->HaveRequests()) SwitchMarker(marker);
        }
    }
}

void VisMap::ExpandRelationsTbl(const ipnt2_t& old_size, const ipnt2_t& new_size)
{
    MSRelation** new_tbl = new MSRelation* [new_size.x*new_size.y];

    memset(new_tbl, 0, sizeof(MSRelation*)*new_size.x*new_size.y);

    if(m_relations){

        for(int m = 0; m < old_size.x; m++){
            for(int s = 0; s < old_size.y; s++){
                new_tbl[m + new_size.x*s] = m_relations[m + old_size.x*s];
            }
        }

        delete[] m_relations;   
    }
    
    m_relations = new_tbl;
}

VisMap::spectator_itor VisMap::spectator_begin(Marker* marker, unsigned type)
{
    return marker ? spectator_itor(marker->GetMID(), type, 0) : spectator_end();
}

VisMap::spectator_itor VisMap::spectator_end()
{
    return spectator_itor(0, VT_NONE, m_spectators.size());
}

VisMap::marker_itor VisMap::marker_begin(Spectator* spectator, unsigned type)
{    
    return spectator ? marker_itor(spectator->GetSID(), type, 0) : marker_end();
}

VisMap::marker_itor VisMap::marker_end()
{
    return marker_itor(0, VT_NONE, m_markers.size());
}

void VisMap::SwitchMarker(Marker* marker)
{
    bool flight = false;
    
    //просмотрим строку таблицы отношений для этого маркера
    for(size_t j = 0; j < m_spectators.size() && !flight; j++){                
        size_t index = marker->GetMID() + m_markers.size() * j;
        flight = m_relations[index] && (m_relations[index]->GetFlags() & VT_HIGHLIGHT);
    }
    
    marker->ZeroRequests();
    
    if(marker->IsHighlighted() != flight)
        marker->Highlight(flight);            
}

unsigned VisMap::GetVisFlags(Spectator* spectator, Marker* marker)
{
    if(spectator == 0 || marker == 0) return 0;

    if(m_spectators[spectator->GetSID()] == spectator && m_markers[marker->GetMID()]){
        size_t index = marker->GetMID() + m_markers.size() * spectator->GetSID();
        if(m_relations[index]) return m_relations[index]->GetFlags();
    }

    return 0;
}

//===================================================================

VisMap::SpectatorItor& VisMap::SpectatorItor::operator ++()
{
    for(m_first++ ; m_first < m_vis_map->m_spectators.size(); m_first++){
        if(IsSuitable(m_vis_map->m_spectators[m_first]))
            return *this;
    }
    
    m_first = m_vis_map->m_spectators.size();
    return *this;
}

VisMap::MarkerItor& VisMap::MarkerItor::operator ++()
{
    for(m_first++ ; m_first < m_vis_map->m_markers.size(); m_first++){
        if(IsSuitable(m_vis_map->m_markers[m_first]))
            return *this;
    }
    
    m_first = m_vis_map->m_markers.size();
    return *this;
}

//===================================================================

void Spectator::Detach(SpectatorObserver* observer)
{
    m_observers.Detach(observer);
}

void Spectator::Attach(SpectatorObserver* observer, SpectatorObserver::event_t event)
{
    m_observers.Attach(observer, event);
}

void Spectator::Notify(SpectatorObserver::event_t event, SpectatorObserver::info_t info)
{
    m_observers.Notify(this, event, info);
}

//===================================================================

void Marker::Detach(MarkerObserver* observer)
{
    m_observers.Detach(observer);
}

void Marker::Attach(MarkerObserver* observer, MarkerObserver::event_t event)
{
    m_observers.Attach(observer, event);
}

void Marker::Notify(MarkerObserver::event_t event, MarkerObserver::info_t info)
{
    m_observers.Notify(this, event, info);
}

void Marker::ZeroRequests()
{ 
    m_counter = 0;
}

bool Marker::HaveRequests() const 
{
    return m_counter > 0;
}

void Marker::AddRequest(unsigned vis)
{
    if(IsHighlighted()){
        if(vis & VT_INVISIBLE) m_counter++;
    }else{
        if(vis & VT_HIGHLIGHT) m_counter++;
    }
}

void Marker::Highlight(bool flag)
{
    if(m_mid) VisMap::GetInst()->Update(this, flag ? VisMap::UE_MARKER_ON : VisMap::UE_MARKER_OFF);
}

//===================================================================

void MSRelation::SwitchMarker(unsigned flags, Marker* marker, Spectator* spectator)
{
    if(m_flags == flags) return;

    m_flags = flags;
    
    if(marker){

        MarkerObserver::spectator_info spr_info(spectator);
    
        if(m_flags & VT_VISIBLE) marker->Notify(MarkerObserver::EV_VISIBLE_FOR_SPECTATOR, &spr_info);
        if(m_flags & VT_INVISIBLE) marker->Notify(MarkerObserver::EV_INVISIBLE_FOR_SPECTATOR, &spr_info);

        if(m_flags & (VT_HIGHLIGHT|VT_INVISIBLE)) marker->AddRequest(m_flags);    
    }

    if(spectator){

        SpectatorObserver::marker_info mrk_info(marker);

        if(m_flags & VT_VISIBLE) spectator->Notify(SpectatorObserver::EV_VISIBLE_MARKER, &mrk_info);
        if(m_flags & VT_INVISIBLE) spectator->Notify(SpectatorObserver::EV_INVISIBLE_MARKER, &mrk_info);
    }            
}
