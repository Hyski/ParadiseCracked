/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   28.07.2000

************************************************************************/
#include "precomp.h"

#include "CpuInfo.h"

CpuInfo::CpuInfo()
{
	//	присваиваем начальные значения переменным
	strcpy(m_sCpuName,PROCESSOR_UNKNOWN);
	//	-----------------------------------------
	m_bSupport = false;
	m_iStepping = 0;
	m_iModel = 0;
	m_iFamily = 0;
	m_iType = 0;
	//	-----------------------------------------
	m_bFPU	 = false;
	m_bMMX	 = false;
	m_b3DNow = false;
	m_bTSC	 = false;
	m_bSIMD  = false;
	m_bCMOV  = false;
	//	извлекаем информацию о процессоре
	Info();
}

void CpuInfo::Info(void)
{
	unsigned int cpuid_max_info = 0;
	unsigned int cpuid_max_extinfo = 0;

	//	проверяем поддержку команды CPUID
	__asm
	{
		pushfd							; EFLAGS копировать в стек
		pop eax							; выбрать в EAX содержимое стека
		mov ebx,eax						; сохранить выбранное значение флагов
		xor eax,200000h					; изменить бит 21 в выбранном значении флагов
		push eax						; поместить новое значение флагов в стек
		popfd							; выбрать в EFLAGS из стека новые значения флагов
		pushfd							; EFLAGS копировать в стек
		pop eax							; выбрать в EAX содержимое стека
		xor eax,ebx						; сравнить считанное новое значение флагов с сохраненным ранее
		je no_cpuid						; переход, если бит 21 не изменился
		mov esi,this					; заносим в регистр ESI адрес этого класса
		mov [esi].m_bSupport,1			; заносим в переменную m_bSupport единицу
no_cpuid:
	}

	//	если CPUID поддерживается, продолжаем
	if(m_bSupport)
	{
		//	определяем максимальное значение понимаемое командой CPUID
		//	и имя производителя
		for(int i=0;i<4;i++)
		{
			switch(i)
			{
			case 0:
				__asm
				{
					mov eax,0					; заносим в EAX ноль
					cpuid						; вызываем команду CPUID
					mov cpuid_max_info,eax		; запоминаем максимальное значение понимаемое командой CPUID
					mov esi,this				; заносим в регистр ESI адрес этого класса
					lea esi,[esi].m_sVendor		; заносим в регистр ESI адрес члена класса m_sVendor
					mov [esi],ebx
					mov [esi+4],edx
					mov [esi+8],ecx
					mov byte ptr[esi+12],0
				}
				break;
			case 1:
				if(cpuid_max_info>0)
				{
					__asm
					{
						//	определяем сигнатуру процессора
						mov eax,1						; заносим в EAX единицу
						cpuid							; вызываем команду CPUID
						mov esi,this					; заносим в регистр ESI адрес этого класса
						mov [esi].m_iStepping,eax		; 
						and [esi].m_iStepping,0000000fh	; нам важны только [3:0] биты
						mov [esi].m_iModel,eax			; 
						shr [esi].m_iModel,4			; 
						and [esi].m_iModel,0000000fh	; нам важны только [7:4] биты
						mov [esi].m_iFamily,eax			; 
						shr [esi].m_iFamily,8			; 
						and [esi].m_iType,0000000fh		; нам важны только [11:8] биты
						shr [esi].m_iType,12			; 
						and [esi].m_iType,00000003h		; нам важны только [13:12] биты
						//	определяем возможности процессора						
						bt  edx,0h						; проверка бита [1]  
						adc [esi].m_bFPU,0				;
						bt  edx,4h						; проверка бита [4]  
						adc [esi].m_bTSC,0				;
						bt  edx,0fh						; проверка бита [15]  
						adc [esi].m_bCMOV,0				;
						bt  edx,17h						; проверка бита [23]  
						adc [esi].m_bMMX,0				;
						bt  edx,19h						; проверка бита [25]  
						adc [esi].m_bSIMD,0				;
					}
					//	обработка процессора AMD
					if(strcmp(m_sVendor,VENDOR_AMD)==0)
					{
						//	проверка на наличие технологии 3DNow!
						__asm
						{
							//	проверка поддержки расширенных режимов
							mov eax,80000000h			; заносим в EAX 0x80000000
							cpuid						; вызываем команду CPUID
							mov cpuid_max_extinfo,eax	; запоминаем максимальное значение понимаемое командой CPUID
						}
						if(cpuid_max_extinfo>0x80000000)
						{
							__asm
							{
								mov esi,this				; заносим в регистр ESI адрес этого класса
								mov eax,80000001h			; заносим в EAX 0x80000001
								cpuid						; вызываем команду CPUID
								bt edx,1fh					; проверка бита 31
								adc [esi].m_b3DNow,0		;
							}
						}
						//	получение наименования процессора
						GetCPUIDStringAMD(0x80000002,m_sCpuName);
						GetCPUIDStringAMD(0x80000003,m_sCpuName+16);
						GetCPUIDStringAMD(0x80000004,m_sCpuName+32);
					}
					//	обработка процессора Intel
					if(strcmp(m_sVendor,VENDOR_INTEL)==0)
					{
						//	получение наименования процессора
						strcpy(m_sCpuName,GetIntelProcName());
					}
				}
				break;
			}
		}
	}
}

CpuInfo::~CpuInfo()
{
}

void CpuInfo::GetCPUIDStringAMD(unsigned long funcNum, char *String)
{
	unsigned long retval;

	_asm
	{
			mov eax,funcNum
			cpuid
			mov	retval,eax
			mov	eax,String
			mov	dword ptr[eax+4],ebx
			mov	dword ptr[eax+8],ecx
			mov	ebx,retval
			mov	dword ptr[eax+12],edx
			mov	dword ptr[eax],ebx
	}
}

const char* CpuInfo::GetIntelProcName(void)
{
	switch(m_iFamily)
	{
		case 4: if (m_iModel == 4) return INTEL_486SL; 
				if (m_iModel == 7) return INTEL_DX2WB; 
				if (m_iModel == 8) return INTEL_DX4; 
				if ((m_iModel == 8)&&(m_iType==1)) return INTEL_DX4O;
				break;
		case 5:	if (m_iModel == 1) return INTEL_P;
				if (m_iModel == 2) return INTEL_P;
				if ((m_iModel == 1)&&(m_iType==1)) return INTEL_PO;
				if ((m_iModel == 2)&&(m_iType==1)) return INTEL_PO;
				if ((m_iModel == 3)&&(m_iType==1)) return INTEL_PO;
				if (m_iModel == 3) return INTEL_P; 
				if (m_iModel == 4) return INTEL_P; 
				if ((m_iModel == 4)&&(m_iType==1)) return INTEL_PO; 
				break;
		case 6: if (m_iModel == 1) return INTEL_PPRO;
				if (m_iModel == 3) return INTEL_PII; 
				if ((m_iModel == 3)&&(m_iType==1)) return INTEL_PIIO;
				if (m_iModel == 5) return INTEL_PII;
				if (m_iModel == 6) return INTEL_CELERON;
				if (m_iModel == 7) return INTEL_PIII; 
				if (m_iModel == 8) return INTEL_PIII; 
				if (m_iModel == 10) return INTEL_XEON;
				break;
	}

	return PROCESSOR_UNKNOWN; 
}