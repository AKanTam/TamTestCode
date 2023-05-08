#pragma once
#include "afxwin.h"


// CDialogStat 对话框

class CDialogStat : public CDialog
{
	DECLARE_DYNAMIC(CDialogStat)

public:
	CDialogStat(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDialogStat();

// 对话框数据
	enum { IDD = IDD_DIALOG_STAT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CStatic StatDlgText;
	afx_msg void OnBnClickedOk();
	afx_msg void OnClose();
};