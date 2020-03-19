/**************************************************************
                                                               
                             Virtuality                        
                                                               
                       (c) by MiST Land 2000                   
        ---------------------------------------------------    
   Description: класс для работы с сеткой
                                                                
                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/

#if !defined (__NET_H__)
#define __NET_H__

// заголовки DirectPlay
#define IDIRECTPLAY2_OR_GREATER
#include "dplay.h"
#include "dplobby.h"

// заголовки STL
#include "string"
#include "vector"

             
// идентификатор приложения
extern GUID AppGUID;

// вектор коннектов
typedef std::vector<GUID> CONNECTION_VECTOR;
// итератор вектора коннектов
typedef CONNECTION_VECTOR::iterator CONNECTION_VECTOR_ITERATOR;

// описание сессии
typedef struct {
	// имя сессии (может содержать дополнительную информацию)
	char * pName;
	// короткое и длинное имя игрока
	char * pShortPlayerName;
	char * pLongPlayerName;
	// максимальное количество игроков
	DWORD dwMaxPlayers;
	// текущее количество игроков
	DWORD dwCurrentPlayers;
	// поле определяется пользователем
	DWORD dwUser1;
	// поле определяется пользователем
	DWORD dwUser2;
	// поле определяется пользователем
	DWORD dwUser3;
	// поле определяется пользователем
	DWORD dwUser4;
	// идентификатор сессии
	GUID guid;
} ST_SESSION;
// вектор сессий
typedef std::vector<ST_SESSION> SESSION_VECTOR;
// итератор вектора сессий
typedef SESSION_VECTOR::iterator SESSION_VECTOR_ITERATOR;

//////////////////////////////////////////////////////////////////////////////////
//
// класс сетки
//
//////////////////////////////////////////////////////////////////////////////////

class Net{
public:
	// типы сообщений
	enum MES_TYPE {
		MT_NOMESSAGE,         // в очереди нет сообщений
		MT_GENERIC,           // обычное сообщение
		MT_CREATEPLAYER,      // системное сообщение - создан игрок
		MT_DESTROYPLAYER,     // системное сообщение - уничтожен игрок
		MT_SESSIONLOST,       // системное сообщение - потеряно соединение с сессией
		MT_CHANGEDPLAYERDATA, // системное сообщение - изменились данные игрока
		MT_SETPLAYERNAME,     // системное сообщение - изменилось имя игрока
		MT_SETSESSIONDDECS,   // системное сообщение - изменились данные сессии
		MT_UNKNOWN            // неизвестное сообщение
	};
	//
	// открытые функции
	//

	// конструктор
	Net();
	// деструктор
	~Net();

	// получить код последней ошибки
	const HRESULT GetDPError() const { return hr; }
	// получить описание ошибки DirectPlay по ее коду
	std::string GetDPErrorDescription(const HRESULT h);

	// доступен (инициализируется) ли коннект по TCP/IP
	bool IsTCPConnectionAvailable();
	// доступен (инициализируется) ли коннект по IPX
	bool IsIPXConnectionAvailable();
	// доступен (инициализируется) ли коннект через модем
	bool IsModemConnectionAvailable();
	// доступен (инициализируется) ли коннект по кабелю
	bool IsSerialConnectionAvailable();

	// приконнектиться используя TCP/IP
	bool ConnectByTCP(const char * strIPAddr, const WORD wPort = 0);
	// приконнектиться используя IPX
	bool ConnectByIPX();
	// дисконнект от провайдера
	void Disconnect();

	// начать или продолжать поиск сессий
	bool UpdateEnumSessions(const DWORD dwPeriod = 0, const bool bAll = false);
	// остановить поиск сессий
	bool StopEnumSessions(const bool bAll = false);
	// получить вектор сессий
	const SESSION_VECTOR& GetSessionVector() const { return sessions; }
	// заджойниться в сессию
	bool JoinSession(const int Index, char * ShortPlayerName, char * LongPlayerName);
	// создать новую сессию
	bool CreateSession(char * SessionName, char * ShortPlayerName, char * LongPlayerName, DWORD u1 = 0, DWORD u2 = 0, DWORD u3 = 0, DWORD u4 = 0);
	// закрыть сессию
	bool CloseSession();

	// заполнить имена игрока в описании сессии
	bool SetPlayerNames(ST_SESSION* pSS);

	// послать сообщение
	bool SendMessage(void * pBuffer, DWORD dwSize);
	// получить размер сообщения
	bool ReceiveMessage(DWORD* pdw);
	// принять сообщение
	Net::MES_TYPE ReceiveMessage(void * pBuffer, DWORD dwSize);

private:
	//
	// закрытые функции
	//

	// поиск коннектов
	bool EnumConnections();

	// функция обратного вызова для поиска коннектов
	static BOOL CALLBACK EnumConnectionsCallback(LPCGUID pguidSP, VOID *pConnection, DWORD dwConnectionSize, LPCDPNAME pName, DWORD dwFlags, LPVOID pContext);
	// функция обратного вызова для поиска сессий
	static BOOL CALLBACK EnumSessionsCallback(LPCDPSESSIONDESC2 lpThisSD, LPDWORD lpdwTimeOut, DWORD dwFlags, LPVOID lpContext);
	// функция обратного вызова для поиска игроков в удаленной сессии
	static BOOL CALLBACK EnumPlayersCallback( DPID dpId, DWORD dwPlayerType, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext);

	// очистить вектор сессий
	void ClearSessionsVector();

	//
	// закрытые данные
	//

	// содержит код последней ошибки (см. также GetDPErrorDescription() )
	HRESULT hr;
	// вектор коннектов
	CONNECTION_VECTOR connections;
	// объект DirectPlay
	LPDIRECTPLAY4 g_pDP;
	// вектор найденных сессий
	SESSION_VECTOR sessions;
	// идентификатор игрока
	DPID PlayerID;
};

#endif // __NET_H__
