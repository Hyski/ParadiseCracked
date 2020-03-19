#pragma warning(disable:4786)

#include "logicdefs.h"

#include "thing.h"
#include "entity.h"
#include "aiutils.h"
#include "graphic.h"
#include "dirtylinks.h"
#include "cheatparser.h"
#include "thingfactory.h"
#include "enemydetection.h"

//=====================================================================

namespace{

    BaseEntity* FindEntity(const rid_t& rid)
    {
        EntityPool::iterator itor = EntityPool::GetInst()->begin();
        while(itor != EntityPool::GetInst()->end()){
            
            if(itor->GetInfo()->GetRID() == rid)
                return &*itor;

            ++itor;
        }

        return 0;
    }

    //
    // направить набор данных на парсинг
    //
    class ItemSetAcceptor : public ItemSetParser::Acceptor{
    public:
        
        ItemSetAcceptor(ThingScriptParser* parser) : m_parser(parser) {}

        void Accept(const rid_t& rid)
        {
            int line;
            std::string script;
            
            if(!AIUtils::GetItemSetStr(rid, &script, &line)) 
                return;
            
            if(m_parser) m_parser->Parse(script);
        }

    private:

        ThingScriptParser* m_parser;
    };

    //
    // бросить предмет рядом с существом
    //
    class DepotAcceptor : public ThingScriptParser::Acceptor{
    public:

        DepotAcceptor(BaseEntity* ent)
        {
            if(HumanEntity* hum = ent->Cast2Human()){
                
                if(VehicleEntity* veh = hum->GetEntityContext()->GetCrew())
                    m_pos = veh->GetGraph()->GetPos2();
                else
                    m_pos = hum->GetGraph()->GetPos2();
            }

            m_pos = ent->GetGraph()->GetPos2();
        }

        bool AcceptThing(BaseThing* thing)
        {
            Depot::GetInst()->Push(m_pos, thing);
            return true;
        }

        void AcceptOrgInfo(const rid_t rid) {}
        void AcceptEntInfo(entity_type type, const rid_t& rid) {}
        void AcceptThingInfo(thing_type type, const rid_t& rid) {}

    private:

        ipnt2_t m_pos;
    };
}

//=====================================================================

CheatServer::CheatServer()
{
}

CheatServer::~CheatServer()
{
}

void CheatServer::AddExperience(exp_type type, int value, const rid_t& rid)
{
    switch(type){
    case ET_QUEST:
        AIUtils::AddExp4Quest(value, &*EntityPool::GetInst()->begin(ET_ALL_ENTS, PT_PLAYER));
        DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("team_got_exp").c_str(), value));
        break;

    case ET_HACK:
        { 
            BaseEntity* entity = FindEntity(rid);
            if(entity && entity->Cast2Human()) AIUtils::AddExp4Hack(value, entity->Cast2Human());
        }
        break;

    case ET_KILL:
        if(BaseEntity* ent = FindEntity(rid)) AIUtils::AddExp4Kill(value, ent);
        break;
    }
}

void CheatServer::SetAlignment(const rid_t& rid_a, const rid_t& rid_b, int value)
{
    EnemyDetector::getInst()->setRelation(rid_a, rid_b, static_cast<RelationType>(value));
}

void CheatServer::GiveItems(const rid_t& ent_rid, const std::string& script)
{
    if(BaseEntity* entity = FindEntity(ent_rid)){
        
        DepotAcceptor     thing_acceptor(entity);
        ThingScriptParser thing_parser(&thing_acceptor, ThingFactory::GetInst());
        ItemSetAcceptor   item_set_acceptor(&thing_parser);
        ItemSetParser     item_set_parser(&item_set_acceptor);

        item_set_parser.Parse(script);
    }
}

//=====================================================================

CheatParser::CheatParser(CheatServer* server) :
    m_server(server)
{
}

CheatParser::~CheatParser()
{
}

bool CheatParser::Parse(const std::string& script)
{
    std::string str = script;
    std::transform(str.begin(), str.end(), str.begin(), tolower);

    m_input.str(str);
    m_input.clear();

    try{

        GetToken(); 

        //разбор скрипта
        while(m_token.m_type != T_END){

            switch(m_token.m_type){
            case T_EXPERIENCE:
                GetToken();
                ParseExperience();
                break;

            case T_ALIGNMENT:
                GetToken();
                ParseAlignment();
                break;

            case T_GIVEITEMS:
                GetToken();
                ParseGiveItems();
                break;
       
            default:
                
                throw ParseError();
            }
        }
    }
    catch(LexerError& ){
        return false;
    }
    catch(ParseError& ){
        return false;
    }
    
    return true;
}

void CheatParser::SetServer(CheatServer* server)
{
    m_server = server;
}

int CheatParser::GetLastPos()
{
    return m_input.tellg();
}

CheatParser::token_type CheatParser::GetToken()
{
    m_token.m_val = 0;
    m_token.m_str.clear();
    m_token.m_type = T_NONE;

    //пропустить пробелы
    m_input >> std::ws;
    
    //дошли до конца?
    if(!m_input.good()) return m_token.m_type = T_END; 

    int ch = m_input.peek();
       
    if(isdigit(ch)){
        m_input >> m_token.m_val;        
        return m_token.m_type = T_NUMBER;
    }
    
    if(iscsym(ch)){

        do{
            m_token.m_str += m_input.get();            
        }while(iscsym(m_input.peek()));

        if(m_token.m_str == "quest") return m_token.m_type = T_QUEST;
        if(m_token.m_str == "hack") return m_token.m_type = T_HACK;
        if(m_token.m_str == "kill") return m_token.m_type = T_KILL;
        if(m_token.m_str == "experience") return m_token.m_type = T_EXPERIENCE;
        if(m_token.m_str == "alignment") return m_token.m_type = T_ALIGNMENT;
        if(m_token.m_str == "enemy") return m_token.m_type = T_ENEMY;
        if(m_token.m_str == "friend") return m_token.m_type = T_FRIEND;
        if(m_token.m_str == "neutral") return m_token.m_type = T_NEUTRAL;
        if(m_token.m_str == "give_items") return m_token.m_type = T_GIVEITEMS;
        
        return m_token.m_type = T_IDENT;
    }

    if(ch == '('){

        do{
            char ch = m_input.get();
            
            m_token.m_str += ch;
            
            if(ch == ')') return m_token.m_type = T_SCRIPT;

        }while(m_input.good());
    }

    throw LexerError();
}

void CheatParser::ParseExperience()
{
    int value;

    if(m_token.m_type != T_NUMBER)
        throw ParseError();

    value = m_token.m_val;

    GetToken();

    if(m_token.m_type == T_HACK || m_token.m_type == T_KILL){

        CheatServer::exp_type type =    m_token.m_type == T_HACK 
                                    ?   CheatServer::ET_HACK
                                    :   CheatServer::ET_KILL;

        if(GetToken() != T_IDENT) throw ParseError();        

        if(m_server) m_server->AddExperience(type, value, m_token.m_str);
        
        GetToken();
        return;
    }

    if(m_token.m_type == T_QUEST)
        GetToken();

    if(m_server) m_server->AddExperience(CheatServer::ET_QUEST, value);
}

void CheatParser::ParseAlignment()
{
    rid_t rid_a, rid_b;
    RelationType type;

    switch(m_token.m_type){
    case T_ENEMY: type = RT_ENEMY; break;
    case T_FRIEND: type = RT_FRIEND; break;
    case T_NEUTRAL: type = RT_NEUTRAL; break;
    default: throw ParseError();
    }

    if(GetToken() != T_IDENT) throw ParseError();

    rid_a = m_token.m_str;

    if(GetToken() != T_IDENT) throw ParseError();

    rid_b = m_token.m_str;

    if(m_server) m_server->SetAlignment(rid_a, rid_b, type);

    GetToken();
}

void CheatParser::ParseGiveItems()
{
    rid_t ent_rid;

    if(m_token.m_type != T_IDENT) throw ParseError();

    ent_rid = m_token.m_str;

    GetToken();

    if(m_token.m_type != T_IDENT && m_token.m_type != T_SCRIPT)
        throw ParseError();

    if(m_server) m_server->GiveItems(ent_rid, m_token.m_str);

    GetToken();
}

