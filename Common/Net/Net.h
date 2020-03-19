/**************************************************************
                                                               
                             Virtuality                        
                                                               
                       (c) by MiST Land 2000                   
        ---------------------------------------------------    
   Description: ����� ��� ������ � ������
                                                                
                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/

#if !defined (__NET_H__)
#define __NET_H__

// ��������� DirectPlay
#define IDIRECTPLAY2_OR_GREATER
#include "dplay.h"
#include "dplobby.h"

// ��������� STL
#include "string"
#include "vector"

             
// ������������� ����������
extern GUID AppGUID;

// ������ ���������
typedef std::vector<GUID> CONNECTION_VECTOR;
// �������� ������� ���������
typedef CONNECTION_VECTOR::iterator CONNECTION_VECTOR_ITERATOR;

// �������� ������
typedef struct {
	// ��� ������ (����� ��������� �������������� ����������)
	char * pName;
	// �������� � ������� ��� ������
	char * pShortPlayerName;
	char * pLongPlayerName;
	// ������������ ���������� �������
	DWORD dwMaxPlayers;
	// ������� ���������� �������
	DWORD dwCurrentPlayers;
	// ���� ������������ �������������
	DWORD dwUser1;
	// ���� ������������ �������������
	DWORD dwUser2;
	// ���� ������������ �������������
	DWORD dwUser3;
	// ���� ������������ �������������
	DWORD dwUser4;
	// ������������� ������
	GUID guid;
} ST_SESSION;
// ������ ������
typedef std::vector<ST_SESSION> SESSION_VECTOR;
// �������� ������� ������
typedef SESSION_VECTOR::iterator SESSION_VECTOR_ITERATOR;

//////////////////////////////////////////////////////////////////////////////////
//
// ����� �����
//
//////////////////////////////////////////////////////////////////////////////////

class Net{
public:
	// ���� ���������
	enum MES_TYPE {
		MT_NOMESSAGE,         // � ������� ��� ���������
		MT_GENERIC,           // ������� ���������
		MT_CREATEPLAYER,      // ��������� ��������� - ������ �����
		MT_DESTROYPLAYER,     // ��������� ��������� - ��������� �����
		MT_SESSIONLOST,       // ��������� ��������� - �������� ���������� � �������
		MT_CHANGEDPLAYERDATA, // ��������� ��������� - ���������� ������ ������
		MT_SETPLAYERNAME,     // ��������� ��������� - ���������� ��� ������
		MT_SETSESSIONDDECS,   // ��������� ��������� - ���������� ������ ������
		MT_UNKNOWN            // ����������� ���������
	};
	//
	// �������� �������
	//

	// �����������
	Net();
	// ����������
	~Net();

	// �������� ��� ��������� ������
	const HRESULT GetDPError() const { return hr; }
	// �������� �������� ������ DirectPlay �� �� ����
	std::string GetDPErrorDescription(const HRESULT h);

	// �������� (����������������) �� ������� �� TCP/IP
	bool IsTCPConnectionAvailable();
	// �������� (����������������) �� ������� �� IPX
	bool IsIPXConnectionAvailable();
	// �������� (����������������) �� ������� ����� �����
	bool IsModemConnectionAvailable();
	// �������� (����������������) �� ������� �� ������
	bool IsSerialConnectionAvailable();

	// ��������������� ��������� TCP/IP
	bool ConnectByTCP(const char * strIPAddr, const WORD wPort = 0);
	// ��������������� ��������� IPX
	bool ConnectByIPX();
	// ���������� �� ����������
	void Disconnect();

	// ������ ��� ���������� ����� ������
	bool UpdateEnumSessions(const DWORD dwPeriod = 0, const bool bAll = false);
	// ���������� ����� ������
	bool StopEnumSessions(const bool bAll = false);
	// �������� ������ ������
	const SESSION_VECTOR& GetSessionVector() const { return sessions; }
	// ������������ � ������
	bool JoinSession(const int Index, char * ShortPlayerName, char * LongPlayerName);
	// ������� ����� ������
	bool CreateSession(char * SessionName, char * ShortPlayerName, char * LongPlayerName, DWORD u1 = 0, DWORD u2 = 0, DWORD u3 = 0, DWORD u4 = 0);
	// ������� ������
	bool CloseSession();

	// ��������� ����� ������ � �������� ������
	bool SetPlayerNames(ST_SESSION* pSS);

	// ������� ���������
	bool SendMessage(void * pBuffer, DWORD dwSize);
	// �������� ������ ���������
	bool ReceiveMessage(DWORD* pdw);
	// ������� ���������
	Net::MES_TYPE ReceiveMessage(void * pBuffer, DWORD dwSize);

private:
	//
	// �������� �������
	//

	// ����� ���������
	bool EnumConnections();

	// ������� ��������� ������ ��� ������ ���������
	static BOOL CALLBACK EnumConnectionsCallback(LPCGUID pguidSP, VOID *pConnection, DWORD dwConnectionSize, LPCDPNAME pName, DWORD dwFlags, LPVOID pContext);
	// ������� ��������� ������ ��� ������ ������
	static BOOL CALLBACK EnumSessionsCallback(LPCDPSESSIONDESC2 lpThisSD, LPDWORD lpdwTimeOut, DWORD dwFlags, LPVOID lpContext);
	// ������� ��������� ������ ��� ������ ������� � ��������� ������
	static BOOL CALLBACK EnumPlayersCallback( DPID dpId, DWORD dwPlayerType, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext);

	// �������� ������ ������
	void ClearSessionsVector();

	//
	// �������� ������
	//

	// �������� ��� ��������� ������ (��. ����� GetDPErrorDescription() )
	HRESULT hr;
	// ������ ���������
	CONNECTION_VECTOR connections;
	// ������ DirectPlay
	LPDIRECTPLAY4 g_pDP;
	// ������ ��������� ������
	SESSION_VECTOR sessions;
	// ������������� ������
	DPID PlayerID;
};

#endif // __NET_H__
