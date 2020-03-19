//
// –еализаци€ методов игроков
//
#pragma warning(disable:4786)

#include "logicdefs.h"

#include "form.h"
#include "Player.h"
#include "AIBase.h"
#include "Entity.h"
#include "AIUtils.h"
#include "GameObserver.h"

//=====================================================

//
// базовый класс дл€ все игроков
//
class BasePlayer{
public:

    BasePlayer(player_type type = PT_NONE) : m_ptype(type) {}
    virtual ~BasePlayer(){}

    DCTOR_ABS_DEF(BasePlayer)

    //определение типа игрока
    player_type GetType() const {return m_ptype;}

    //в этой функции игрок думает
    virtual void Think(state_type* st) = 0;

    //обработчик сохранени€
    virtual void MakeSaveLoad(SavSlot& slot)
    {
        if(slot.IsSaving()){
            slot << static_cast<int>(m_ptype);
        }else{
            int tmp; slot >> tmp;
            m_ptype = static_cast<player_type>(tmp);
        }
    }

private:

    //необходимо дл€ выполнени€ операций сохранени€ / загрузки
    player_type  m_ptype;     //тип команды кот "рулит" игрок     
};

//=====================================================

namespace{

//=====================================================

//
// общие элементы дл€ всех игроков
//
class CommonPlayer : public BasePlayer, protected GameObserver{
public:

    CommonPlayer(player_type type) : BasePlayer(type)
    {
        GameEvMessenger::GetInst()->Attach(this, EV_TURN);
    }

    ~CommonPlayer()
    {
        GameEvMessenger::GetInst()->Detach(this);
    }

protected:

    void Update(subject_t subj, event_t event, info_t info)
    {
        turn_info* ptr = static_cast<turn_info*>(info);

        if(ptr->m_player == GetType() && ptr->IsBegining()){

            EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_ALL_ENTS, GetType());

            while(itor != EntityPool::GetInst()->end()){
                AIUtils::CalcMovepnts4Turn(&*itor);
                ++itor;
            }        
        }
    }
};

//=====================================================

//
// "обычный" игрок
//
class HumanPlayer: public CommonPlayer{
public:

    HumanPlayer() : CommonPlayer(PT_PLAYER), m_current(EntityPool::GetInst()->end()) { }

    DCTOR_DEF(HumanPlayer)

    void Think(state_type* st)
    {
        while(      m_current != EntityPool::GetInst()->end() 
                && !PanicPlayer::GetInst()->Execute()){
            InitEntity(++m_current);
        }

        Forms::GetInst()->HandleInput(st);
    }
    
private:

    void Update(subject_t subj, event_t event, info_t info)
    {
        CommonPlayer::Update(subj, event, info);

        turn_info* ptr = static_cast<turn_info*>(info);

        if(ptr->m_player != GetType())
            return;

        if(ptr->IsBegining()){
            
            CameraPool::GetInst()->HandleNextTurn();
            ShieldPool::GetInst()->HandleNextTurn();
            
            InitEntity(m_current = EntityPool::GetInst()->begin(ET_ALL_ENTS, GetType()));
        }
    }

    void InitEntity(EntityPool::iterator& itor)
    {
        if(m_current == EntityPool::GetInst()->end()){
            Forms::GetInst()->ShowGameForm(true);
            return;
        }

        PanicPlayer::GetInst()->Init(&*m_current);

        if(PanicPlayer::GetInst()->Execute()) Forms::GetInst()->ShowPanicForm((&*m_current)->Cast2Human());        
   }

private:

    EntityPool::iterator m_current;
};

DCTOR_IMP(HumanPlayer)

//=====================================================

//
// компьютерный игрок
//
class ComputerPlayer: public CommonPlayer{
public:

    ComputerPlayer(player_type type, AINode* ai) : 
      CommonPlayer(type), m_ai(ai) {}

    ~ComputerPlayer() { delete m_ai; }

    void Think(state_type* st)
    {
        float progress = m_ai->Think(st);
        if(st) Forms::GetInst()->UpdateHMProress(progress/m_ai->getComplexity());
    }

    void MakeSaveLoad(SavSlot& slot)
    {
        BasePlayer::MakeSaveLoad(slot);
        m_ai->MakeSaveLoad(slot);        
    }

protected:

    void Update(subject_t subj, event_t event, info_t info)
    {
        CommonPlayer::Update(subj, event, info);

        turn_info* ptr = static_cast<turn_info*>(info);

        if(ptr->m_player == GetType() && ptr->IsBegining())
            Forms::GetInst()->ShowHMForm(ptr->m_player);        
    }

private:

    AINode* m_ai;
};

//
// игрок от Civilian
//
class CivilianPlayer : public ComputerPlayer{
public:

    CivilianPlayer() : ComputerPlayer(PT_CIVILIAN, AIFactory::GetInst()->CreateCivilianAI()) {}

    DCTOR_DEF(CivilianPlayer)
};

DCTOR_IMP(CivilianPlayer)

//
// игрок от Enemy
//
class EnemyPlayer : public ComputerPlayer{
public:

    EnemyPlayer() : ComputerPlayer(PT_ENEMY, AIFactory::GetInst()->CreateEnemyAI()) {}

    DCTOR_DEF(EnemyPlayer)
};

DCTOR_IMP(EnemyPlayer)

//=====================================================

} //namespace

//=====================================================

BasePlayer* PlayerFactory::CreateHumanPlayer()
{
    return new HumanPlayer();
}

BasePlayer* PlayerFactory::CreateEnemyPlayer()
{
    return new EnemyPlayer();
}

BasePlayer* PlayerFactory::CreateCivilianPlayer()
{
    return new CivilianPlayer();
}

PlayerFactory* PlayerFactory::GetInst()
{
    static PlayerFactory imp;
    return &imp;
}

//=====================================================

PlayerPool::PlayerPool(): m_current(MAX_TEAMS)
{
    memset(m_players, 0, sizeof(m_players));
}

PlayerPool::~PlayerPool()
{
    Reset();
}

void PlayerPool::Insert(BasePlayer* player)
{
    for(int k = 0; k < MAX_TEAMS; k++){
        if(m_players[k] == 0){
            m_players[k] = player;
            return;
        }
    }

    throw CASUS("PlayerPool: слишком много игроков");
}

void PlayerPool::Reset()
{
    for(int k = 0; k < MAX_TEAMS; delete m_players[k++]);

    memset(m_players, 0, sizeof(m_players));

    m_current = MAX_TEAMS;
}

void PlayerPool::Think()
{
		STACK_GUARD("PlayerPool::Think");

			{
		STACK_GUARD("PlayerPool::This");
    if(m_players[0] == 0) return;
			}

    if(m_current == MAX_TEAMS){
		STACK_GUARD("PlayerPool::Think::messages");

        //уведомить о начале игры    
        GameObserver::turn_info info(m_players[m_current = 0]->GetType(), GameObserver::TEV_BEG);
        GameEvMessenger::GetInst()->Notify(GameObserver::EV_TURN, &info);         
        return;
    }

			{
			STACK_GUARD("PlayerPool::Think::thinkCycle");
			
			for(size_t k = 0; k < MAX_TEAMS; k++){
        if(m_players[k] && k != m_current)
					m_players[k]->Think(0);
				}
			}
    
    //дать текущему игроку возможность подумать
    state_type st = ST_INCOMPLETE;
			{
					STACK_GUARD("PlayerPool::Think::CurPlayer");

    m_players[m_current]->Think(&st);
			}
    
    //игрок передал ход другому?
    if(st == ST_FINISHED){
		STACK_GUARD("PlayerPool::ThinlEndTurn");
        
        GameObserver::turn_info info(m_players[m_current]->GetType(), GameObserver::TEV_END);
        GameEvMessenger::GetInst()->Notify(GameObserver::EV_TURN, &info);         

        m_current = (m_current + 1)%MAX_TEAMS;

        info.m_event  = GameObserver::TEV_BEG;
        info.m_player = m_players[m_current]->GetType();

        GameEvMessenger::GetInst()->Notify(GameObserver::EV_TURN, &info);         
    }
}

void PlayerPool::MakeSaveLoad(SavSlot& slot)
{
    if(slot.IsSaving()){
        
        slot << m_current;        

        for(int k = 0; k < MAX_TEAMS; k++){
            
            slot << (m_players[k] != 0);
            
            if(m_players[k]){
                DynUtils::SaveObj(slot, m_players[k]);
                m_players[k]->MakeSaveLoad(slot);
            }
        }

    }else{

        slot >> m_current;

        bool flag;

        for(int k = 0; k < MAX_TEAMS; k++){
            
            slot >> flag;
            
            if(flag){
                DynUtils::LoadObj(slot, m_players[k]);
                m_players[k]->MakeSaveLoad(slot);
            }
        }
    }
}

PlayerPool* PlayerPool::GetInst()
{
    static PlayerPool imp;
    return &imp;
}