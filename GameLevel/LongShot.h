/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Класс, отвечающий за поддержку и отрисовку                                                                  
               дальнего плана в игре

   Creation Date: 16.06.00
                                                                                
   Author: Grom
***************************************************************/                
                                                                                
 #if !defined(AFX_LONGSHOT_H__440CB9A0_43A0_11D4_A0E0_525405F0AA60__INCLUDED_)
#define AFX_LONGSHOT_H__440CB9A0_43A0_11D4_A0E0_525405F0AA60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include "../common/graphpipe/simpletexturedobject.h"
class GraphPipe;
class Camera;
class LongShot:public TexObject
  {
  protected:
    std::map<std::string,std::string> LevelToShader;
    point3 CamOffset;//точка, от которой строится фанера
    std::string CurrentLevelName;
    void ParseXLS();
		bool Enabled;
  public:
		void Enable(bool Flag);
    void LevelChanged(const std::string &NewName, GraphPipe *Pipe);
	  void Draw(GraphPipe *Pipe);
	  void Update(const Camera *Cam);
    LongShot();
    virtual ~LongShot();
  };

#endif // !defined(AFX_LONGSHOT_H__440CB9A0_43A0_11D4_A0E0_525405F0AA60__INCLUDED_)
