#pragma once
#include "protocol.h"
#include "ServerTable.h"

//�ִ� Listener ���Ӽ� 
#define MAX_LISTENER_NUM		50
#define IP_LENGTH				16

//�ִ� RMTool Login ���ɼ� (30���� IP�� ���)
#define MAX_RM_LOGIN			30

//RM_LOGIN ����ü�� bType define
#define RM_TYPE_TOOL			1
#define RM_TYPE_LISTENER		2

//ProxyServer -> Listener ���� ������ ��Ʈ ��ȣ 
#define PROXY_SERVER_CONNECTION_PORT	24694


//RM Packet ��� 
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
#define MSG_RM_SHUTDOWN_PROXY				11		//Proxy�� ShutDown
#define MSG_RM_SERVER_DISCONNECTED			12		//Proxy -> RMClient�� 
#define MSG_RM_SHUTDOWN_SUBSERVER			13		//Agent,Map,DBDemon�� �Ѱ��� �������� �˴ٿ� 
#define MSG_RM_SERVER_UP					14		//Proxy�� ��� ������ ����������..
#define MSG_RM_EXECUTE_SERVER				15		//Proxy Server�� ������ ������ ���� 
#define MSG_RM_REBOOT_SERVER				16		//���� ����� 
#define MSG_RM_PROXY_CONTROL				17		//���� ����� 
#define MSG_RM_RELOAD_DATA					18		//Reload Gameserver Data
#define MSG_RM_NOTICE						19		//�������� ���� 
#define MSG_RM_CHANGE_WEATHER				20		//���� �ٲ� 
#define MSG_RM_DOWN_SERVER					21		//Server Down �ƴ�!!
#define MSG_RM_COPY_PATH					22		//Copy�ؿ� ��Ʈ�� ��� �н� 
#define MSG_RM_REQUEST_ALL_LISTENER_INFO	23		//������ ���� ��û 
#define MSG_RM_REPLY_ALL_LISTENER_INFO		24
#define MSG_RM_CONNECT_ALL_LISTENER			25
#define MSG_RM_LISTENER_CONNECT_COMPLETE	26
#define MSG_RM_CHECK_LISTENER_CONNECTION	27
#define MSG_RM_CHECK_LISTENER_CONNECTION_RESULT	28
#define MSG_RM_RELOADING_GAMESERVER_DATA	29		//GameServerData Reload �ϰ� �ִ��� 


#define MSG_RM_BROADCAST_AGENT		50	//Agent Listener ���Ը�... 
#define MSG_RM_BROADCAST_DBDEMON	51	//DBDEMON Listener ���Ը�... 
#define MSG_RM_BROADCAST_MAP		52	//MAP Listener ���Ը�...
#define MSG_LISTENER_EXECUTE_SERVER 53	//Listener���� ���� �����϶�� �޼��� ���� 
#define MSG_LISTENER_REBOOT_SERVER  54	//Listener���� ���� ������϶�� �޼��� ���� 



//Proxy < -- > Other Server
#define MSG_SHUT_DOWN				100	//ShutDown
#define MSG_RELOAD_GAMESERVER_DATA  101 //Reload Gameserver data
#define MSG_CHANGE_WEATHER			102	//ChangeWeather(MAP Server)
#define MSG_ECHO					103

#define MSG_RM_KICKOFF_USER			111	// 030224 kyo RM���� ���� ���� 
#define MSG_RM_KICKOFF_USER_ALL		112	// BBD 040110 RM ��ü���� ���� ��������
#define MSG_RM_KICKOFF_USER_SEVERAL	113	// BBD 040110 RM ������ ����� ��������
#define MSG_RM_KICKOFF_AGENTCANJOIN	114	// BBD 040110 ������Ʈ->�� ���� ���

#define CMD_RM_CHANGE_WEATHER		26000
#define MSG_LISTENER_LOGIN			100

#pragma pack(push,1)


struct RMCLIENT_INFO
{
	BYTE			bConnectType;		//OnDisconnectServer���� Client���� Listener���� �����ϱ� ���� �տ� 1Byte	
	DWORD			dwID;				//Listener ������ ��ȣ�̴�(IP�� ULONG������ �Ѵ�.).
	DWORD			ConnectionIndex;	//���Ͻü������� ���ӿ� ���� ������ �ο��� ��ȣ 
	DWORD			IPAddress;			//������ Tool�� IP Address
	
	char			szLoginID[20];		//�α����� �� ���̵� 
	char			szName[25];			//�α����� ������ �̸� 

	RMCLIENT_INFO*		pPrvUserInfo;		//hash ���̺��� ���� ��Ŷ���� ���� ���ڵ��� �ּҰ�  
	RMCLIENT_INFO*		pNextUserInfo;		//hash ���̺��� ���� ��Ŷ���� ���� ���ڵ��� �ּҰ�  
};

struct RM_LISTENER_INFO
{
	BYTE bConnectType;			//OnDisconnectServer���� Client���� Listener���� �����ϱ� ���� �տ� 1Byte
	DWORD dwConnectionIndex;	
//	SERVER_TYPE dwServerType;	//� ���� Type�� �������̳�...
	char szIP[ IP_LENGTH ];		//�ش� IP

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
	BYTE bOpenTemplate;		//���ο� ���ø��� �ε� �Ұ��ΰ� ���Ұ��ΰ� �ϴ� ���� 
	DWORD dwFrameID;		//���� �޼����� ���� ������ID�� (bOpenTemplate�� FALSE�϶���)
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
	DWORD dwFrameID;		//���� �޼����� ���� ������ID�� (bOpenTemplate�� FALSE�϶���)
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

//�α��� ���ΰ� �Բ� ���� ���� ������ ���� �Ѱ��ش�. 
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

//Proxy������ Listener�� Accept�Ǿ����� Listener���� �����ִ� ���� �̸��̴�� ���� 
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
//<! BBD 040110		��ü���� ���� ���� �߰�
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
//> BBD 040110		��ü���� ���� ���� �߰�

//<! BBD 040110		��ü���� ���� ���� �߰�
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
//> BBD 040110		��ü���� ���� ���� �߰�

//<! BBD 040110		RM ���� ������Ʈ �α� ���
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
//> BBD 040110		RM ���� ������Ʈ �α� ���
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
	BYTE bWeather;	//0�̸� ����, 1�̸� ��, 2�̸� �� 
	BYTE bStopWeather;	//WeatherSystem ���� 
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


//���� �ٿ� �ɶ� �˸� �޼��� 
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
	BYTE bStart;	//RELOAD�� ���� �ѰŸ� TRUE; �����Ÿ� FALSE;
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
