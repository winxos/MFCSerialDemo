
// MFCSerialDemoDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MFCSerialDemo.h"
#include "MFCSerialDemoDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCSerialDemoDlg 对话框



CMFCSerialDemoDlg::CMFCSerialDemoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCSERIALDEMO_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCSerialDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_port);
	DDX_Control(pDX, IDC_COMBO2, m_baud);
}

BEGIN_MESSAGE_MAP(CMFCSerialDemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCSerialDemoDlg::OnBnClickedButton1)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON2, &CMFCSerialDemoDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CMFCSerialDemoDlg 消息处理程序
OVERLAPPED m_osWrite;
OVERLAPPED m_osRead;
HANDLE hCom;  //全局变量，串口句柄
BOOL CMFCSerialDemoDlg::find_port()
{
	long lReg;
	HKEY hKey;
	DWORD MaxValueLength;
	DWORD dwValueNumber;
	lReg = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"),
		0, KEY_QUERY_VALUE, &hKey);
	if (lReg != ERROR_SUCCESS) //成功时返回ERROR_SUCCESS，
	{
		AfxMessageBox(TEXT("Open Registry Error!\n"));
		return FALSE;
	}

	lReg = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL,
		&dwValueNumber, &MaxValueLength, NULL, NULL, NULL);

	if (lReg != ERROR_SUCCESS) //没有成功
	{
		AfxMessageBox(TEXT("Getting Info Error!\n"));
		return FALSE;
	}

	TCHAR* pValueName, * pCOMNumber;
	DWORD cchValueName, dwValueSize = 10;

	for (int i = 0; i < dwValueNumber; i++)
	{
		cchValueName = MaxValueLength + 1;
		dwValueSize = 10;
		pValueName = (TCHAR*)VirtualAlloc(NULL, cchValueName, MEM_COMMIT, PAGE_READWRITE);
		lReg = RegEnumValue(hKey, i, pValueName,
			&cchValueName, NULL, NULL, NULL, NULL);

		if ((lReg != ERROR_SUCCESS) && (lReg != ERROR_NO_MORE_ITEMS))
		{
			AfxMessageBox(TEXT("Enum Registry Error or No More Items!\n"));
			return FALSE;
		}

		pCOMNumber = (TCHAR*)VirtualAlloc(NULL, 6, MEM_COMMIT, PAGE_READWRITE);
		lReg = RegQueryValueEx(hKey, pValueName, NULL,
			NULL, (LPBYTE)pCOMNumber, &dwValueSize);

		if (lReg != ERROR_SUCCESS)
		{
			AfxMessageBox(TEXT("Can not get the name of the port"));
			return FALSE;
		}

		CString str(pCOMNumber);
		m_port.AddString(str); //把获取的值加入到ComBox控件中

		VirtualFree(pValueName, 0, MEM_RELEASE);
		VirtualFree(pCOMNumber, 0, MEM_RELEASE);
	}
	m_port.SetCurSel(0);
	return true;
}
BOOL CMFCSerialDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	// TODO: 在此添加额外的初始化代码
	int baud[] = { 9600,115200 };
	for (int i = 0; i < 2; i++)
	{
		CString b;
		b.Format(_T("%d"), baud[i]);
		m_baud.AddString(b);
	}
	m_baud.SetCurSel(0);
	find_port();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCSerialDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCSerialDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCSerialDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMFCSerialDemoDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	DWORD dwBytesWrite = 0;
	COMSTAT ComStat;
	DWORD dwErrorFlags;
	BOOL bWriteStat;
	ClearCommError(hCom, &dwErrorFlags, &ComStat);
	memset(&m_osWrite, 0, sizeof(OVERLAPPED));
	m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	CString input;
	GetDlgItem(IDC_EDIT1)->GetWindowText(input);
	bWriteStat = WriteFile(hCom, input, input.GetLength(), &dwBytesWrite, &m_osWrite);
	if (!bWriteStat)
	{
		if (GetLastError() == ERROR_IO_PENDING)
		{
			WaitForSingleObject(m_osWrite.hEvent, 1000);
		}
		return;
	}
	PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
}


void CMFCSerialDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent)
	{
	case 1:
		read_serial();
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}
int CMFCSerialDemoDlg::read_serial()
{
	// TODO: 在此处添加实现代码.
	COMSTAT ComStat;
	DWORD dwErrorFlags;

	char str[100];
	memset(str, 0, 100);
	DWORD dwBytesRead = 0;//读取的字节数
	BOOL bReadStat;

	ClearCommError(hCom, &dwErrorFlags, &ComStat);
	dwBytesRead = (DWORD)ComStat.cbInQue;
	if (dwBytesRead <= 0)
	{
		return 0;
	}
	bReadStat = ReadFile(hCom, str, dwBytesRead, &dwBytesRead, &m_osRead);
	if (!bReadStat)
	{
		if (GetLastError() == ERROR_IO_PENDING)
		{
			WaitForSingleObject(m_osRead.hEvent, 2000);
		}
	}
	CString r;
	GetDlgItem(IDC_EDIT2)->GetWindowText(r);
	CString s = r + "\r\n" + str;
	GetDlgItem(IDC_EDIT2)->SetWindowText(s);
	UpdateData(FALSE);
	PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	return 0;
}


void CMFCSerialDemoDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	CString port;
	m_port.GetWindowTextA(port);
	HANDLE tmp = CreateFile(port, //change to your port
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, //
		NULL);
	if (tmp == INVALID_HANDLE_VALUE) //already open
	{
		GetDlgItem(IDC_BUTTON2)->SetWindowText("打开");
		CloseHandle(hCom);
		return;
	}
	hCom = tmp;
	SetupComm(hCom, 100, 100);

	COMMTIMEOUTS TimeOuts;
	//设定读超时
	TimeOuts.ReadIntervalTimeout = 1000;
	TimeOuts.ReadTotalTimeoutMultiplier = 500;
	TimeOuts.ReadTotalTimeoutConstant = 5000;
	//设定写超时
	TimeOuts.WriteTotalTimeoutMultiplier = 500;
	TimeOuts.WriteTotalTimeoutConstant = 2000;
	SetCommTimeouts(hCom, &TimeOuts); //设置超时

	DCB dcb;
	GetCommState(hCom, &dcb);
	CString baud;
	m_baud.GetWindowTextA(baud);
	dcb.BaudRate = _ttoi(baud);
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	SetCommState(hCom, &dcb);

	PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR);
	SetTimer(1, 10, NULL); //10 ms check
	memset(&m_osRead, 0, sizeof(OVERLAPPED));
	m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	GetDlgItem(IDC_BUTTON2)->SetWindowText(_T("关闭"));
}
