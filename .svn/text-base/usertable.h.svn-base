#ifndef USERTABLE_H
#define USERTABLE_H

#include "stdafx.h"
#include "struct.h"
#include "servertable.h"

#define SIZE_OF_USER_DISCONNECT_TICK		5
#define MAX_ALLOWED_SLEEP_TIME				5			// x*15, 75초.

enum USER_STATUS
{
	STATUS_USER_NOT_CERTIFIED		=	0,
	STATUS_USER_ACTIVATED			=	1,
	STATUS_USER_INACTIVATED			=	2,
	STATUS_USER_AWAITING_DISCONNECT	=	3
};

#ifdef __IS_PROXY_SERVER
enum CLIENT_TYPE
{
	CLIENT_TYPE_NORMAL							=	0,
	CLIENT_TYPE_ILLEGAL							=	1,
	CLIENT_TYPE_MANAGER_UNDER_AUTHENTICATION	=	2,
	CLIENT_TYPE_MANAGER							=	3
};

#define MANAGER_CLIENT_FIRST_PACKET_TYPE		421
#define MANAGER_CLIENT_FIRST_PACKET_SIZE		0
#define MANAGER_CLIENT_FIRST_PACKET_CRC			247	//Modified by KBS 020330
#endif

class CConnection;

struct USERINFO;
struct USERINFO_LIST;

struct USERINFO
{
	DWORD			dwConnectionIndex;
	DWORD			dwID;
	DWORD			dwStatus;
	DWORD			dwAddress;

	USERINFO_LIST*	pAwaitingDisconnectUserInfoList;

#ifdef __IS_PROXY_SERVER
	DWORD			dwType;
#endif

#ifdef __IS_AGENT_SERVER
	DWORD			dwSleepProcessTick;

	DWORD			dwMapServerConnectionIndex;
	DWORD			dwDBDemonConnectionIndex;
#endif

	BYTE			bNameLength;

	char			szName[16];
	char			szIP[16];

	bool			bAmILogon;
	bool			bOnConnectUserCalled;

	WORD			wUDPPort;
	WORD			wPort;

	//2001/01/29 zhh
	char			logintime[15];		//로그인 한 시간 저장

	USERINFO*		pPrvUserInfo;
	USERINFO*		pNextUserInfo;
};


struct USERINFO_LIST
{
	DWORD			dwTick;
	USERINFO*		pUserInfo;
	USERINFO_LIST*	pPrvUserInfoList;
	USERINFO_LIST*	pNextUserInfoList;
};

class CUserTable
{

	DWORD					m_wMaxBucketNum;
	DWORD					m_dwUserNum;
	DWORD					m_dwDisconnectTick;

	USERINFO_LIST			*m_pAwaitingDisconnectUserList[SIZE_OF_USER_DISCONNECT_TICK];

	USERINFO**				m_ppInfoTable;
	
	void					RemoveAllUserTable();
	void					AddUserInfo(USERINFO* info);

	DWORD					IncreaseDisconnectTick();

#ifdef __IS_AGENT_SERVER
	DWORD					m_dwSleepProcessTick;
#endif

public:								  
	void					RemoveUserID(DWORD id);
	void					RemoveUser(DWORD dwConnectionIndex);
	void					RemoveUserFromAwaitingDisconnectUserList( USERINFO *pUserInfo );
	void					DisconnectUserImmediately( USERINFO *pUserInfo );
	void					DisconnectUserImmediately( DWORD dwConnectionIndex );
	void					SetTickForSleptTimeProcess( USERINFO *pUserInfo );

	bool					DisconnectUserBySuggest( USERINFO* pUserInfo );
	bool					DisconnectUserBySuggest( USERINFO* pUserInfo, WORD wRajaCmdNum );
	bool					IsUserAvailable( USERINFO* pUserInfo );
	bool					IsUserAvailable( DWORD dwUserID );
	bool					SendToUser( USERINFO* pUserInfo, char* pMsg, DWORD dwLength );
	bool					SendToUser( DWORD dwUserID, char* pMsg, DWORD dwLength );
	bool					SendToUserByConnectionIndex( DWORD dwConnectionIndex, char* pMsg, DWORD dwLength );

	DWORD					AddUser(DWORD dwConnectionIndex);
	DWORD					CloseConnectionWithAwaitingToDisconnect();
	DWORD					GetUserNum();
	DWORD					GetBucketNum(){return m_wMaxBucketNum;}

	USERINFO*				GetUserInfo(DWORD id);
	USERINFO**				GetUserInfoTable(){return m_ppInfoTable;}
	USERINFO_LIST*			GetUserInfoList( DWORD dwTick );

#ifdef __IS_AGENT_SERVER
	DWORD					CloseConnectionWithSleepingUsers( void );
	DWORD					RemoveAllUserByMapServerConnectionIndex(DWORD dwMapServerConnectionIndex);
	DWORD					RemoveAllUserByDBDemonConnectionIndex(DWORD dwDBDemonConnectionIndex);
#endif

	CUserTable( WORD wMaxBucketNum );
	~CUserTable();
};

extern CUserTable* g_pUserTable;

#endif
