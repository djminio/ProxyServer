#pragma once

#define MAX_CHARACTEROFID						4

// 8bytes ordered
//#define SIZE_OF_T_HEADER						8
// 1byte ordered
#define SIZE_OF_T_HEADER						5//010816 lsw

#define RAJA_MAX_PACKET_SIZE					3000

#define ID_LENGTH								20
#define PW_LENGTH								20
#define NM_LENGTH								20
#define	IP_LENGTH								16

#define TEXT_LENGTH								255
#define SIZE_OF_TAC_SKILL_EXP					(13  * sizeof( unsigned int	 ))	

// ....
#define CONNECT_OK								1

// Agent ���� ���̴� ��Ŷ�� ������.
#define CMD_NONE								0
#define CMD_PING								5
#define CMD_ACCESS_LOGIN						6066 // CSD-030322
//   Clinet->MapServer
#define CMD_ACCESS_JOIN							30

#define CMD_REQ_INSERT_USERID					34
#define CMD_REQ_DELETE_USERID					35
#define CMD_CREATE_CHAR							50
#define CMD_DELETE_CHAR							52
#define CMD_CHAT_DATA							82
#define CMD_CHANGE_MAP_DETECTED			109	
#define CMD_CHANGE_MAP							110
#define CMD_LOST_CONNECTION						127
#define CMD_CONNECT_INFO						200
#define CMD_LEVELUP_POINT						410

// Added by chan78 at 2000/12/07 :: ����
#define CMD_ACCESS_PAY_PER_MIN					710

#define	CMD_CHR_ITEM_INFO_0						5000	// Inventory
#define	CMD_CHR_ITEM_INFO_1						5001	// Equip
#define	CMD_CHR_ITEM_INFO_2						5002	// quick.
#define CMD_CHAR_INFO_MAGIC						5004
#define CMD_CHAR_INFO_SKILL						5005
#define CMD_CHAR_INFO_TAC_SKILL_EXP				5008
#define CMD_ISTHERE_CHARNAME					7540

#define CMD_HOW_MANY_IN_MAP						8413
#define CMD_UPDATE_VERY_IMPORTANT_STATUS		8465
// Login Server -> Game Server
// id�� ���� ���� �����...
#define CMD_CLOSE_LOGIN_ID						8462

#define CMD_UPDATE_VERY_IMPORTANT_TACTICS		8466
#define CMD_ITEM_DURATION_CHANGE				8426
#define CMD_TACTICS_PARRYING_EXP				8470
#define CMD_UPDATE_CHAR_DB						8500
#define CMD_UPDATE_BINARY_DATA0					8501
#define CMD_UPDATE_BINARY_DATA1					8502
#define CMD_UPDATE_SCRIPT_DATA					8503
#define CMD_UPDATE_INV_DATA						8504	
#define CMD_UPDATE_ITEM_DATA					8505
#define CMD_UPDATE_BANKITEM_DATA				8506	
#define CMD_IM_GAME_SERVER         				9011
#define CMD_GLOBAL_CHAT_DATA					9001
#define CMD_ACCESS_CHAR_DB						9021
#define CMD_CONNECT_INFO1						10000		// 1218 YGI
#define CMD_DELETE_ITEM							10011		// ������ �μ���
#define CMD_REQ_PARTY_MEMBER					10036
#define	CMD_CHECK_BETA_TEST						10068
#define	CMD_PARTY_ACCESS						10082
#define CMD_REQ_PARTY_TOGETHER					10083		// ���� ������ �ΰ� �ִ��� �˾ƺ��� ���� �α��� ������ ��� ��û�Ѵ�.
#define CMD_THROW_DICE							10088		// �ֻ��� ������...
#define CMD_CREATE_ABILITY						10089		// �⺻��ġ ���� ����
#define CMD_CHECK_NEW_CHAR						10120

// 001205 KHS 
#define CMD_RESET_JOB							10213	// JOB�� ���� ����
#define CMD_SELECT_NATION						10215	// ���� ���� 

#define MAX_LEARN_ITEM						1000			//1220
#define MAX_DIR_TABLE						 256
#define	MAX_SHORTPATH						  50
#define MAX_BIX_ITEM						  15
#define	MAX_PC_CONTROL_NPC					   8

// Added by chan78 at 2000/11/28
#define CMD_SV_CONNECT_SERVER_COUNT			12001	// ���� ī��Ʈ ��������
#define CMD_SV_GET_CONNECT_SERVER_PORT		12002	// ����� ���Ӽ��� ��������

#define CMD_ACCEPT_LOGIN				11

// Added by chan78 at 2001/01/09
#define CMD_CLOSE_CONNECTION_NORMAL					13001
#define CMD_CLOSE_CONNECTION_ABNORMAL				13002
#define CMD_CLOSE_CONNECTION_SAME_ID_LOGON			13003
#define CMD_CLOSE_CONNECTION_SLEPT_TOO_LONG_TIME	13004

// 010109 KHS
#define	CMD_SV_SEND_MESSAGE_ALL				12003	// ���� ������ �޽����� �ѷ��ش�.		// 010110 YGI
#define	CMD_LOGIN_BBS						8450	// 010110 YGI

#define CMD_ALL_READY					33

// 011013 KBS
#define CMD_PROXY_TO_MAP_CHANGE_DATE		14000	//
// 011212 KBS
#define CMD_PROXY_TO_MAP_CHANGE_WEATHER		14001	//
//


