/***********************************************************************

                               Alfa Project

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   10.12.2001

************************************************************************/
#include "Precomp.h"
#include "Exception.h"

#if defined(_DEBUG) && defined(_BREAK_ON_EXCEPTION)
#define	MAKE_DEBUG_BREAK	__asm int 3
#else
#define	MAKE_DEBUG_BREAK
#endif

/////////////////////////////////////////////////////////////////////////
//////////////////////    class CasusImprovisus    //////////////////////
/////////////////////////////////////////////////////////////////////////
CasusImprovisus::CasusImprovisus() {DbgBreak();}

CasusImprovisus::CasusImprovisus(const std::string msg){DbgBreak();}

CasusImprovisus::CasusImprovisus(const std::string file,unsigned int line,const std::string msg){DbgBreak();}

CasusImprovisus::CasusImprovisus(const NoBreak&) {}

CasusImprovisus::CasusImprovisus(const std::string msg,const NoBreak&) {}

CasusImprovisus::CasusImprovisus(const std::string file,unsigned int line,const std::string msg,const NoBreak&) {}

CasusImprovisus::~CasusImprovisus() {}
//	получить строковое содержимое исключения
const char* CasusImprovisus::Content(void) {return "Unknown exception.";}
//	поставить точку останова
void CasusImprovisus::DbgBreak(void)
{
	MAKE_DEBUG_BREAK
}

/////////////////////////////////////////////////////////////////////////
////////////////////////    class CommonCasus    ////////////////////////
/////////////////////////////////////////////////////////////////////////
CommonCasus::CommonCasus(const std::string file,unsigned int line,const std::string msg) : CasusImprovisus(file,line,msg),m_Content(msg)
{
#if defined(_HOME_VERSION) || defined(_DEBUG)
	static unsigned int counter = 0;
	FILE* target_file;
	char buff[MAX_PATH];
	struct tm* now;
    time_t ltime;

	time(&ltime);
	now = localtime(&ltime); 
	sprintf(buff,"Casus_%0.2d%0.2d_%0.2d%0.2d_%0.4d.log",now->tm_mday,now->tm_mon+1,now->tm_hour,now->tm_min,counter++);
	if((target_file = fopen(buff,"wb")))
	{
		std::ostringstream formatStream;
		struct tm* now;
		time_t ltime;
		
		time(&ltime);
		now = localtime(&ltime); 
		
		formatStream << "Time:     " << ctime(&ltime);
		formatStream << "CmdLine: " << GetCommandLine() << "\n\n";
		formatStream << "Content: " << msg << "\n";
		formatStream << "File: " << file << "\n";
		formatStream << "Line: " << line;

	    formatStream << "\n\nCall Stack:\n" << StackGuard::GetAllNames();
		
		fputs(formatStream.str().c_str(),target_file);
		fclose(target_file);
	}
#endif
}

const char* CommonCasus::Content(void) {return m_Content.c_str();}
extern unsigned int major_version;
extern unsigned int minor_version;
extern unsigned int build_number;

/////////////////////////////////////////////////////////////////////////
//	class InitSETranslator && class InitSETranslator  ///////////////////
/////////////////////////////////////////////////////////////////////////
namespace
{	

	/////////////////////////////////////////////////////////////////////////
	////////////////////////    class SEException    ////////////////////////
	/////////////////////////////////////////////////////////////////////////
	class SEException : public CasusImprovisus
	{
	private:
		std::string m_Content;
	public:
		SEException(unsigned n, _EXCEPTION_POINTERS* ptrs) : CasusImprovisus(),m_Content(std::string("System Exception: ")+GetExceptionDesc(ptrs->ExceptionRecord->ExceptionCode))
		{
			static unsigned int counter = 0;
			FILE* target_file;
			char buff[MAX_PATH];
			struct tm* now;
			time_t ltime;

			if(!counter)
			{
				time(&ltime);
				now = localtime(&ltime); 
				sprintf(buff,"Crush_%0.2d%0.2d_%0.2d%0.2d_%0.4d.log",now->tm_mday,now->tm_mon+1,now->tm_hour,now->tm_min,counter++);
				if((target_file = fopen(buff,"wb")))
				{
					std::ostringstream formatStream;
					struct tm* now;
					time_t ltime;
					
					time(&ltime);
					now = localtime(&ltime); 
					
					char buff[100]; 
					
					#if defined(_HOME_VERSION)
					sprintf(buff,"Ver. %d.%0.2d.%0.3d domestic",major_version,minor_version,build_number);
					#endif
					
					#if defined(DEMO_VERSION)
					sprintf(buff,"Demo version");
					#else
					sprintf(buff,"Ver. %d.%0.2d.%0.3d",major_version,minor_version,build_number);
					#endif
					
					formatStream << "Version Id: " << buff << "\n\n";
					
					formatStream << "Time:     " << ctime(&ltime);
					formatStream << "CmdLine: " << GetCommandLine() << "\n\n";
					formatStream << m_Content;
					formatStream << "\n\nRegistres:\n\n";
					
					DumpRegs(formatStream,ptrs->ContextRecord);
					
					formatStream << "\n\nCall Stack:\n" << StackGuard::GetAllNames();
					
					fputs(formatStream.str().c_str(),target_file);
					fclose(target_file);
				}
			}
		}
	private:
		//	получить описание исключения
		const char* GetExceptionDesc(DWORD code)
		{
			switch (code)
			{
			case EXCEPTION_ACCESS_VIOLATION:        return "Access violation.";
			case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:   return "The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking.";
			case EXCEPTION_BREAKPOINT:              return "A breakpoint was encountered.";
			case EXCEPTION_DATATYPE_MISALIGNMENT:   return "The thread tried to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on.";
			case EXCEPTION_FLT_DENORMAL_OPERAND:    return "One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value.";
			case EXCEPTION_FLT_DIVIDE_BY_ZERO:      return "The thread tried to divide a floating-point value by a floating-point divisor of zero.";
			case EXCEPTION_FLT_INEXACT_RESULT:      return "The result of a floating-point operation cannot be represented exactly as a decimal fraction.";
			case EXCEPTION_FLT_INVALID_OPERATION:   return "This exception represents any floating-point exception not included in this list.";
			case EXCEPTION_FLT_OVERFLOW:            return "The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type.";
			case EXCEPTION_FLT_STACK_CHECK:         return "The stack overflowed or underflowed as the result of a floating-point operation.";
			case EXCEPTION_FLT_UNDERFLOW:           return "The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.";
			case EXCEPTION_ILLEGAL_INSTRUCTION:     return "The thread tried to execute an invalid instruction.";
			case EXCEPTION_IN_PAGE_ERROR:           return "The thread tried to access a page that was not present, and the system was unable to load the page. For example, this exception might occur if a network connection is lost while running a program over the network.";
			case EXCEPTION_INT_DIVIDE_BY_ZERO:      return "The thread tried to divide an integer value by an integer divisor of zero.";
			case EXCEPTION_INT_OVERFLOW:            return "The result of an integer operation caused a carry out of the most significant bit of the result.";
			case EXCEPTION_INVALID_DISPOSITION:     return "An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception.";
			case EXCEPTION_NONCONTINUABLE_EXCEPTION:return "The thread tried to continue execution after a noncontinuable exception occurred.";
			case EXCEPTION_PRIV_INSTRUCTION:        return "The thread tried to execute an instruction whose operation is not allowed in the current machine mode.";
			case EXCEPTION_SINGLE_STEP:             return "A trace trap or other single-instruction mechanism signaled that one instruction has been executed.";
			case EXCEPTION_STACK_OVERFLOW:          return "The thread used up its stack.";
			default:								return "Unknown.";
			}
		}
		//	дамп регистров
		void DumpRegs(std::ostringstream& fs,PCONTEXT context)
		{
			char buffer[200];
			
			sprintf(buffer,"CS:EIP %04X:%08X\n",context->SegCs,context->Eip); fs << buffer;
			sprintf(buffer,"SS:ESP %04X:%08X\n",context->SegSs,context->Esp); fs << buffer;
			sprintf(buffer,"DS:ESI %04X:%08X\n",context->SegDs,context->Esi); fs << buffer;
			sprintf(buffer,"ES:EDI %04X:%08X\n",context->SegEs,context->Edi); fs << buffer;
			sprintf(buffer,"EAX:%08X EBX:%08X \nECX:%08X EDX:%08X\n",context->Eax,context->Ebx,context->Ecx,context->Edx); fs << buffer;
			sprintf(buffer,"EBP:%08X \n",context->Ebp); fs << buffer;
		}
		//	получить описание исключения
		const char* Content(void) {return m_Content.c_str();}
	};

	//	обработчик системных исключений
	void SETranslator(unsigned n, _EXCEPTION_POINTERS* ptrs) 
	{ 
		throw SEException(n,ptrs); 
	}
#if (!defined(_DEBUG) && !defined(_HOME_VERSION))
	class InitSETranslator {public: InitSETranslator() {_set_se_translator(SETranslator);} } m_InitSETranslator;
#endif//_DEBUG
}
