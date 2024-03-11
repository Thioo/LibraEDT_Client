// CEMApiEvents.h  : Declaration of ActiveX Control wrapper class(es) created by Microsoft Visual C++

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CEMApiEvents

class CEMApiEvents : public CWnd
{
protected:
	DECLARE_DYNCREATE(CEMApiEvents)
public:
	CLSID const& GetClsid()
	{
		static CLSID const clsid
			= { 0x71BD42C3, 0xEBD3, 0x11D0, { 0xAB, 0x3A, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0 } };
		return clsid;
	}
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,DWORD dwStyle,
						const RECT& rect, CWnd* pParentWnd, UINT nID,
						CCreateContext* pContext = nullptr)
	{ 
		return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID);
	}

	BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
				UINT nID, CFile* pPersist = nullptr, BOOL bStorage = FALSE,
				BSTR bstrLicKey = nullptr)
	{ 
		return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID,
		pPersist, bStorage, bstrLicKey);
	}

// Attributes
public:

// Operations
public:

	void Notify(LPCTSTR lpszParameter, long Reason)
	{
		static BYTE parms[] = VTS_BSTR VTS_I4;
		InvokeHelper(0x1, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, lpszParameter, Reason);
	}
	void NotifyWithCurrentValue(LPCTSTR lpszParameter, long Reason, long paramid, double dLastKnownValue)
	{
		static BYTE parms[] = VTS_BSTR VTS_I4 VTS_I4 VTS_R8;
		InvokeHelper(0x2, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, lpszParameter, Reason, paramid, dLastKnownValue);
	}


};
