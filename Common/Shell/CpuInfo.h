/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   28.07.2000

************************************************************************/
#ifndef _CPU_INFO_H_
#define _CPU_INFO_H_

//	Наименование производителя процессора
#define		VENDOR_INTEL	"GenuineIntel"
#define		VENDOR_AMD		"AuthenticAMD"
#define		VENDOR_CYRIX	"CyrixInstead"

//	Наименование процессора от Intel
#define		INTEL_486SL		"486(TM) SL processor"
#define		INTEL_DX2WB		"Write-Back Enhanced DX2(TM) processor"
#define		INTEL_DX4		"DX4(TM) processor"
#define		INTEL_DX4O		"DX4(TM) OverDrive(R) processor"
#define		INTEL_P			"Pentium(R) processor"
#define		INTEL_PO		"Pentium(R) OverDrive(R) processor"
#define		INTEL_PPRO		"Pentium(R) Pro processor"
#define		INTEL_PII		"Pentium(R) II processor"
#define		INTEL_CELERON	"Celeron(TM) processor"
#define		INTEL_PIII		"Pentium(R) III processor"
#define		INTEL_XEON		"Pentium(R) III Xeon processor"
#define		INTEL_PIIO		"Pentium(R) II OverDrive(R) processor"

#define     PROCESSOR_UNKNOWN "Unknown processor"

class CpuInfo
{
public:
	unsigned int m_bSupport;				//	флаг поддержки команды CPUID
	char m_sVendor[13];						//	название производителя
	char m_sCpuName[48];					//	название процессора
	//	сигнатура процессора
	int m_iStepping;						//	номер разработки микропроцессора
	int m_iModel;							//	модель микропроцессора
	int m_iFamily;							//	номер семейства микропроцессора
	int m_iType;							//	тип микропроцессора 
	//	флаги возможностей процессора
	unsigned int m_bFPU;					//	наличие FPU
	unsigned int m_bMMX;					//	наличие MMX инструкций
	unsigned int m_b3DNow;					//	поддержка технологии 3DNow! (только для AMD)
	unsigned int m_bTSC;					//	поддержка команды RDTSC и бита CR4.TSD
	unsigned int m_bSIMD;					//	поддержка расширений потоковой обработки SIMD
	unsigned int m_bCMOV;					//	поддержка команды CMOV и при наличии интегрированного 
											//  FPU (m_bFPU==true) - команд FCMOV и FCOMI
public:
	CpuInfo();
	virtual ~CpuInfo();
public:
	void Info(void);
private:
	void GetCPUIDStringAMD(unsigned long funcNum, char *String);
	const char* GetIntelProcName(void);

};

#endif	//_CPU_INFO_H_