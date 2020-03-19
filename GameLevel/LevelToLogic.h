// LevelToLogic.h: interface for the LevelToLogic class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LEVELTOLOGIC_H__83AD63A2_D1FB_4A72_8A34_831057F0686C__INCLUDED_)
#define AFX_LEVELTOLOGIC_H__83AD63A2_D1FB_4A72_8A34_831057F0686C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class LogicAPI
  {
  private:
    static LogicAPI *API;
  public://API methods:
    virtual void CaseDestroyed(const std::string &/*Name*/){};
    //уведомление об изменении опций
    virtual void OptionsChanged() = 0;

  public:
    static LogicAPI *GetAPI(){return API;};
    void SetAPI(LogicAPI *api){API=api;};


    LogicAPI(){API=NULL;};
    virtual ~LogicAPI(){};
    
  };
class LevelAPI
  {
  private:
    static LevelAPI *API;

  public://API methods:
    virtual void EndTurn(unsigned Flag)=0;
    virtual void EnableJoint(int Num,bool Flag)=0;

  public:
    static LevelAPI* GetAPI(){return API;};
    void SetAPI(LevelAPI *api){API=api;};

    LevelAPI(){API=NULL;};
    virtual ~LevelAPI(){};
    
  };

#endif // !defined(AFX_LEVELTOLOGIC_H__83AD63A2_D1FB_4A72_8A34_831057F0686C__INCLUDED_)
