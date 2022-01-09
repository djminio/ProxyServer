
// ---------------------------------------------------
// Commands
// 20010508 수정
// ---------------------------------------------------

// 서버정보 관련 패킷
#define MANAGER_CMD_TARGET_NOT_AVAILABLE		101
#define MANAGER_CMD_AUTH						102

#define MANAGER_CMD_REQUEST_SERVER_INFO			201
#define MANAGER_CMD_REQUEST_SERVER_SUBLIST		202
#define MANAGER_CMD_REQUEST_SERVER_STATUS		203
#define MANAGER_CMD_REQUEST_SUBSERVER_INFO		204
#define MANAGER_CMD_REQUEST_SUBSERVER_STATUS	205
#define MANAGER_CMD_REQUEST_SERVER_SHUTDOWN		206

// 서버 조정 관련 패킷
#define MANAGER_CMD_CONTROL_USER_ALLOW			301
#define MANAGER_CMD_CONTROL_SERVER_MAXUSER		302
#define MANAGER_CMD_CONTROL_SERVER_LIMITUSER	303

// 공지사항  패킷
#define MANAGER_CMD_PUBLIC_NOTICE				401

// 맵서버에게 보내는 명령
#define	CMD_BBS									8411
#define CMD_BBS_RMTOOL							10641

#pragma once
#include "ServerTable.h"

#if defined(__IS_AGENT_SERVER) || defined(__IS_PROXY_SERVER)
#include "UserTable.h"
#endif

#pragma pack(push, 1)

#define MANAGER_ID			"manager12"
#define MANAGER_PASSWD		"testok32"

typedef struct request_auth
{
	char		szID[16+1];
	char		szPasswd[16+1];
} MGR_REQUEST_AUTH_PACKET, *LP_MGR_REQUEST_AUTH_PACKET;

typedef struct answer_auth_packet
{
	WORD		wPort;

} MGR_ANSWER_AUTH_PACKET, *LP_MGR_ANSWER_AUTH_PACKET;

// Server-Set(Proxy) Packet
typedef struct server_info
{
	DWORD		dwNumOfMaxUser;
	bool		bIsUserAcceptAllow;
	BYTE		NumOfLimit;

} MGR_SERVER_INFO_PACKET, *LP_MGR_SERVER_INFO_PACKET;

typedef struct server_sublistnum
{
	WORD		wSubTotalNum;
	WORD		wPort[200];

} MGR_SERVER_SUBLIST_PACKET, *LP_MGR_SERVER_SUBLIST_PACKET;

// SubServer Packet
typedef struct subserver_request
{
	WORD		wPort;

} MGR_SUBSERVER_REQUEST_PACKET, *LP_MGR_SUBSERVER_REQUEST_PACKET;

typedef struct subserver_info
{
	WORD		wType;
	char		szIp[16];

} MGR_SUBSERVER_INFO_PACKET, *LP_MGR_SUBSERVER_INFO_PACKET;

// Server Control
typedef struct user_allow
{
	bool		bIsUserAcceptAllow;

} MGR_USER_ALLOW_PACKET, *LP_MGR_USER_ALLOW_PACKET;

typedef struct server_maxuser
{
	DWORD		dwNumOfMaxUser;		

} MGR_SERVER_MAXUSER_PACKET, *LP_MGR_SERVER_MAXUSER_PACKET;

typedef struct server_limituser
{
	BYTE		NumOfLimit;

} MGR_SERVER_LIMITUSER_PACKET, *LP_MGR_SERVER_LIMITUSER_PACKET;

typedef struct server_shutdown
{
	WORD		wPort;

} MGR_SERVER_SHUTDOWN_PACKET, *LP_MGR_SERVER_SHUTDOWN_PACKET;

// 서버 공통 정보
typedef struct server_status
{
	DWORD		dwNumOfUsers;
	DWORD		dwStatus;
	WORD		wSubConnectionNum;

} MGR_SERVER_STATUS_PACKET, *LP_MGR_SERVER_STATUS_PACKET;

// 공지사항 전달
typedef struct public_notice
{
	WORD		wPort;
	WORD		wLengthOfMsg;
	char		szMessage[260];

} MGR_PUBLIC_NOTICE_PACKET, *LP_MGR_PUBLIC_NOTICE_PACKET;

//---------------------------------------------
typedef struct manager_packet_header
{
	BYTE		bPTCL;
	WORD		wCMD;
	DWORD		dwCRC;

	union
	{
		WORD		wTargetServerID;
		DWORD		dwTargetManagerID;
	} uTarget;

} MANAGER_PACKET_HEADER, *LP_MANAGER_PACKET_HEADER;

typedef struct manager_packet
{
	MANAGER_PACKET_HEADER h;
	
	union
	{
		MGR_REQUEST_AUTH_PACKET			MgrRequestAuthPacket;
		MGR_ANSWER_AUTH_PACKET			MgrAnswerAuthPacket;
		MGR_SERVER_INFO_PACKET			MgrServerInfoPacket;
		MGR_SERVER_SUBLIST_PACKET		MgrServerSubListPacket;
		MGR_SERVER_STATUS_PACKET		MgrServerStatusPacket;
		MGR_SUBSERVER_REQUEST_PACKET	MgrSubServerRequestPacket;
		MGR_SUBSERVER_INFO_PACKET		MgrSubServerInfoPacket;

		// 서버 조정
		MGR_USER_ALLOW_PACKET			MgrUserAllowPacket;
		MGR_SERVER_MAXUSER_PACKET		MgrServerMaxUserPacket;
		MGR_SERVER_LIMITUSER_PACKET		MgrServerLimitUserPacket;
		MGR_SERVER_SHUTDOWN_PACKET		MgrServerShutDownPacket;

		// 공지사항
		MGR_PUBLIC_NOTICE_PACKET		MgrPublicNoticePacket;
	} b;
} MANAGER_PACKET, *LP_MANAGER_PACKET;
//---------------------------------------

// Map 서버 관련 패킷
//--------------------------------------------------------------------

// 공지사항
typedef struct bbs
{
	char msg[260];

} MANAGER_TO_MAP_BBS, *LP_MANAGER_TO_MAP_BBS;

//-------------------------------
typedef struct map_packet_header
{
	short int		type;
	short int		size;
	char			crc;

} MANAGER_TO_MAP_PACKET_HEADER, *LP_MANAGER_TO_MAP_PACKET_HEADER;

typedef struct map_packet
{
	union
	{
		char							data[sizeof(MANAGER_TO_MAP_PACKET_HEADER)];
		MANAGER_TO_MAP_PACKET_HEADER	header;
	} h;

	union
	{
		MANAGER_TO_MAP_BBS				PublicNotice;
	} u;

} MANAGER_TO_MAP_PACKET, *LP_MANAGER_TO_MAP_PACKET;
// --------------------------------

#pragma pack(pop)

#ifdef __IS_PROXY_SERVER
bool OnRecvMsgFromManager( USERINFO *pUserInfo, LP_MANAGER_PACKET pPacket, DWORD dwLength );
void AnswerAuthPacket( USERINFO* pUserInfo );		//20010508 ADD
bool OnRecvAuthMsgFromManager( USERINFO *pUserInfo, LP_MANAGER_PACKET pPacket, DWORD dwLength );
bool AnswerToManager( LP_MANAGER_PACKET pPacket, DWORD dwLength );
#else
bool OnRecvMsgFromManager( LP_MANAGER_PACKET pPacket, DWORD dwLength );
bool AnswerToManager( LP_MANAGER_PACKET pPacket, DWORD dwLength );
#endif
