// DialogStat.cpp : 实现文件
//

#include "stdafx.h"
#include "ACEInfoBridge.h"
#include "DialogStat.h"
#include "afxdialogex.h"


// CDialogStat 对话框

IMPLEMENT_DYNAMIC(CDialogStat, CDialog)

CDialogStat::CDialogStat(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogStat::IDD, pParent)
{

}

CDialogStat::~CDialogStat()
{
}

void CDialogStat::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STAT_STATIC, StatDlgText);
}


BEGIN_MESSAGE_MAP(CDialogStat, CDialog)
	ON_BN_CLICKED(IDOK, &CDialogStat::OnBnClickedOk)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CDialogStat 消息处理程序


void CDialogStat::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	DestroyWindow();
	//CDialog::OnOK();
}



void CDialogStat::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	DestroyWindow();
	//CDialog::OnClose();
}
