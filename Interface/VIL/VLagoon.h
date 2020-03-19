/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   22.08.2000

************************************************************************/
#ifndef _VLAGOON_H_
#define _VLAGOON_H_

#include <map>
#include <string>
#include <vector>
#include "VRegion.h"
#include "VMouseState.h"
#include "../../Common/GraphPipe/GraphPipe.h"

class VLagoon
{
protected:
	char *m_pName;
	std::string m_sClass;	
protected:			//	������������� ������
	//	����������� ����������
	float m_x;					//	���������� ������-�������� ���� ��������������
	float m_y;
	float m_width;				//	������ � ������ ��������������
	float m_height;
protected:			// ������ �����	
	VRegion m_VirtualRegion;	//	����������� ������������ ������� �����
	VRegion m_ScreenRegion;		//	�������� ������������ ������� �����
protected:
	float m_vResX;				//	����������� ������� ������
	float m_vResY;
	float m_sResX;				//	�������� ������� ������
	float m_sResY;
protected:
	std::map<std::string,VLagoon*> m_mChildren; 		//	����� �������� ��������
	VLagoon *m_pParent;			//	������������ ������
protected:
	//	��� ��� �������� � z �����������
	float m_zBegin;		//	������ ����������� ���������� - ������� �������� Z
	float m_zEnd;		//	����� ����������� ����������
	//	������������ �������� �� z
	std::vector<VLagoon*> m_zBuffer;	
protected:
	//	��� ��� ������� � ���������� ������ ����� � ����������
	bool m_bFocus;				//	������� �� ����� ����� �� ������ ������
	bool m_bFocusRequired;		//	��������� ���������� ����� ����� ������� ��������
protected:
	//	��� ��� �������� � Drag & Drop'��
	bool m_bDrag;
	bool m_bDragged;			//	������ � ������ ������ ���������������
public:
	//	����������� ����������
	float m_xOffsetInDrag;		//	�������� ����� ������
	float m_yOffsetInDrag;		//	�������� ����� ������
protected:
	//	������ ��� ���������
	point3 m_Vertex[4];
	texcoord m_TexCoord[4];
	Primi m_Object;
public:
	std::string m_sShader;
	bool m_bVisible;			//	���������� ������ ������ ��� ���
//	bool m_bEnable;				//	�������� �� ��� ������ ��� ���*/
public:
	VLagoon(const char *pName);
	virtual ~VLagoon();
public:
	//	������� ��������� ��� ����� �������� � ����������� ����������
	virtual void ScreenChanged(float vResX,float vResY,float sResX,float sResY);
	//	������� ������� ����������� ���������
	virtual void SetRegion(float x,float y,float right,float bottom);
	virtual void SetRegion(VPoint *points,unsigned int num);
	//	������� ��������� �������������� (����������� ����������)
	virtual void MoveTo(float x,float y);
	//	������� ��� ������ � ��������� ������
	virtual bool AddChild(VLagoon *pLagoon);
	virtual void DeleteChild(const char *pName);
	virtual void SetParent(VLagoon *pLagoon);
	virtual void BringToTop(const char *pName);
	virtual void BringToBottom(const char *pName);
	unsigned int ChildrenNum(void);		//	���-�� �������� ��������
	//	---------------------------------------------------------------
	virtual void PaintRegion(HDC hdc);
	virtual void Render(GraphPipe *lpGraphPipe);
	virtual void SetVertex(void);
	//	���������, ������������ ����������� ��� �������� ��������� ����
	virtual void OnDeleteChild(VLagoon *pLagoon);
	//	������� ��������� z ���������
	void SetZOrder(float zBegin,float zEnd);
	//	���������, ������������ ����������� �� ����
	virtual void OnMouseIn(VMouseState *pMS);
	virtual void OnMousePresent(VMouseState *pMS);
	virtual void OnMouseOut(VMouseState *pMS);
	//	��������� ��� ������� � ����������� ����
	virtual void OnMouseCapture(bool bCapture);
	//	��������� ������������ � ����� � ��������� ������ �����
	virtual void OnFocusIn(void);
	virtual void OnFocusPresent(void);
	virtual void OnFocusOut(void);
	//	��������������� ���������
	virtual void OnNotify(VLagoon *pLagoon,int iMsg);
	//	��������� ��������� � Drag'n'Drop'��
	virtual bool OnDrag(VLagoon *pLagoon,VMouseState *pMS);	//	������ �� ��������������
	virtual void OnDragEnd(VMouseState *pMS);	//	����������� � ���, ��� ������ ������ ���� ������
	virtual bool OnDrop(VLagoon *pLagoon,VMouseState *pMS);	//	������ �� �����������
public:
	//	�������������� �������
	const char* Name(void);
	float X(void);
	float Y(void);
	float Width(void);
	float Height(void);
	float Right(void);
	float Bottom(void);
	float ZBegin(void);
	float ZEnd(void);
	//	���������� � ������ �����
	bool IsFocusRequired(void);
	// ���� �� �������������� ����� ������� (�������� ����������)
	bool PtInLagoon(float x,float y);
	//	���������� �� ���� � �����  (�������� ����������)
	VLagoon* GetLagoon(float x,float y);
//	VLagoon* GetVLagoon(float x,float y);
	//	������� drag'n'drop
	bool IsDragged(void);
	bool IsCanDragged(void);
	void SetDragged(bool bDragged);
public:
	//	���������� ��������� �� ������ ���������� ������
//	virtual void* This(void) {return this;};
	//	���������� ��������� �� ������� ����� VLagoon
//	VLagoon* VThis(void) {return (VLagoon *)VLagoon::This();};
	bool IsClass(const char *pClass);
};

//*********************************************************************//
inline const char* VLagoon::Name(void)
{
	return m_pName;
}

inline float VLagoon::X(void)
{
	return m_x;
}

inline float VLagoon::Y(void)
{
	return m_y;
}

inline float VLagoon::Width(void)
{
	return m_width;
}

inline float VLagoon::Height(void)
{
	return m_height;
}

inline float VLagoon::Right(void)
{
	return m_x+m_width;
}

inline float VLagoon::Bottom(void)
{
	return m_y+m_height;
}
//	���������� ���������� �� z
inline float VLagoon::ZBegin(void)
{
	return m_zBegin;
}
//	���������� ���������� �� z
inline float VLagoon::ZEnd(void)
{
	return m_zEnd;
}
// ���� �� �������������� ����� ������� (�������� ����������)
inline bool VLagoon::PtInLagoon(float x,float y)
{
	return m_ScreenRegion.PtInRegion(x,y);
}
//	���-�� �������� ��������
inline unsigned int VLagoon::ChildrenNum(void)
{
	return m_mChildren.size();
}
//	�������� �� ��������� ������ �����
inline bool VLagoon::IsFocusRequired(void)
{
	return m_bFocusRequired;
}

inline bool VLagoon::IsDragged(void)
{
	return m_bDragged;
}

inline bool VLagoon::IsCanDragged(void)
{
	return m_bDrag;
}

inline void VLagoon::SetDragged(bool bDragged)
{
	m_bDragged = bDragged;
}


#endif	//_VLAGOON_H_