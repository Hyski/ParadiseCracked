#ifndef _UTILS_H_
#define _UTILS_H_

#define RELEASE(object) {if(object) {object->Release(); object = NULL;}}
#define DESTROY(object) {if(object) {delete object; object = NULL;}}
#define FREE(object)    {if(object) {delete[] object; object = NULL;}}

// для определения размерности массива
// !!! определение массива д.б. доступно в месте использования !!!
#define DIMOF(x) (sizeof(x)/sizeof(x[0]))

#ifdef PI
#undef PI
#endif

//константы с pi
const float PI   = 3.1415926535897932384626433832795f;
const float PIm2 = 6.28318530717958647692528676655901f;
const float PId2 = 1.57079632679489661923132169163975f;
const float PId3 = 1.04719755119659774615421446109317f;
const float PId4 = 0.785398163397448309615660845819876f;

//точность 
const float epsilon = 0.001f;

//константы с cos, sin, tg
const float COS30   = 0.866025403784438646763723170752936f;
const float COS30m2 = 1.73205080756887729352744634150587f;
const float COS60   = 0.5f;
const float TAN30   = 0.577350269189625764509148780501957f;

//оси, вектора & etc.
const point3 AXISX(1,0,0);    //ось Х
const point3 AXISY(0,1,0);    //ось Y
const point3 AXISZ(0,0,1);    //ось Z
const point3 NULLVEC(0,0,0);  //нулевой вектор

struct _D3DMATRIX;
extern const _D3DMATRIX *IDENTITYMAT;

#define TORAD(x) (PI*(x)/180)
#define TODEG(x) (180*(x)/PI)
#define TODXVECTOR(x)  (*((D3DVECTOR*)(&x)))


//	-------------------- Нахождение значения в заданном интервале -----------
template <class T>
inline T saturate(const T& value,const T& min,const T& max)
{
	if(value<min)
		return min;
	if(value>max)
		return max;
	return value;
}
//	-------------------------------------------------------------------------
float __fastcall FastAbs(float a);
float __fastcall FastSin(float a);
float __fastcall FastCos(float a);
float __stdcall FastSqrt(float a);
float __stdcall FastMul(float a,float b);
float __stdcall FastDiv(float a,float b);
float __stdcall FPUDist(float pnt1,float pnt2);

float frand(); //возвращает псевдослучайное число в диапазоне 0.f .. 1.f
BOOL MungeFPCW(WORD *pwOldCW );
void RestoreFPCW(WORD wSave);
void LoadString(FILE *f, std::string *s);
void SaveString(FILE *f, const std::string &s);
void LoadPascalString(FILE *f, std::string *s,bool lower=true);
void SavePascalString(FILE *f, const std::string &s);

void print(char *Template,...);
void status(char *Template,...);
#pragma warning(disable:4035)
inline int GetBit(unsigned Bit)
  {
    _asm
      {
      xor ecx,ecx
      bsf eax,Bit
      cmove eax,ecx
      };
  };
inline SavSlot& operator << (SavSlot& st, const point3& val)
{
    st<<val.x<<val.y<<val.z;    
    return st;
}

inline SavSlot& operator >> (SavSlot& st, point3& val)
{
    st>>val.x>>val.y>>val.z;    
    return st;
}
	 
class ProgressBar
{
private:
	int StringLen;
	int tick_cnt;
public:
	ProgressBar():StringLen(0),tick_cnt(0){};
	void print(char *Template,...)
	{
		while(StringLen){::print("\b \b");--StringLen;}
  char r[255];
  va_list ap;
  va_start(ap,Template);
  vsprintf(r, Template, ap);
  ::print("%s",r);
	StringLen=strlen(r);
	};
	void flush(){StringLen=0;};
	void clear(){while(StringLen){::print("\b \b");--StringLen;}};
  void nl(){::print("\n");StringLen=0;};
  void tick();
};

//из строки удаляются все пробелы
void KillSpaces(std::string &data);

//
// "безопасный" аналог printf
//
std::string mlprintf(const char* str, ...);

// Макросы для генерации сообщений, варнингов и ошибок компилятора
// При двойном щелчке на сообщении в окне билда произойдет переход на строку,
//   в которой было сгенерировано сообщение
#if !defined(__SMART_MESSAGE_MACROS__)
#define MACRO_TO_STR(str)			#str
#define	MACRO_TO_STR2(str)			MACRO_TO_STR(str)
#define	MACRO_PLACE_STAMP			__FILE__"("MACRO_TO_STR2(__LINE__)")"
#define SMART_MESSAGE(msg)			message(MACRO_PLACE_STAMP" : "msg)
#define	SMART_WARNING(msg)			SMART_MESSAGE("warning [Paradise Cracked]: "msg)
#define	SMART_ERROR(msg)			SMART_MESSAGE("error [Paradise Cracked]: "msg)
#define __SMART_MESSAGE_MACROS__
#endif

#endif	//_UTILS_H_
