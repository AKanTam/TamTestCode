#pragma once
#include "afxwin.h"


// CDialogStat �Ի���

class CDialogStat : public CDialog
{
	DECLARE_DYNAMIC(CDialogStat)

public:
	CDialogStat(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDialogStat();

// �Ի�������
	enum { IDD = IDD_DIALOG_STAT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CStatic StatDlgText;
	afx_msg void OnBnClickedOk();
	afx_msg void OnClose();
};