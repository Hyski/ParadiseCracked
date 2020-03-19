/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ���������� ������ ������ � ������
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "Net.h"
#include "objbase.h"

// ������������� ����������
GUID AppGUID = { 0xf7dbc930, 0x5e5e, 0x11d4, { 0xB1, 0xC2, 0x0, 0x60, 0x52, 0x5, 0x8D, 0x18 } };

// �����������
Net::Net()
{
	g_pDP = 0;
	hr = 0;
	PlayerID = 0;
	EnumConnections();
}

// ����������
Net::~Net()
{
	// �������� ������ ������
	ClearSessionsVector();
	// ������� ������
	if(g_pDP) { g_pDP->Release(); g_pDP = 0; }
}

/////////////////////////////////////////////////////////////////////////////
// ������� ��������� ������ ��� ������ ���������
/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK Net::EnumConnectionsCallback(LPCGUID pguidSP, VOID *pConnection, DWORD dwConnectionSize, LPCDPNAME pName, DWORD dwFlags, LPVOID pContext)
{
    HRESULT       h = 0;
    LPDIRECTPLAY4 pDP = 0;

	// �������� ��������� ������ DirectPlay
	h = CoCreateInstance(CLSID_DirectPlay, 0, CLSCTX_ALL, IID_IDirectPlay4A, (VOID**)&pDP);
	if(h) // �� ������� ������� ������
	{
		((Net *)pContext)->hr = h;
		return false; // ��������� �����
	}

	// ��������, ����� �� ������������������� �������
	h = pDP->InitializeConnection(pConnection, 0);
	// ��������� ������
	if(pDP) pDP->Release();
	if(h)  // ������� ������������������� �� ������� - ��������� �����
	{
		((Net *)pContext)->hr = h;
		return true;
	}

	// ������� �������
	// �������� ���������� � ��� � ������� ���������
	GUID guid = *pguidSP;
	((Net *)pContext)->connections.push_back(guid);
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// ����� ���������
/////////////////////////////////////////////////////////////////////////////
bool Net::EnumConnections()
{
	LPDIRECTPLAY4A pDP;
	// �������� ������ ���������
	connections.clear();
	hr = 0;
	// ������� ������
	hr = CoCreateInstance(CLSID_DirectPlay, 0, CLSCTX_ALL,
		IID_IDirectPlay4A, (VOID**)&pDP );
	if(hr) return false;
	// ����� ��������� � ���������� ���������� � ��� � �������
	hr = pDP->EnumConnections(0, Net::EnumConnectionsCallback, this, 0);
	pDP->Release();
	if(hr) return false;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// �������� �������� ������ DirectPlay �� �� ����
/////////////////////////////////////////////////////////////////////////////
std::string Net::GetDPErrorDescription(const HRESULT h)
{
	std::string str;
	switch (h)
	{
	case S_OK : str = "OK"; break;
	case DPERR_ALREADYINITIALIZED : str = "Already Initialized"; break;
	case DPERR_ACCESSDENIED : str = "Access Denied"; break;
	case DPERR_ACTIVEPLAYERS : str = "Active Players"; break;
	case DPERR_BUFFERTOOSMALL : str = "Buffer Too Small"; break;
	case DPERR_CANTADDPLAYER : str = "Can't Add Player"; break;
	case DPERR_CANTCREATEGROUP : str = "Can't Create Group"; break;
	case DPERR_CANTCREATEPLAYER : str = "Can't Create Player"; break;
	case DPERR_CANTCREATESESSION : str = "Can't Create Session"; break;
	case DPERR_CAPSNOTAVAILABLEYET : str = "Caps Not Available Yet"; break;
	case DPERR_EXCEPTION : str = "Exception"; break;
	case DPERR_GENERIC : str = "Generic"; break;
	case DPERR_INVALIDFLAGS : str = "Invalid Flags"; break;
	case DPERR_INVALIDOBJECT : str = "Invalid Object"; break;
	case DPERR_INVALIDPARAM : str = "Invalid Param"; break;
	case DPERR_INVALIDPLAYER : str = "Invalid Player"; break;
	case DPERR_INVALIDGROUP : str = "Invalid Group"; break;
	case DPERR_NOCAPS : str = "No Caps"; break;
	case DPERR_NOCONNECTION : str = "No Connection"; break;
	case DPERR_OUTOFMEMORY : str = "Out Of Memory"; break;
	case DPERR_NOMESSAGES : str = "No Messages"; break;
	case DPERR_NONAMESERVERFOUND : str = "No Name Server Found"; break;
	case DPERR_NOPLAYERS : str = "No Players"; break;
	case DPERR_NOSESSIONS : str = "No Session"; break;
	case DPERR_PENDING : str = "Pending"; break;
	case DPERR_SENDTOOBIG : str = "Send Too Big"; break;
	case DPERR_TIMEOUT : str = "Timeout"; break;
	case DPERR_UNAVAILABLE : str = "Unavailable"; break;
	case DPERR_UNSUPPORTED : str = "Unsupported"; break;
	case DPERR_BUSY : str = "Busy"; break;
	case DPERR_USERCANCEL : str = "Userr Cancel"; break;
	case DPERR_NOINTERFACE : str = "No Interface"; break;
	case DPERR_CANNOTCREATESERVER : str = "Cannot Create Server"; break;
	case DPERR_PLAYERLOST : str = "Player Lost"; break;
	case DPERR_SESSIONLOST : str = "Session Lost"; break;
	case DPERR_UNINITIALIZED : str = "Uninitialized"; break;
	case DPERR_NONEWPLAYERS : str = "No New Players"; break;
	case DPERR_INVALIDPASSWORD : str = "Invalid Password"; break;
	case DPERR_CONNECTING : str = "Connecting"; break;
	case DPERR_CONNECTIONLOST : str = "Connecting Lost"; break;
	case DPERR_UNKNOWNMESSAGE : str = "Unknown Message"; break;
	case DPERR_CANCELFAILED : str = "Cancle Failed"; break;
	case DPERR_INVALIDPRIORITY : str = "Invalid Priority"; break;
	case DPERR_NOTHANDLED : str = "Not Handled"; break;
	case DPERR_CANCELLED : str = "Canceled"; break;
	case DPERR_ABORTED : str = "Aborted"; break;
	case DPERR_BUFFERTOOLARGE : str = "Buffer Too Large"; break;
	case DPERR_CANTCREATEPROCESS : str = "Can't Create Process"; break;
	case DPERR_APPNOTSTARTED : str = "App Not Started"; break;
	case DPERR_INVALIDINTERFACE : str = "Invalid Interface"; break;
	case DPERR_NOSERVICEPROVIDER : str = "No Service Provider"; break;
	case DPERR_UNKNOWNAPPLICATION : str = "Unknown Application"; break;
	case DPERR_NOTLOBBIED : str = "Not Lobbied"; break;
	case DPERR_SERVICEPROVIDERLOADED : str = "Service Provider Loaded"; break;
	case DPERR_ALREADYREGISTERED : str = "Already Registered"; break;
	case DPERR_NOTREGISTERED : str = "Not Registered"; break;
	case DPERR_AUTHENTICATIONFAILED : str = "Authentication Failed"; break;
	case DPERR_CANTLOADSSPI : str = "Can't Load SSPI"; break;
	case DPERR_ENCRYPTIONFAILED : str = "Encryption Failed"; break;
	case DPERR_SIGNFAILED : str = "Sign Failed"; break;
	case DPERR_CANTLOADSECURITYPACKAGE : str = "Can't Load Security Package"; break;
	case DPERR_ENCRYPTIONNOTSUPPORTED : str = "Encryption"; break;
	case DPERR_CANTLOADCAPI : str = "Can't Load CAPI"; break;
	case DPERR_NOTLOGGEDIN : str = "Not Logged In"; break;
	case DPERR_LOGONDENIED : str = "Logon Denied"; break;
	default : str = "Unexpected Error";
	}
	return str;
}

/////////////////////////////////////////////////////////////////////////////
// �������� (����������������) �� ������� �� TCP/IP
/////////////////////////////////////////////////////////////////////////////
bool Net::IsTCPConnectionAvailable()
{
	if(connections.empty()) return false;
	CONNECTION_VECTOR_ITERATOR i;
	for(i = connections.begin(); i != connections.end(); i++)
	{
		if(IsEqualGUID((*i), DPSPGUID_TCPIP)) return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
// �������� (����������������) �� ������� �� IPX
/////////////////////////////////////////////////////////////////////////////
bool Net::IsIPXConnectionAvailable()
{
	if(connections.empty()) return false;
	CONNECTION_VECTOR_ITERATOR i;
	for(i = connections.begin(); i != connections.end(); i++)
	{
		if(IsEqualGUID((*i), DPSPGUID_IPX)) return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
// �������� (����������������) �� ������� ����� �����
/////////////////////////////////////////////////////////////////////////////
bool Net::IsModemConnectionAvailable()
{
	if(connections.empty()) return false;
	CONNECTION_VECTOR_ITERATOR i;
	for(i = connections.begin(); i != connections.end(); i++)
	{
		if(IsEqualGUID((*i), DPSPGUID_MODEM)) return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
// �������� (����������������) �� ������� �� ������
/////////////////////////////////////////////////////////////////////////////
bool Net::IsSerialConnectionAvailable()
{
	if(connections.empty()) return false;
	CONNECTION_VECTOR_ITERATOR i;
	for(i = connections.begin(); i != connections.end(); i++)
	{
		if(IsEqualGUID((*i), DPSPGUID_SERIAL)) return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
// ��������������� ��������� TCP/IP
/////////////////////////////////////////////////////////////////////////////
bool Net::ConnectByTCP(const char * strIPAddr, const WORD wPort/*=0*/)
{
	hr = 0;
	if(!strIPAddr) return false;
	if(!IsTCPConnectionAvailable()) return false;

	// ����� ��������
	LPVOID pConnection = 0;
	// ������ ������ ��������
	DWORD dwAddressSize = 0;
	// ��������� ����� ������ ��� �������� ������ ��������
	LPDIRECTPLAYLOBBY3A pDPLobby = 0;
	// ������ � ����������� ��� �������� ������ ��������
	DPCOMPOUNDADDRESSELEMENT addressElements[3];
	// ���������� ����������� ��������� � �������
	DWORD dwElementCount = 0;

	// ������� ��������� ������ Lobby
	hr = CoCreateInstance( CLSID_DirectPlayLobby, 0, CLSCTX_ALL,
                           IID_IDirectPlayLobby3A, (VOID**)&pDPLobby );
	if(hr) return false;

	// ������� ����������� ����� ����������, ����� ������������� �������
	// ��� ��������
	addressElements[dwElementCount].guidDataType = DPAID_ServiceProvider;
	addressElements[dwElementCount].dwDataSize   = sizeof(GUID);
	addressElements[dwElementCount].lpData       = (VOID*)&DPSPGUID_TCPIP;
	dwElementCount++;
	// IP �����, �� �������� ����� ������� ����� ������
	addressElements[dwElementCount].guidDataType = DPAID_INet;
	addressElements[dwElementCount].dwDataSize   = lstrlen(strIPAddr) + 1;
	addressElements[dwElementCount].lpData       = (LPVOID)strIPAddr;
	dwElementCount++;
	// ����� �����, ���� ����������
	if(wPort > 0)
	{
		addressElements[dwElementCount].guidDataType = DPAID_INetPort;
		addressElements[dwElementCount].dwDataSize   = sizeof(WORD);
		addressElements[dwElementCount].lpData       = (LPVOID)&wPort;
		dwElementCount++;
	}

	// ������� ������ ������
	hr = pDPLobby->CreateCompoundAddress(addressElements, dwElementCount,
		0, &dwAddressSize);
	if(hr != DPERR_BUFFERTOOSMALL)
	{
		pDPLobby->Release();
		return false;
	}

	// ������� ������
	pConnection = new char[dwAddressSize];
	if(!pConnection)
	{
		pDPLobby->Release();
		hr = DPERR_OUTOFMEMORY;
		return false;
	}

	// ������� �����
	hr = pDPLobby->CreateCompoundAddress(addressElements, dwElementCount,
		pConnection, &dwAddressSize);
	// ��������� ��������� ����� ������
	pDPLobby->Release();
	if(hr)
	{
		delete [] pConnection;
		return false;
	}


	// ������� ������ DirectPlay
	hr = CoCreateInstance(CLSID_DirectPlay, 0, CLSCTX_ALL,
		IID_IDirectPlay4A, (VOID**)&g_pDP );
	if(hr)
	{
		delete [] pConnection;
		return false;
	}

	// ����������������� �������
	hr = g_pDP->InitializeConnection(pConnection, 0);
	if(hr)
	{
		g_pDP->Release();
		g_pDP = 0;
		delete [] pConnection;
		return false;
	}

	delete [] pConnection;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// ��������������� ��������� IPX
/////////////////////////////////////////////////////////////////////////////
bool Net::ConnectByIPX()
{
	hr = 0;
	// ����� ��������
	LPVOID pConnection = 0;
	// ������ ������ ��������
	DWORD dwAddressSize = 0;
	// ��������� ����� ������ ��� �������� ������ ��������
	LPDIRECTPLAYLOBBY3A pDPLobby = 0;
	// ������ � ����������� ��� �������� ������ ��������
	DPCOMPOUNDADDRESSELEMENT addressElements[1];
	// ���������� ����������� ��������� � �������
	DWORD dwElementCount = 0;

	// ������� ��������� ������ Lobby
	hr = CoCreateInstance( CLSID_DirectPlayLobby, 0, CLSCTX_ALL,
                           IID_IDirectPlayLobby3A, (VOID**)&pDPLobby );
	if(hr) return false;

	// ������� ����������� ����� ����������, ����� ������������� �������
	// ��� ��������
	addressElements[dwElementCount].guidDataType = DPAID_ServiceProvider;
	addressElements[dwElementCount].dwDataSize   = sizeof(GUID);
	addressElements[dwElementCount].lpData       = (VOID*)&DPSPGUID_IPX;
	dwElementCount++;

	// ������� ������ ������
	hr = pDPLobby->CreateCompoundAddress(addressElements, dwElementCount,
		0, &dwAddressSize);
	if(hr != DPERR_BUFFERTOOSMALL)
	{
		pDPLobby->Release();
		return false;
	}

	// ������� ������
	pConnection = new char[dwAddressSize];
	if(!pConnection)
	{
		pDPLobby->Release();
		hr = DPERR_OUTOFMEMORY;
		return false;
	}

	// ������� �����
	hr = pDPLobby->CreateCompoundAddress(addressElements, dwElementCount,
		pConnection, &dwAddressSize);
	// ��������� ��������� ����� ������
	pDPLobby->Release();
	if(hr)
	{
		delete [] pConnection;
		return false;
	}

	// ������� ������ DirectPlay
	hr = CoCreateInstance(CLSID_DirectPlay, 0, CLSCTX_ALL,
		IID_IDirectPlay4A, (VOID**)&g_pDP );
	if(hr)
	{
		delete [] pConnection;
		return false;
	}

	// ����������������� �������
	hr = g_pDP->InitializeConnection(pConnection, 0);
	if(hr)
	{
		g_pDP->Release();
		g_pDP = 0;
		delete [] pConnection;
		return false;
	}

	delete [] pConnection;
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// ������� ��������� ������ ��� ������ ������� � ��������� ������
/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK Net::EnumPlayersCallback( DPID dpId, DWORD dwPlayerType, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext)
{
	((ST_SESSION *)lpContext)->pShortPlayerName = new char[lstrlen(lpName->lpszShortNameA) + 1];
	strcpy(((ST_SESSION *)lpContext)->pShortPlayerName, lpName->lpszShortNameA);

	((ST_SESSION *)lpContext)->pLongPlayerName = new char[lstrlen(lpName->lpszLongNameA) + 1];
	strcpy(((ST_SESSION *)lpContext)->pLongPlayerName, lpName->lpszLongNameA);

	return false;
}

/////////////////////////////////////////////////////////////////////////////
// ��������� ����� ������ � �������� ������
/////////////////////////////////////////////////////////////////////////////
bool Net::SetPlayerNames(ST_SESSION* pSS)
{
	hr = 0;
	hr = g_pDP->EnumPlayers(&pSS->guid, Net::EnumPlayersCallback, pSS, DPENUMPLAYERS_SESSION);
	if(hr == DP_OK) return true;
	else return false;
}


/////////////////////////////////////////////////////////////////////////////
// ������� ��������� ������ ��� ������ ������
/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK Net::EnumSessionsCallback(LPCDPSESSIONDESC2 lpThisSD, LPDWORD lpdwTimeOut, DWORD dwFlags, LPVOID lpContext)
{
	if(dwFlags & DPESC_TIMEDOUT)
	{
		*lpdwTimeOut = 0;
		return false;
	}

	// ������� ������
	ST_SESSION ss;
	ss.dwCurrentPlayers = lpThisSD->dwCurrentPlayers;
	ss.dwMaxPlayers = lpThisSD->dwMaxPlayers;
	ss.dwUser1 = lpThisSD->dwUser1;
	ss.dwUser2 = lpThisSD->dwUser2;
	ss.dwUser3 = lpThisSD->dwUser3;
	ss.dwUser4 = lpThisSD->dwUser4;
	ss.guid = lpThisSD->guidInstance;
	ss.pName = new char [lstrlen(lpThisSD->lpszSessionNameA) + 1];
	strcpy(ss.pName, lpThisSD->lpszSessionNameA);

	ss.pShortPlayerName = 0;
	ss.pLongPlayerName = 0;

	((Net *)lpContext)->sessions.push_back(ss);

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// ������ ��� ���������� ����� ������
/////////////////////////////////////////////////////////////////////////////
bool Net::UpdateEnumSessions(const DWORD dwPeriod, const bool bAll)
{
	hr = 0;
	// �������� ������
	ClearSessionsVector();

	if(!g_pDP) return false;

	// �������� ��������� �������� ������
	DPSESSIONDESC2 SessionDesc2;
	ZeroMemory(&SessionDesc2, sizeof(DPSESSIONDESC2));
	SessionDesc2.dwSize = sizeof(DPSESSIONDESC2);
	SessionDesc2.guidApplication = AppGUID;

	DWORD dwFlags = DPENUMSESSIONS_RETURNSTATUS | DPENUMSESSIONS_ASYNC;
	if(bAll) dwFlags |= DPENUMSESSIONS_ALL;
	else dwFlags |= DPENUMSESSIONS_AVAILABLE;

	// �������� ��� ��������� ������� ������
	while(true)
	{
		hr = g_pDP->EnumSessions(&SessionDesc2, dwPeriod,
			Net::EnumSessionsCallback, this, dwFlags);
		if(hr == DP_OK) break; // ��������� ��� ������
		if(hr == DPERR_CONNECTING) continue; // ���� ��� ������
		// ��������� �����-�� ������
		return false;
	}
/*
	// ��������� ����� ������� � ��������� �������
	int n = sessions.size();
	for(int i = 0; i < n; i++) SetPlayerNames(&sessions[i]);
*/
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// ���������� ����� ������
/////////////////////////////////////////////////////////////////////////////
bool Net::StopEnumSessions(const bool bAll)
{
	hr = 0;
	// �������� ������
	ClearSessionsVector();

	if(!g_pDP) return false;

	// �������� ��������� �������� ������
	DPSESSIONDESC2 SessionDesc2;
	ZeroMemory(&SessionDesc2, sizeof(DPSESSIONDESC2));
	SessionDesc2.dwSize = sizeof(DPSESSIONDESC2);
	SessionDesc2.guidApplication = AppGUID;

	DWORD dwFlags = DPENUMSESSIONS_RETURNSTATUS | DPENUMSESSIONS_STOPASYNC | DPENUMSESSIONS_ASYNC;
	if(bAll) dwFlags =  DPENUMSESSIONS_ALL;
	else dwFlags |= DPENUMSESSIONS_AVAILABLE;

	// ��������� ������� ��������� ������
	while(true)
	{
		hr = g_pDP->EnumSessions(&SessionDesc2, 0,
			Net::EnumSessionsCallback, this, dwFlags);
		if(hr == DP_OK) break; // ��������� ��� ������
		if(hr == DPERR_CONNECTING) continue; // ���� ��� ������
		// ��������� �����-�� ������
		return false;
	}

	// ��������� ����� ������� � ��������� �������
	int n = sessions.size();
	for(int i = 0; i < n; i++) SetPlayerNames(&sessions[i]);

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// ������� ������
/////////////////////////////////////////////////////////////////////////////
bool Net::CloseSession()
{
	hr = 0;
	hr = g_pDP->Close();
	if(hr == DP_OK) return true;
	else return false;
}


/////////////////////////////////////////////////////////////////////////////
// �������� ������ ������
/////////////////////////////////////////////////////////////////////////////
void Net::ClearSessionsVector()
{
	SESSION_VECTOR_ITERATOR i;
	// ���������� ������, ������� �������� �������� ������
	for(i = sessions.begin(); i != sessions.end(); i++)
	{
		if((*i).pName) delete (*i).pName;
		if((*i).pShortPlayerName) delete (*i).pShortPlayerName;
		if((*i).pLongPlayerName) delete (*i).pLongPlayerName;
	}
	// �������� ������
	sessions.clear();
}

/////////////////////////////////////////////////////////////////////////////
// ���������� �� ����������
/////////////////////////////////////////////////////////////////////////////
void Net::Disconnect()
{
	// �������� ������ ������
	ClearSessionsVector();
	// ������� ������
	if(g_pDP) { g_pDP->Release(); g_pDP = 0; }
	PlayerID = 0;
	hr = 0;
}

/////////////////////////////////////////////////////////////////////////////
// ������������ � ������
/////////////////////////////////////////////////////////////////////////////
bool Net::JoinSession(const int Index, char * ShortPlayerName, char * LongPlayerName)
{
	hr = 0;
	// �������� ����� �� ������� �������
	if( (Index < 0) || (Index >= sessions.size()) ) return false;
	// ������� �� ������, � ������� ����� ����������
	DPSESSIONDESC2 sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.dwSize = sizeof(sd);
	sd.guidInstance = sessions[Index].guid;

	// ����� ����������, ���� �� ��������� ��� �� ����� ������
	while(true)
	{
		hr = g_pDP->Open(&sd, DPOPEN_JOIN | DPOPEN_RETURNSTATUS);
		if(hr == DP_OK) break; // ��������� ��� ������
		if(hr == DPERR_CONNECTING) continue; // ���� ��� ������
		// ��������� �����-�� ������
		return false;
	}

	// ������������, �������� ������
	DPNAME name;
	name.dwSize = sizeof(name);
	name.dwFlags = 0;
	name.lpszLongNameA = LongPlayerName;
	name.lpszShortNameA = ShortPlayerName;
	hr = g_pDP->CreatePlayer(&PlayerID, &name, 0, 0, 0, 0);
	if(hr) return false;
	else return true;
}

/////////////////////////////////////////////////////////////////////////////
// ������� ����� ������
/////////////////////////////////////////////////////////////////////////////
bool Net::CreateSession(char *SessionName, char *ShortPlayerName, char *LongPlayerName, DWORD u1, DWORD u2, DWORD u3, DWORD u4)
{
	hr = 0;
	// ������� �� ������, ������� ����� ���������
	DPSESSIONDESC2 sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.dwSize = sizeof(sd);
	sd.dwCurrentPlayers = 0;
	sd.dwMaxPlayers = 2;
	sd.dwUser1 = u1;
	sd.dwUser2 = u2;
	sd.dwUser3 = u3;
	sd.dwUser4 = u4;
	sd.guidApplication = AppGUID;
	sd.lpszSessionNameA = SessionName;
	sd.dwFlags = DPSESSION_DIRECTPLAYPROTOCOL | DPSESSION_KEEPALIVE;

	// ����� ���������, ���� �� ��������� ��� �� ����� ������
	while(true)
	{
		hr = g_pDP->Open(&sd, DPOPEN_CREATE | DPOPEN_RETURNSTATUS);
		if(hr == DP_OK) break; // ��������� ��� ������
		if(hr == DPERR_CONNECTING) continue; // ���� ��� ������
		// ��������� �����-�� ������
		return false;
	}

	// ������������, �������� ������
	DPNAME name;
	name.dwSize = sizeof(name);
	name.dwFlags = 0;
	name.lpszLongNameA = LongPlayerName;
	name.lpszShortNameA = ShortPlayerName;
	hr = g_pDP->CreatePlayer(&PlayerID, &name, 0, 0, 0, DPPLAYER_SERVERPLAYER);
	if(hr) return false;
	else return true;
}

/////////////////////////////////////////////////////////////////////////////
// ������� ���������
/////////////////////////////////////////////////////////////////////////////
bool Net::SendMessage(void *pBuffer, DWORD dwSize)
{
	hr = 0;
	hr = g_pDP->SendEx(PlayerID, DPID_ALLPLAYERS,
		DPSEND_GUARANTEED | DPSEND_ASYNC | DPSEND_NOSENDCOMPLETEMSG,
		(LPVOID)pBuffer, dwSize, 0, 0, 0, 0);
	if(hr == DPERR_PENDING) return true;
	if(hr) return false;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// �������� ������ ���������
/////////////////////////////////////////////////////////////////////////////
bool Net::ReceiveMessage(DWORD* pdw)
{
	hr = 0;
	DPID idFrom = 0, idTo = 0;
	hr = g_pDP->Receive(&idFrom, &idTo,
		DPRECEIVE_ALL | DPRECEIVE_PEEK, 0, pdw);

	if(hr == DPERR_NOMESSAGES) // ��� ���������
	{
		*pdw = 0;
		return true;
	}

	if(hr && (hr != DPERR_BUFFERTOOSMALL))  // ������
		return false;
		
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// ������� ���������
/////////////////////////////////////////////////////////////////////////////
Net::MES_TYPE Net::ReceiveMessage(void *pBuffer, DWORD dwSize)
{
	hr = 0;
	DPID idFrom = 0, idTo = 0;
	hr = g_pDP->Receive(&idFrom, &idTo,
		DPRECEIVE_ALL, pBuffer, &dwSize);

	if(hr == DPERR_NOMESSAGES) // ��� ���������
		return Net::MT_NOMESSAGE;
	if(hr)  // ������
		return Net::MT_UNKNOWN;

	if(idFrom != DPID_SYSMSG) // ������� ���������
		return Net::MT_GENERIC;

	// ������ ��������� ���������
	switch( ((DPMSG_GENERIC*)pBuffer)->dwType )
	{
	case DPSYS_CREATEPLAYERORGROUP : return Net::MT_CREATEPLAYER;
	case DPSYS_DESTROYPLAYERORGROUP: return Net::MT_DESTROYPLAYER;
	case DPSYS_SESSIONLOST : return Net::MT_SESSIONLOST;
	case DPSYS_SETPLAYERORGROUPDATA : return Net::MT_CHANGEDPLAYERDATA;
	case DPSYS_SETPLAYERORGROUPNAME : return Net::MT_SETPLAYERNAME;
	case DPSYS_SETSESSIONDESC : return Net::MT_SETSESSIONDDECS;
	default : return Net::MT_UNKNOWN;
	}
}
