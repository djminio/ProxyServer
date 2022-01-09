// LocalizingMgr.h: interface for the CLocalizingMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOCALIZINGMGR_H__F4B2DCF8_D77A_426D_8C68_DE978C3113E8__INCLUDED_)
#define AFX_LOCALIZINGMGR_H__F4B2DCF8_D77A_426D_8C68_DE978C3113E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define PROXY_SERVER_INI_		".\\ProxyServer.ini"
#define SERVER_MANAGER_INI_		PROXY_SERVER_INI_
#define DB_DEMON_INI_			".\\DBDemon.ini"
#define AGENT_SERVER_INI_		".\\agentserver.ini"
#define MAP_SERVER_INI_			".\\MapServer.ini"

extern const char szKorea	[];
extern const char szChina	[];
extern const char szTaiwan	[];
extern const char szThai	[];
extern const char szHongKong[];
extern const char szUsa		[];
extern const char szJapan	[];

enum MY_CODE
{
	KOREA_MYCODE	= 1315,
	TAIWAN_MYCODE	= 3315,
	HONGKONG_MYCODE	= 4315,
	CHINA_MYCODE	= 5315,
	THAI_MYCODE		= 6315,	
	JAPAN_MYCODE	= 8315
};

enum NationCode
{
	NOTSET	= 0x00000000,//NOTSET�� ���� ������ ����ó���մϴ�.
	KOREA	= 0x00000001,
	CHINA	= 0x00000002,
	TAIWAN	= 0x00000004,
	THAI	= 0x00000008,
	HONGKONG= 0x00000010,
	USA		= 0x00000020,	
	JAPAN	= 0x00000040
};

enum DBType//DBŸ��
{
	TOTAL_DB	= 1,
	DRAGON_DB	= 2,	
	CHRLOG_DB	
};

const int  ID	=	1;
const int  PASS	=	0;

class CLocalizingMgr  
{
private:
	int ConvertNameToCode(const char* szNationName);
	int m_iNationCode;//�����ڵ� �����
	char *m_pszNationName;
	
	char *m_pszTotalDbID;
	char *m_pszTotalDbPW;
	char *m_pszDragonDbID;
	char *m_pszDragonDbPW;
	char *m_pszChrlogDbID;
	char *m_pszChrlogDbPW;

	int m_iMyCode;//�����ڵ� �����
	int m_iIsFreeBeta;//�����ڵ� �����
private:
	CLocalizingMgr operator=(const CLocalizingMgr &old);//���Կ����� ��� �Ұ�.
	CLocalizingMgr(const CLocalizingMgr &old);//��������� ��� �Ұ�.

private:
	void SetNationName(const char* szNationName);
	int SetDBAccount(const int iType, const char* szId,const char* szPw);
public:
	CLocalizingMgr();
	virtual ~CLocalizingMgr();
public://SetNationCode ���� �Լ��� ������ �ʽ��ϴ�. ���� ���� �߿� ������ ���� �����Դϴ�.
	const int GetNationCode()const{return m_iNationCode;}//���� �ڵ带 �޾ƿɴϴ�.
	const char* GetNationName()const{return m_pszNationName;}//���� �̸��� �޾� �ɴϴ�.
	const int IsFreeBeta()const{return (m_iIsFreeBeta)?1:0;}//�̰��� ������Ÿ���� �˾Ƴ��ϴ�.

	int InitVersion(const int iNationCode,const int iIsFreeBeta = false);//�� �ѹ��� ȣ�� �ϵ��� �Ͻʽÿ�.
	int InitVersion(const char* szNationName,const int iIsFreeBeta = false);//�� �ѹ��� ȣ�� �ϵ��� �Ͻʽÿ�.
	
	void DisplayLocalizingSet()const;//���� ���ö���¡ ������ �����ݴϴ�.

	int IsChangeMoney(){return (IsAbleNation(TAIWAN|CHINA|HONGKONG))?0:1; }//�븸 �汹 ȫ���� ������ ����
	
	int IsAbleMyCode(const int iMyCode)const;//�Ұ����� �����ڵ��� 0�� ���� �����ϸ� 1�� ����
	int IsAbleNation(const int iNationCode)const;//�Ұ����� ������� 0�� ���� �����ϸ� 1�� ����
/*	
	void example()//IsAbleNation()�� ���� �ڵ�
	{
		if(IsAbleNation(KOREA||TAIWAN)
		{//�ѱ��̰ų� �븸�̸� ���� �˴ϴ�.
		}
		else
		{//������ �������� ���� �˴ϴ�.
		}
	}
*/
	const char *GetDBAccount(const int iType, bool bIsID);//iType TOTAL_DB,���� Ÿ���Դϴ�.bIsID ID, PASS ���߿� �ϳ��� �� �� �ֽ��ϴ�. ����� const char*�� �Ѿ�ɴϴ�. ��Ʈ��ī�ǳ� ��ī�Ǹ� ����� �ֽʽÿ�.
/*	
	void example()//GetDBAccount()�� ���� �ڵ�
	{
		char id[30],pw[30];

		if( (Init_SQL("DragonRajaDB", GetDBAccount(TOTAL_DB,ID), GetDBAccount(TOTAL_DB,PASS)) ) == 0)
		{//
		}
	}
*/
};
extern CLocalizingMgr LocalMgr;
#endif // !defined(AFX_LOCALIZINGMGR_H__F4B2DCF8_D77A_426D_8C68_DE978C3113E8__INCLUDED_)
