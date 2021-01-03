// identifyIdCardDlg.cpp: 实现文件
//

#include "pch.h"
#include "headers.h"
#include "framework.h"
#include "identifyIdCard.h"
#include "identifyIdCardDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 输入和摄像头信息
Mat globalInputImage;
bool globalInputCheck;
bool cameraFlag;

// 嵌套的 OpenCV 窗宽高
CRect rc;
CWnd* pWnd;
int showingHigh;
int showingWidth;

// CidentifyIdCardDlg 对话框

CidentifyIdCardDlg::CidentifyIdCardDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_IDENTIFYIDCARD_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CidentifyIdCardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CidentifyIdCardDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CidentifyIdCardDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CidentifyIdCardDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON4, &CidentifyIdCardDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON3, &CidentifyIdCardDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CidentifyIdCardDlg 消息处理程序

BOOL CidentifyIdCardDlg::OnInitDialog(){
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	// 执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	bool globalInputCheck = false;
	bool cameraIsOpen = false;
	bool openCamera = false;

	// 获取嵌套的 OpenCV 窗宽高
	pWnd = GetDlgItem(IDC_STATIC);
	pWnd->GetClientRect(&rc);
	showingHigh = rc.Height();
	showingWidth = rc.Width();

	// 嵌套 OpenCV 窗口，显示身份证
	namedWindow("ImageShow");                         // 创建 OpenCV 窗口
	HWND hWnd = (HWND)cvGetWindowHandle("ImageShow"); // 嵌套 OpenCV 窗口
	HWND hParent = ::GetParent(hWnd);
	::SetParent(hWnd, GetDlgItem(IDC_STATIC)->m_hWnd);
	::ShowWindow(hParent, SW_HIDE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CidentifyIdCardDlg::OnPaint()
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
HCURSOR CidentifyIdCardDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 选择图片
void CidentifyIdCardDlg::OnBnClickedButton1() {
	// 选择被识别的图片
	CString defaultDir = L"";                                            // 默认打开的文件路径
	CString fileName = L"";                                              // 默认打开的文件名
	CString filter = L"文件 (*.png; *.jpg; *.jpeg)|*.png;*.jpg;*.jpeg||"; // 文件过虑的类型
	CFileDialog openFileDlg(TRUE, defaultDir, fileName, OFN_HIDEREADONLY | OFN_READONLY, filter, NULL);
	INT_PTR result = openFileDlg.DoModal();
	CString inputImagePath;
	// 获取文件路径，读取图片
	if (result == IDOK) {
		inputImagePath = openFileDlg.GetPathName();
		CWnd::SetDlgItemText(IDC_EDIT1, inputImagePath); // 输出图片路径
		string temp(CW2A(inputImagePath.GetString()));
		globalInputImage = imread(temp, ImreadModes::IMREAD_UNCHANGED);
		globalInputCheck = true;

		// 调整图片填充显示窗口
		Mat showingImage;
		resize(globalInputImage, showingImage, Size(showingWidth, showingHigh));
		imshow("ImageShow", showingImage);
		waitKey(1);
	}
}

// 识别
void CidentifyIdCardDlg::OnBnClickedButton2(){
	if (globalInputCheck) {
		// 识别身份证，获取信息
		string idResult = identifyIdCard(globalInputImage);

		// 获取出生地
		string birthland;
		char cities[6 + 1];
		cities[6] = '\0';
		for(int i = 0; i < 6;i++)
			cities[i] = idResult[i];
		char szbuff[128] = { 0 };
		ifstream citiesFile("cities.txt");
		while (!citiesFile.eof())
		{
			citiesFile.getline(szbuff, 128);
			if (strncmp(szbuff, cities, 6) == 0)
			{
				birthland = szbuff + 7;
				break;
			}
		}
		citiesFile.close();

		// 获取生日，计算年龄
		string birthdayYear = idResult.substr(6, 4);
		string birthdayMonth = idResult.substr(10, 2);
		string birthdayDay = idResult.substr(12, 2);
		time_t now = time(0);
		tm* localTm = localtime(&now);
		int age = localTm->tm_year + 1900 - atoi(birthdayYear.c_str());

		// 获取性别
		string sex;
		Sex se;
		if ((idResult[16] - '0') % 2 == 0)
			sex = se.female;
		else
			sex = se.male;

		// 检查校验和
		int sum = 0;
		int factors[17] = { 7, 9, 10, 5, 8, 4, 2, 1, 6, 3, 7, 9, 10, 5, 8, 4, 2 };
		string map = "10X98765432";
		for (int i = 0; i < 17; i++)
			sum += factors[i] * (idResult[i] - '0');
		string checksum;
		if (idResult[17] == map[sum % 11])
			checksum = "true";
		else
			checksum = "false";
		
		// 转换为 CSring 并输出
		CString idResultC, birthlandC, birthdayDayC, ageC, sexC, checksumC;
		idResultC = idResult.c_str();
		birthlandC = birthland.c_str();
		birthdayDayC = (birthdayYear + "." + birthdayMonth + "." +birthdayDay).c_str();
		ageC =to_string(age).c_str();
		sexC = sex.c_str();
		checksumC = checksum.c_str();
		CWnd::SetDlgItemText(IDC_EDIT2, idResultC);
		CWnd::SetDlgItemText(IDC_EDIT3, birthlandC);
		CWnd::SetDlgItemText(IDC_EDIT4, birthdayDayC);
		CWnd::SetDlgItemText(IDC_EDIT5, ageC);
		CWnd::SetDlgItemText(IDC_EDIT6, sexC);
		CWnd::SetDlgItemText(IDC_EDIT7, checksumC);

	}
}


// 打开摄像头
void CidentifyIdCardDlg::OnBnClickedButton3() {
	cameraFlag = true;
	VideoCapture capture(0);

	while (cameraFlag) {
		capture >> globalInputImage; // 读取当前帧
		Mat showingImage;
		resize(globalInputImage, showingImage, Size(showingWidth, showingHigh));
		imshow("ImageShow", showingImage);
		waitKey(1);
	}
}

// 拍照
void CidentifyIdCardDlg::OnBnClickedButton4() {
	if (cameraFlag) { 
		cameraFlag = false;
		CWnd::SetDlgItemText(IDC_EDIT1, L"Camera"); // 输出图片路径
		globalInputCheck = true;
	}
}