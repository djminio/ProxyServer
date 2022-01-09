#pragma once
#include "protocol.h"
#include "ServerTable.h"

//최대 Listener 접속수 
#define MAX_LISTENER_NUM		50
#define IP_LENGTH				16

//최대 RMTool Login 가능수 (30개의 IP만 허용)
#define MAX_RM_LOGIN			30

//RM_LOGIN 구조체의 bType define
#define RM_TYPE_TOOL			1
#define RM_TYPE_LISTENER		2

//ProxyServer -> Listener 로의 접속할 포트 번호 
#define PROXY_SERVER_CONNECTION_PORT	24694


//RM Packet 헤더 
#define MSG_RM_LOGIN						1
#define MSG_RM_DOWNLOAD_PATH				2
#define MSG_RM_COPY_FILE					3  //SYNC Socket 
#define MSG_RM_LISTENER_PATCH				4
#define MSG_RM_LOGIN_OK						5
#define MSG_RM_LOGIN_FAIL					6
#define MSG_RM_REQUEST_ALL_SEBSERVER_INFO	7
#define MSG_RM_REPLY_ALL_SEBSERVER_INFO		8
#define MSG_RM_SHUTDOWN_SERVER				9
#define MSG_RM_USER_ACCEPT_ALLOWED			10
#define MSG_RM_SHUTDOWN_PROXY				11		//Proxy만 ShutDown
#define MSG_RM_SERVER_DISCONNECTED			12		//Proxy -> RMClient로 
#define MSG_RM_SHUTDOWN_SUBSERVER			13		//Agent,Map,DBDemon등 한가지 종류별로 셧다운 
#define MSG_RM_SERVER_UP					14		//Proxy에 어떠한 서버가 접속했을때..
#define MSG_RM_EXECUTE_SERVER				15		//Proxy Server를 제외한 서버들 실행 
#define MSG_RM_REBOOT_SERVER				16		//서버 재부팅 
#define MSG_RM_PROXY_CONTROL				17		//서버 재부팅 
#define MSG_RM_RELOAD_DATA					18		//Reload Gameserver Data
#define MSG_RM_NOTICE						19		//공지보낼 서버 
#define MSG_RM_CHANGE_WEATHER				20		//날씨 바꿈 
#define MSG_RM_DOWN_SERVER					21		//Server Down 됐다!!
#define MSG_RM_COPY_PATH					22		//Copy해올 네트웍 경로 패스 
#define MSG_RM_REQUEST_ALL_LISTENER_INFO	23		//리스너 정보 요청 
#define MSG_RM_REPLY_ALL_LISTENER_INFO		24
#define MSG_RM_CONNECT_ALL_LISTENER			25
#define MSG_RM_LISTENER_CONNECT_COMPLETE	26
#define MSG_RM_CHECK_LISTENER_CONNECTION	27
#define MSG_RM_CHECK_LISTENER_CONNECTION_RESULT	28
#define MSG_RM_RELOADING_GAMESERVER_DATA	29		//GameServerData Reload 하고 있는중 


#define MSG_RM_BROADCAST_AGENT		50	//Agent Listener 에게만... 
#define MSG_RM_BROADCAST_DBDEMON	51	//DBDEMON Listener 에게만... 
#define MSG_RM_BROADCAST_MAP		52	//MAP Listener 에게만...
#define MSG_LISTENER_EXECUTE_SERVER 53	//Listener에게 서버 실행하라고 메세지 보냄 
#define MSG_LISTENER_REBOOT_SERVER  54	//Listener에게 서버 재부팅하라고 메세지 보냄 



//Proxy < -- > Other Server
#define MSG_SHUT_DOWN				100	//ShutDown
#define MSG_RELOAD_GAMESERVER_DATA  101 //Reload Gameserver data
#define MSG_CHANGE_WEATHER			102	//ChangeWeather(MAP Server)
#define MSG_ECHO					103

#define MSG_RM_KICKOFF_USER			111	// 030224 kyo RM유저 접속 종료 
#define MSG_RM_KICKOFF_USER_ALL		112	// BBD 040110 RM 전체유저 강제 접속종료
#define MSG_RM_KICKOFF_USER_SEVERAL	113	// BBD 040110 RM 정해진 명수의 유저종료
#define MSG_RM_KICKOFF_AGENTCANJOIN	114	// BBD 040110 에이전트->맵 조인 허용

#define CMD_RM_CHANGE_WEATHER		26000
#define MSG_LISTENER_LOGIN			100

#pragma pack(push,1)


struct RMCLIENT_INFO
{
	BYTE			bConnectType;		//OnDisconnectServer에서 Client인지 Listener인지 구분하기 위한 앞에 1Byte	
	DWORD			dwID;				//Listener 고유의 번호이다(IP의 ULONG값으로 한다.).
	DWORD			ConnectionIndex;	//프록시서버에서 접속에 따라 고유로 부여한 번호 
	DWORD			IPAddress;			//접속한 Tool의 IP Address
	
	char			szLoginID[20];		//로그인한 툴 아이디 
	char			szName[25];			//로그인한 유저의 이름 

	RMCLIENT_INFO*		pPrvUserInfo;		//hash 테이블에서 같은 버킷안의 이전 레코드의 주소값  
	RMCLIENT_INFO*		pNextUserInfo;		//hash 테이블에서 같은 버킷안의 다음 레코드의 주소값  
};

struct RM_LISTENER_INFO
{
	BYTE bConnectType;			//OnDisconnectServer에서 Client인지 Listener인지 구분하기 위한 앞에 1Byte
	DWORD dwConnectionIndex;	
//	SERVER_TYPE dwServerType;	//어떤 서버 Type의 리스너이냐...
	char szIP[ IP_LENGTH ];		//해당 IP

	RM_LISTENER_INFO* pPrevInfo;
	RM_LISTENER_INFO* pNextInfo;
};



//---------------------------------  RM Packet -------------------------------------
struct PACKET_CHECK_LISTENER_CONNECTION
{
	BYTE bPtcl;
	BYTE bHeader;
};


struct PACKET_RM_PROXY_CONTROL
{
	BYTE bPtcl;
	BYTE bHeader;
	BYTE bUserAccept;
	BYTE bTryToConnect;
	WORD wMaxUser;
};
struct PACKET_SHUTDOWN_SUBSERVER
{
	BYTE bPtcl;
	BYTE bHeader;
	BYTE bServerType;
	DWORD GetPacketSize() { return (DWORD)3;	}

};

struct PACKET_RM_LOGIN
{
	BYTE	bPtcl;
	BYTE	bHeader;
	char	IP[IP_LENGTH];
	char	ID[20];
	char	Name[25];

	DWORD GetPacketSize(){ return (DWORD)63;	}
};

struct PACKET_REQUEST_ALL_SUBSERVER_INFO
{
	BYTE bPtcl;
	BYTE bHeader;
	BYTE bOpenTemplate;		//새로운 템플릿을 로드 할것인가 안할것인가 하는 정보 
	DWORD dwFrameID;		//응답 메세지를 받을 프레임ID값 (bOpenTemplate이 FALSE일때만)
};


struct PACKET_CONNECT_ALL_LISTENER
{
	BYTE bPtcl;
	BYTE bHeader;
	DWORD dwFrameID;
};

struct PACKET_LISTENER_CONNECT_COMPLETE
{
	BYTE bHeader;
	DWORD dwFrameID;
	DWORD GetPacketSize()	{	return (DWORD)5;	}
	
	PACKET_LISTENER_CONNECT_COMPLETE(DWORD frameID)
	{
		bHeader = MSG_RM_LISTENER_CONNECT_COMPLETE;
		dwFrameID = frameID;
	}
};

struct PACKET_CHECK_LISTENER_CONNECTION_RESULT
{
	BYTE bHeader;
	DWORD dwCount;
	DWORD GetPacketSize()	{	return (DWORD)5;	}

	PACKET_CHECK_LISTENER_CONNECTION_RESULT(DWORD count)
	{
		bHeader = MSG_RM_CHECK_LISTENER_CONNECTION_RESULT;
		dwCount = count;
	}
};

struct PACKET_REQUEST_ALL_LISTENER_INFO
{
	BYTE bPtcl;
	BYTE bHeader;
	DWORD dwFrameID;		//응답 메세지를 받을 프레임ID값 (bOpenTemplate이 FALSE일때만)
};

struct ServerStatusInfo
{
	WORD	wPort;
	DWORD	dwStatus;
	DWORD	dwNumOfUsers;
};

struct PACKET_RM_SERVER_DISCONNECT
{
	BYTE bHeader;
	BYTE bServerSetNum;
	WORD wPort;

	DWORD GetPacketSize()	{ return (DWORD)4;	}

	PACKET_RM_SERVER_DISCONNECT(BYTE bServerSet, WORD wServerPort)
	{
		bHeader = MSG_RM_SERVER_DISCONNECTED;
		bServerSetNum = bServerSet;
		wPort = wServerPort;
	}
};


struct PACKET_RM_SERVER_UP
{
	BYTE bHeader;
	BYTE bServerSetNum;
	WORD wPort;

	DWORD GetPacketSize()	{ return (DWORD)4;	}

	PACKET_RM_SERVER_UP(BYTE bServerSet, WORD wServerPort)
	{
		bHeader = MSG_RM_SERVER_UP;
		bServerSetNum = bServerSet;
		wPort = wServerPort;
	}
};

//로그인 승인과 함께 서버 상태 정보를 같이 넘겨준다. 
struct PACKET_RM_LOGIN_OK
{
	BYTE	bHeader;
	BYTE	bServerSetNum;

	DWORD	GetPacketSize()	{ return (DWORD)2;	}
	PACKET_RM_LOGIN_OK(BYTE num)
	{
		bHeader = MSG_RM_LOGIN_OK;
		bServerSetNum = num;
	}
};

struct PACKET_RM_LOGIN_FAIL
{
	BYTE	bHeader;
	DWORD	GetPacketSize()	{ return (DWORD)1;	}
	PACKET_RM_LOGIN_FAIL()
	{
		bHeader = MSG_RM_LOGIN_FAIL;
	}
};



struct PACKET_LISTENER_SERVER_EXCUTE
{
	BYTE bPtcl;
	BYTE bHeader;
	BYTE bServerType;
	WORD wPort;
	BYTE bFileNameLen;
	char szFileName[ MAX_PATH ];
	
	DWORD GetPacketSize()	{	return (DWORD)(6 + bFileNameLen);	}

	PACKET_LISTENER_SERVER_EXCUTE(BYTE ServerType, WORD port, char* filename)
	{
		bPtcl = PTCL_RM;
		bHeader = MSG_LISTENER_EXECUTE_SERVER;
		bServerType = ServerType;
		wPort = port;
		bFileNameLen = static_cast<BYTE>(strlen(filename));
		memcpy(szFileName, filename, bFileNameLen);
	}
};

struct PACKET_LISTENER_SERVER_REBOOT
{
	BYTE bPtcl;
	BYTE bHeader;

	DWORD GetPacketSize()	{	return (DWORD)2;	}
	PACKET_LISTENER_SERVER_REBOOT()
	{
		bPtcl = PTCL_RM;
		bHeader = MSG_LISTENER_REBOOT_SERVER;
	}
};

//Proxy서버가 Listener에 Accept되었을때 Listener에서 날려주는 서버 이름이담긴 정보 
/*
struct PACKET_LISTENER_LOGIN
{
	BYTE bPtcl;
	BYTE bHeader;
	BYTE bNameLen;
	char szMachineName[20];

	DWORD GetPacketSize()	{ return (3 + bNameLen);	}
	void GetServerName(char* szName)
	{
		memcpy(szName, szMachineName, bNameLen);
	}

};
*/


//------------------------ Proxy -> Other Server
struct PACKET_SHUT_DOWN
{
	BYTE bPtcl;
	BYTE bHeader;

	PACKET_SHUT_DOWN()
	{
		bPtcl = PTCL_RM_FROM_PROXY;
		bHeader = MSG_SHUT_DOWN;
	}

};

struct PACKET_KICKOFF_USER	// 030224 kyo
{
	BYTE  bPtcl;
	BYTE  bHeader;
	char  szUserID[20];

	PACKET_KICKOFF_USER()
	{
		bPtcl	= PTCL_RM_FROM_PROXY;
		bHeader	= MSG_RM_KICKOFF_USER;
	}
};
//<! BBD 040110		전체유저 강퇴를 위한 추가
struct PACKET_KICKOFF_USER_ALL
{
	BYTE bPtcl;
	BYTE bHeader;
	PACKET_KICKOFF_USER_ALL()
	{
		bPtcl	= PTCL_RM_FROM_PROXY;
		bHeader = MSG_RM_KICKOFF_USER_ALL;
	}
};
//> BBD 040110		전체유저 강퇴를 위한 추가

//<! BBD 040110		전체유저 강퇴를 위한 추가
struct PACKET_KICKOFF_USER_SEVERAL
{
	BYTE bPtcl;
	BYTE bHeader;
	PACKET_KICKOFF_USER_SEVERAL()
	{
		bPtcl	= PTCL_RM_FROM_PROXY;
		bHeader = MSG_RM_KICKOFF_USER_SEVERAL;
	}
};
//> BBD 040110		전체유저 강퇴를 위한 추가

//<! BBD 040110		RM 으로 에이전트 로긴 허용
struct PACKET_KICKOFF_AGENTCANJOIN
{
	BYTE bPtcl;
	BYTE bHeader;
	PACKET_KICKOFF_AGENTCANJOIN()
	{
		bPtcl	= PTCL_RM_FROM_PROXY;
		bHeader = MSG_RM_KICKOFF_AGENTCANJOIN;
	}
};
//> BBD 040110		RM 으로 에이전트 로긴 허용
struct PACKET_RELOAD_GAMESERVER_DATA
{
	BYTE bPtcl;
	BYTE bHeader;

	DWORD GetPacketSize()	{	return (DWORD)2;	}
	PACKET_RELOAD_GAMESERVER_DATA()
	{
		bPtcl = PTCL_RM_FROM_PROXY;
		bHeader = MSG_RELOAD_GAMESERVER_DATA;
	}
};

struct PACKET_CHANGE_WEATHER
{
	BYTE bPtcl;
	BYTE bHeader;	
	BYTE bWeather;	//0이면 맑음, 1이면 비, 2이면 눈 
	BYTE bStopWeather;	//WeatherSystem 정지 
	DWORD dwAmount;

	DWORD GetPacketSize()	{	return (DWORD)8;	}
	PACKET_CHANGE_WEATHER(BYTE weather, BYTE stop, DWORD amount)
	{
		bPtcl = PTCL_RM_FROM_PROXY;
		bHeader = MSG_CHANGE_WEATHER;
		bWeather = weather;
		bStopWeather = stop;
		dwAmount = amount;
	}
};


//서버 다운 될때 알림 메세지 
struct PACKET_DOWN_SERVER
{
	BYTE bHeader;
	BYTE bServerSetNum;
	WORD wPort;

	DWORD GetPacketSize()	{	return (DWORD)4;	}
	PACKET_DOWN_SERVER(BYTE serversetnum, WORD port)
	{
		bHeader = MSG_RM_DOWN_SERVER;
		bServerSetNum = serversetnum;
		wPort = port;
	}
};


struct PACKET_RELOADING_GAMESERVER_DATA
{
	BYTE bHeader;
	BYTE bServerSetNum;
	BYTE bStart;	//RELOAD를 시작 한거면 TRUE; 끝난거면 FALSE;
	WORD wPort;

	DWORD GetPacketSize()	{	return (DWORD)5;	}
	PACKET_RELOADING_GAMESERVER_DATA(BYTE start, BYTE serversetnum, WORD port)
	{
		bHeader = MSG_RM_RELOADING_GAMESERVER_DATA;
		bServerSetNum = serversetnum;
		bStart = start;
		wPort = port;
	}

};


struct PACKET_ECHO
{
	BYTE bPtcl;
	BYTE bHeader;	
	BYTE bBucketIndex;
	WORD wPort;
	DWORD dwEchoID;


	DWORD GetPacketSize() { return (DWORD)9;	}
	PACKET_ECHO(BYTE bucket, WORD port, DWORD echo)
	{
		bPtcl = PTCL_RM_FROM_PROXY;
		bHeader = MSG_ECHO;
		bBucketIndex = bucket;
		wPort = port;
		dwEchoID = echo;
	}
};

struct EchoBucket
{
	WORD wPort;
	BYTE bSended;
	BYTE bReceived;
};
#pragma pack(pop)
