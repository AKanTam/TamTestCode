// DialogStat.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ACEInfoBridge.h"
#include "DialogStat.h"
#include "afxdialogex.h"


// CDialogStat �Ի���

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


// CDialogStat ��Ϣ�������


void CDialogStat::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	DestroyWindow();
	//CDialog::OnOK();
}



void CDialogStat::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	DestroyWindow();
	//CDialog::OnClose();
}
