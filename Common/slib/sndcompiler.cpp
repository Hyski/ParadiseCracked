/**************************************************************                 
                                                                                
                             Paradise Cracked                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Sound compiler
                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/

#include "tokens.h"
#include "AToken.h"
#include "atokenbuffer.h"
#include "SndLexer.h"
#include "..\precomp.h"
#include "..\3d\quat.h"
#include "sndcompiler.h"
#include "SndRecorder.h"
#include "..\..\globals.h"

extern const char* log_name;

#include "PakStream.h"

bool SndCompiler::Compile(const char* file_name)
{
    m_error = SUCCESSED;
	std::string compile_message;

	std::vector<DataMgr::FileInfo> FInfo;
	DataMgr::Scan("scripts/sound/",FInfo);
	std::string extens=".spt";
	for(int i=0;i<FInfo.size();i++)
	{
		if(FInfo[i].m_Extension==extens)
		{
			compile_message="success";
			DLGPakStream in(std::string(std::string("scripts/sound/")+FInfo[i].m_FullName).c_str());
			if(!(in.IsError()))
			{
				//make sound parser
				SndLexer            lexer(&in);
				ANTLRTokenBuffer    pipe(&lexer,1);
				FastToken           aToken;
				SndRecorder         recorder(&pipe);

				lexer.setToken(&aToken);
				
				try{
					recorder.Run();
				}
				catch(LexerErr)     
				{ 
					compile_message="lexer error";
					m_error = LEXER_ERROR; 
				}
				catch(ParserErr)
				{ 
					compile_message="parser error";
					m_error = PARSER_ERROR;
				}
				catch(TokenErr)     
				{ 
					compile_message="token error";
					m_error = TOKEN_ERROR;
				}
				catch(TokBufferErr) 
				{ 
					compile_message="token buffer error";
					m_error = TOKEN_BUFFER_ERROR;
				}
				catch(PCCTS_Err)    
				{ 
					compile_message="unknown error";     
					m_error = UNKNOWN_ERROR;
				}
#ifdef DSPL__DEB_VER_STORE_ERRORS				
				logFile[log_name]("...[compiling sound]: File [scripts/sound/%s] has proceeded with [%s].\n",FInfo[i].m_FullName.c_str(),compile_message);
#endif
				m_line = lexer.line();
			}
			else
			{
#ifdef DSPL__DEB_VER_STORE_ERRORS
				logFile[log_name]("file open error\n");
#endif
				m_error = FILE_OPEN_ERROR;
				m_line  = 0;
			}

		}
	}
    return (m_error==SUCCESSED);
}