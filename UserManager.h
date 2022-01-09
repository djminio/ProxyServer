// UserManager.h: interface for the CUserManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USERMANAGER_H__4969A899_8FEA_4F7A_BC59_8A2E3CCCCA16__INCLUDED_)
#define AFX_USERMANAGER_H__4969A899_8FEA_4F7A_BC59_8A2E3CCCCA16__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

///////////////////////////////////////////////////////////////////////////////
// 立加磊 包府 努贰胶
class CUserManager  
{
	enum {DELAY = 10000};
	typedef map<string, DWORD>     HASH_USER;
	typedef HASH_USER::iterator   ITOR_USER;
	typedef HASH_USER::value_type PAIR_USER;

public:
	CUserManager();
	virtual ~CUserManager();

public:
	void AddLogin(const char* pName);
	void DelLogin(const char* pName);

	void AddLogout(const char* pName);
	void DelLogout();

public:
	int GetLoginSize() const 
	{ 
		return m_mpLogin.size(); 
	}

	int GetLogoutSize() const 
	{ 
		return m_mpLogout.size(); 
	}

	bool IsExistLogin(const char* pName) const
	{
		return (m_mpLogin.find(pName) != m_mpLogin.end()) ? true:false;
	}

	bool IsExistLogout(const char* pName) const
	{
		return (m_mpLogout.find(pName) != m_mpLogout.end()) ? true:false;
	}

private:
	HASH_USER m_mpLogin;
	HASH_USER m_mpLogout;
};
//
///////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_USERMANAGER_H__4969A899_8FEA_4F7A_BC59_8A2E3CCCCA16__INCLUDED_)
