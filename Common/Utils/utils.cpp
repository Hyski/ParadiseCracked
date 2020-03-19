#include "Precomp.h"
#include "Utils.h"  

static _D3DMATRIX __Ident(1,0,0,0,
                          0,1,0,0,
                          0,0,1,0,
                          0,0,0,1);
const _D3DMATRIX *IDENTITYMAT=&__Ident;

__declspec(naked) float __fastcall FastAbs(float a) 
  {
	__asm {
		fld		DWORD PTR [esp+4] 
		fabs
		ret 4
	}
}

__declspec(naked) float __fastcall FastSin(float a) 
  {
	__asm 
    {
		fld		DWORD PTR [esp+4] 
		fsin
		ret 4
	}
}

__declspec(naked) float __fastcall FastCos(float a)
  {
	__asm 
    {
		fld		DWORD PTR [esp+4] 
		fcos
		ret 4
	}
}

__declspec(naked) float __stdcall FastSqrt(float a)
  {
	__asm 
    {
		fld		DWORD PTR [esp+4] 
		fsqrt
		ret 4
	}
}

__declspec(naked) float __stdcall FastMul(float a,float b)
  {
	__asm 
    {
		fld		DWORD PTR [esp+4] 
		fmul	DWORD PTR [esp+8] 
		ret 8
	}
}

__declspec(naked) float __stdcall FastDiv(float a,float b)
  {
	__asm 
    {
		fld		DWORD PTR [esp+4] 
		fdiv	DWORD PTR [esp+8] 
		ret 8
	}
}

__declspec(naked) float __stdcall FPUDist(float pnt1,float pnt2)
{
	__asm 
    {
		fld		DWORD PTR [esp+8]
		fld		DWORD PTR [esp+4]
		fld		DWORD PTR [esp+8]
		fld		DWORD PTR [esp+4]
		fmulp	ST(2),ST
		fmulp	ST(2),ST
		faddp	ST(1),ST
		fsqrt
		ret		8
	}
}

BOOL MungeFPCW( WORD *pwOldCW )
  {
  BOOL ret = FALSE; 
  WORD wTemp, wSave; 
  __asm fstcw wSave
    if (wSave & 0x300 ||            // Not single mode
      0x3f != (wSave & 0x3f) ||   // Exceptions enabled
      wSave & 0xC00)              // Not round to nearest mode    
      {
      __asm 
        {
        mov ax, wSave
          and ax, not 300h    ;; single mode
          or  ax, 3fh         ;; disable all exceptions
          and ax, not 0xC00   ;; round to nearest mode
          mov wTemp, ax         
          fldcw   wTemp
        }
      ret = TRUE;
      }
    *pwOldCW = wSave; 
    return ret;
  }
float frand() //возвращает псевдослучайное число в диапазоне 0.f .. 1.f
{
	return (float)rand()/RAND_MAX;
}

void RestoreFPCW(WORD wSave)
  {
  __asm fldcw wSave
  }
void LoadString(FILE *f, std::string *s)
  {
  char t[1000];
  fgets(t,199,f);
  while(t[strlen(t)-1]=='\n'||t[strlen(t)-1]=='\r') t[strlen(t)-1]=0;
  strlwr(t);
  *s=t;
  }
void SaveString(FILE *f, const std::string &s)
  {
    fprintf(f,"%s\n",s.c_str());
  }
void LoadPascalString(FILE *f, std::string *s,bool lower)
  {
  int charnum;
  char t[1000];
  fread(&charnum,1,sizeof(int),f);
  fread(t,1,charnum,f);
  if(lower)strlwr(t);
  *s=t;
  }
void SavePascalString(FILE *f, const std::string &s)
  {
  int charnum;
  charnum=s.size();
   fwrite(&charnum,1,sizeof(int),f);
   fwrite(s.c_str(),1,charnum,f);
  }
void print(char * /*Template*/,...){}
void status(char * /*Template*/,...){}

std::string mlprintf(const char* str, ...)
{
    char buff[1024];
 
    va_list  lst;
    va_start(lst, str);

    vsprintf(buff, str, lst);

    va_end(lst);

    return std::string(buff);
}

void KillSpaces(std::string &data)
  {
  std::string::size_type t=0;
  while((t=data.find_first_of("\t "),t)!=data.npos)
    {
    data.erase(t,1);
    }
  }
