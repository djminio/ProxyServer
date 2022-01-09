//
//
//

#pragma once
#include "stdafx.h"
#include "dr_agent_defines.h"

//< CSD-030322
typedef struct server_connect_info
{
	char			startposition;
	int				port;
	char			ip[3][IP_LENGTH];
} t_server_connect_info;
//> CSD-030322
typedef struct client_access_login
{
	HSEL_INITIAL	init; // CSD-030322

	char			id[ID_LENGTH];
	char			pw[PW_LENGTH];
	short int		version;
	short int		mycode;

} t_client_access_login;

// Added by chan78 at 2000/12/07 :: 과금
typedef struct client_access_login_zoung
{
	short int		key;
	char			id[ID_LENGTH];
	char			pw[PW_LENGTH];
	short int		version;
	short int		mycode;
	char			ip[IP_LENGTH];
	WORD			wPort;		// Added by chan78 at 2000/12/17
} t_client_access_login_zoung;

typedef struct client_access_pay_per_min
{
	HSEL_INITIAL	init; // CSD-030322

	char			id[ID_LENGTH];
	char			pw[PW_LENGTH];
	short int		version;
	short int		mycode;
	//1115 zhh_edited
	short int		Corp_Code;	//업체 코드
	char			User_ID[ID_LENGTH];	//종량제 업체용 ID
	char			GateWayIP[20];
	int				GateWayPORT;
} t_client_access_pay_per_min;

typedef struct client_access_pay_per_min_zoung
{
	short int		key;
	char			id[ID_LENGTH];
	char			pw[PW_LENGTH];
	short int		version;
	short int		mycode;
	//1115 zhh_edited
	short int		Corp_Code;	//업체 코드
	char			User_ID[ID_LENGTH];	//종량제 업체용 ID
	char			GateWayIP[IP_LENGTH];
	int				GateWayPORT;
	char			ip[IP_LENGTH];		//Added by zoung
	WORD			wPort;		// Added by chan78 at 2000/12/17
} t_client_access_pay_per_min_zoung;

typedef struct agent_user_connection_lost
{
	char			id[ID_LENGTH];
} t_agent_user_connection_list;

// --------------------

typedef struct server_change_map
{
	short int		server_id;
	char			mapname[ NM_LENGTH ];
	short int		x;
	short int		y;
	int				port;
} t_server_change_map;

typedef struct server_accept_login					
{													
	short int		server_id;						
	char			name[ MAX_CHARACTEROFID][NM_LENGTH];
	unsigned char	level[ MAX_CHARACTEROFID];		
	unsigned char	job[ MAX_CHARACTEROFID];		
	unsigned char	cla[ MAX_CHARACTEROFID];		
	unsigned char	gender[ MAX_CHARACTEROFID];		
													
	unsigned char   bodyr[ MAX_CHARACTEROFID];		
	unsigned char   bodyg[ MAX_CHARACTEROFID];		
	unsigned char   bodyb[ MAX_CHARACTEROFID];		
													
	unsigned char   clothr[ MAX_CHARACTEROFID];		
	unsigned char   clothg[ MAX_CHARACTEROFID];		
	unsigned char   clothb[ MAX_CHARACTEROFID];		
													
	short int		age[ MAX_CHARACTEROFID];		
	unsigned int	money[ MAX_CHARACTEROFID];		
	unsigned char	acc_equip1[ MAX_CHARACTEROFID];	
	unsigned char	acc_equip2[ MAX_CHARACTEROFID];	
	unsigned char	acc_equip3[ MAX_CHARACTEROFID];	
	unsigned char	acc_equip4[ MAX_CHARACTEROFID];	
													
	char			nation;							
	short int		remained_day;					

	char			id[ ID_LENGTH];

} t_server_accept_login;

typedef struct client_access_join
{	
	char			id[ID_LENGTH];
	char			pw[PW_LENGTH];
	char			name[NM_LENGTH];
	char			startposition;		// 99이면 맨 마지막에 LogOut한자리에서 나타난다. 
	
} t_client_access_join;

// Added by chan78 at 2000/11/28
typedef struct server_port
{
	short int count;
	short int port[100];
}k_server_port;


#pragma pack (push, 1 )

typedef struct dragon_header
{					
	short int		type;
	short int		size;
	char			crc;

} t_header;			
					
typedef union kein_imsi
{
	k_server_port				server_port;
	char						default_char;
} t_kein;

typedef struct login_close_login_id 
{
	char id[ NM_LENGTH];
}t_login_close_login_id;

// 010109 KHS
typedef struct login_bbs		// 010110 YGI
{
	char	bbs[ MAX_PATH];
}t_login_bbs;

// 011012 KBS
struct t_send_map_change_date
{
	BYTE		server_header;	//PTCL
	t_header	cmd_header;		//Raja Packet의 t_header
	BYTE		bChanged;		//날(Day)가 변경되었으면 1, 달(Month)과 날짜가 같이 변경되었으면 2
	WORD		wDay;			//Day
	WORD		wMonth;			//Month

	DWORD	GetPacketSize() { return (DWORD)11/* 1+5+1+2+2 */;	}
	t_send_map_change_date(BYTE changed, WORD day, WORD month)
	{
		server_header = PTCL_PROXY_TO_MAP;
		
		cmd_header.type = CMD_PROXY_TO_MAP_CHANGE_DATE;
		cmd_header.size = 5;
		cmd_header.crc = NULL;
		
		bChanged = changed;
		wDay = day;
		wMonth = month;
	}	
};

struct t_rm_change_weather
{
	BYTE	server_header;
	t_header cmd_header;
	BYTE	bWeather;		//0이면 정상, 1이면 비, 2이면 눈 
	DWORD	dwAmount;

	DWORD GetPacketSize() { return (DWORD)11;	}
	t_rm_change_weather(BYTE weather, DWORD amount)
	{
		server_header = PTCL_PROXY_TO_MAP;

		cmd_header.type = CMD_PROXY_TO_MAP_CHANGE_DATE;
		cmd_header.size = 5;
		cmd_header.crc = NULL;
		
		bWeather = weather;
		dwAmount = amount;
	}

};

//--------------------------------------------------------
typedef struct packet
{					
	union			
	{				
		char					data[sizeof(t_header)];
		t_header				header;
	} h;			
						
	union			
	{				
		char								data[RAJA_MAX_PACKET_SIZE];
		t_client_access_login				client_access_login;
		t_client_access_login_zoung			client_access_login_zoung;
		t_server_accept_login				server_accept_login;
		t_server_connect_info				server_connect_info;
		t_client_access_join				client_access_join;
		t_server_change_map					server_change_map;
		t_client_access_pay_per_min			client_access_pay_per_min;
		t_client_access_pay_per_min_zoung	client_access_pay_per_min_zoung;
		t_login_close_login_id				login_close_login_id;

		// 010109 KHS
		char								default_msg[200];		// 010110 YGI
		t_login_bbs							login_bbs;

		t_kein					kein;
	} u;

	struct packet *next;

} t_packet;


#pragma pack (pop)

//--------------------------------------------------------


//--------------------------------------------------------
/*
typedef struct connection 
{
	SOCKET			socket;
	struct			sockaddr_in addr;
	int				state;
	t_packet		*inbuf;
	t_packet		*outbuf;
	int				receive;
	int				receive_count;
    DWORD			connect_time;
	DWORD			monitor_time;
	DWORD			send_bytes;
	DWORD			receive_bytes;
	int				send_try;		// 보내기 횟수...

	int				send_addlen;

	char			ip_address[128];

	int				last_year;
	int				last_mon;
	int				last_day;
	int				last_hour;
	int				last_min;

	int				login_year;
	int				login_mon;
	int				login_day;
	int				login_hour;
	int				login_min;
	int				login_sec;

	t_packet		packet;
	int				packet_pnt;
	int				packet_size;
	int				packet_status;
	int				packet_count[2];

	int				kick_out;
	DWORD			kick_out_time;

	int				save_db;
	DWORD			save_db_time;

	char			id[ID_LENGTH];
	char			pw[PW_LENGTH];
	char			name[NM_LENGTH];
	char			mapname[NM_LENGTH];

	int				server_check;
	char			*SendBuf;				// send()로 보낼 내용을 다음에 담아 놓는다. 
	int				SendBufSize;			// send()로 보낼 내용의 크기. 
	int				SendErr;				// send()로 보낼 내용의 크기. 

} t_connection;
*/

typedef struct im_game_server
{		
	int port;
}t_im_game_server;

typedef struct char_info_magic{	
	unsigned char magic[ 200 ];
}t_char_info_magic;

typedef struct char_info_skill{	
	unsigned char skill[ 200 ];
}t_char_info_skill;

typedef struct char_info_tac_skillexp
{
	unsigned int tac_skillEXP[ 13];

}t_char_info_tac_skillexp;

typedef struct client_isthere_charname
{		
	char name[ NM_LENGTH];
}t_client_isthere_charname;

typedef struct how_many_in_map
{
	char		map[ NM_LENGTH ];
	short int	how;
}t_how_many_in_map;

typedef struct update_very_important_status
{
	char            name[ NM_LENGTH];
	
	short int		Level;

	short int		Str  ;	
	short int		Con  ;	
	short int		Dex  ;	
	short int		Wis  ;	
	short int		Int  ;	
	short int		MoveP;	  
	short int		Char ;	 
	short int		Endu ;	 
	short int		Moral;	  
	short int		Luck ;	 
	short int		wsps ;	 

	int				HpMax ;
	int				ManaMax;
	int				HungryMax;

	short int		reserved_point;

	DWORD			Exp;

}t_update_very_important_status;

typedef struct update_very_important_tactics
{
	char name[NM_LENGTH];
	char tac_skillEXP[SIZE_OF_TAC_SKILL_EXP];
}t_update_very_important_tactics;

typedef struct item_duration_change
{
	BYTE pos;
	WORD dur;
}t_item_duration_change;

typedef struct tactics_parrying_exp
{
	DWORD exp;
}t_tactics_parrying_exp;

typedef struct t_POS
{
	char type ; 
	short int p1, p2, p3 ;
} POS ;

typedef struct itemAttr
{
		short int		item_no;
		unsigned int	attr[6];
} ItemAttr;

typedef struct tag_K_ITEM
{
	ItemAttr	item_attr;
	POS			item_pos;
}K_ITEM, *lpITEM;

typedef struct tag_CharRank
{
//public :
	DWORD	nation		: 4 ;		// 속한나라	0 : 없음, 1, 2, 3: 바이서스 4: 자이펀 5:헤게모니아 6: 일스
	DWORD	counselor	: 2 ;		// 0 : 없음, 1: 회사 운영자, 2: 게임내 운영자
	DWORD	king		: 1 ;		// 나라 왕
	DWORD	guild_code	: 9 ;		// 길드 코드 ( 512개 )
	DWORD	guild_master: 3 ;		// 길드 마스터

//public :
//	CCharRank() { memset( this, 0, sizeof( CCharRank) ); }
}CCharRank;
////////////////////////////////////////////////////////////////////////////

typedef enum enumDIRECTION
{

	DIRECTION_SAME  			=   0,
	DIRECTION_UP				=   4,
	DIRECTION_RIGHTUP			=   5,
	DIRECTION_RIGHT				=   6,
	DIRECTION_RIGHTDOWN			=   7,
	DIRECTION_DOWN				=   0,
	DIRECTION_LEFTDOWN			=   1,
	DIRECTION_LEFT				=   2,
	DIRECTION_LEFTUP			=   3

} DIRECTION;

typedef struct tagCharacterParty
{
	short int	On;
	int			Server_id;
	char		Name[ 31];
	int			Face;
	int			Level;
	int			Gender;
	int			Str;
	int			Class;
}CharacterParty, *LpCharacterParty;

