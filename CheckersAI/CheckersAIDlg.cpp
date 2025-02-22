
// CheckersAIDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CheckersAI.h"
#include "CheckersAIDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define		PLAYER_HUMAN	FALSE
#define		PLAYER_AI		TRUE

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
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


// CCheckersAIDlg dialog



CCheckersAIDlg::CCheckersAIDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_CHECKERSAI_DIALOG, pParent)
	, m_RedPlayer(0)
	, m_WhitePlayer(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCheckersAIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GRID_CHECKERBOARD, m_grdCheckersBoard);
	DDX_Radio(pDX, IDC_RADIO_RED_HUMAN, m_RedPlayer);
	DDX_Radio(pDX, IDC_RADIO_WHITE_HUMAN, m_WhitePlayer);
	DDX_Control(pDX, IDC_SLIDER_DIF_RED, m_SliderDifRed);
	DDX_Control(pDX, IDC_SLIDER_DIF_WHITE, m_SliderDifWhite);
}

BEGIN_MESSAGE_MAP(CCheckersAIDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_RADIO_WHITE_AI, &CCheckersAIDlg::OnBnClickedRadioWhiteAi)
	ON_BN_CLICKED(IDC_RADIO_RED_HUMAN, &CCheckersAIDlg::OnBnClickedRadioRedHuman)
	ON_BN_CLICKED(IDC_RADIO_RED_AI, &CCheckersAIDlg::OnBnClickedRadioRedAi)
	ON_BN_CLICKED(IDC_RADIO_WHITE_HUMAN, &CCheckersAIDlg::OnBnClickedRadioWhiteHuman)
	ON_BN_CLICKED(IDC_BTN_RESTART, &CCheckersAIDlg::OnBnClickedBtnRestart)
END_MESSAGE_MAP()


// CCheckersAIDlg message handlers

BOOL CCheckersAIDlg::OnInitDialog()
{

	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	// 난이도 설정은 여기서
	m_SliderDifRed.SetRange(5,11);
	m_SliderDifRed.SetPos(8);
	m_SliderDifRed.EnableWindow(FALSE);
	m_SliderDifWhite.SetRange(5, 11);
	m_SliderDifWhite.SetPos(8);
	((CButton*)GetDlgItem(IDC_RADIO_RED_HUMAN))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_RADIO_RED_AI))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_RADIO_WHITE_HUMAN))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_RADIO_WHITE_AI))->SetCheck(1);

	m_RedPlayer = 0; m_bRedPlayerAI = FALSE;
	m_WhitePlayer = 1; m_bWhitePlayerAI = TRUE;
	m_pCheckerGame = new CCheckerGame(m_bRedPlayerAI, m_bWhitePlayerAI);
	m_pCheckerGame->SetEventHandler(this);
	InitImageList();
	InitCheckersBoard();
	ResetGame();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCheckersAIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCheckersAIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCheckersAIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

VOID CCheckersAIDlg::InitImageList()
{
	m_pImageList = new CImageList;
	m_pImageList->Create(67, 67, ILC_COLOR24, 1, 1);
	CBitmap* Bitmap = NULL;

	// 0 - Red
	Bitmap = new CBitmap;
	Bitmap->LoadBitmapW(IDB_BITMAP_RED);
	m_pImageList->Add(Bitmap, RGB(255, 255, 255));

	// 1 - Red king
	Bitmap = new CBitmap;
	Bitmap->LoadBitmapW(IDB_BITMAP_RED_K);
	m_pImageList->Add(Bitmap, RGB(255, 255, 255));

	// 2 - White
	Bitmap = new CBitmap;
	Bitmap->LoadBitmapW(IDB_BITMAP_WHITE);
	m_pImageList->Add(Bitmap, RGB(255, 255, 255));

	// 3 - White king
	Bitmap = new CBitmap;
	Bitmap->LoadBitmapW(IDB_BITMAP_WHITE_K);
	m_pImageList->Add(Bitmap, RGB(255, 255, 255));
}

VOID CCheckersAIDlg::InitCheckersBoard()
{
	m_grdCheckersBoard.SetEditable(FALSE);
	m_grdCheckersBoard.EnableSelection(FALSE);
	m_grdCheckersBoard.EnableScrollBar(SB_BOTH, FALSE);
	m_grdCheckersBoard.SetEventHandler(this);
	m_grdCheckersBoard.SetImageList(m_pImageList);

	// 행/열 갯수 설정
	m_grdCheckersBoard.SetRowCount(8);
	m_grdCheckersBoard.SetColumnCount(8);

	// 열의 넓이, 행 높이 동시 설정
	INT nCnt = 8;		// 전체크기 / 원소크기 = 원소개수
	INT nWidthHeight[8];

	for(INT i = 0; i < nCnt; i++)
		nWidthHeight[i] = 75;
	
	for(INT c = 0; c < nCnt; c++)
	{
		for(INT r = 0; r < nCnt; r++)
		{
			m_grdCheckersBoard.SetColumnWidth(c, nWidthHeight[c]);
			m_grdCheckersBoard.SetRowHeight(r, nWidthHeight[c]);
			m_grdCheckersBoard.SetItemFormat(r, c, DT_CENTER);
		}
	}

	ReColorCheckerBoard();
	RefreshCheckerBoard();

	m_grdCheckersBoard.Invalidate();
}

VOID CCheckersAIDlg::OnGridClick(INT a_nRow, INT a_nCol)
{
	static BOOL bSelected = FALSE;
	static INT nBeforeRow = -1;
	static INT nBeforeCol = -1;
	INT nCurrentTurn = m_pCheckerGame->GetPlayerTurn();
	INT nResult = 0;
	BOOL bMoveResult = FALSE;

	if(nBeforeRow >= 0 && nBeforeCol >= 0)
		m_grdCheckersBoard.SetItemFgColour(nBeforeRow, nBeforeCol, RGB(0, 0, 0));
	ReColorCheckerBoard();

	// 현재 턴이 AI턴이라면 리턴
	if(m_bRedPlayerAI && nCurrentTurn == CHECKER_TEAM_RED)
		return;
	if(m_bWhitePlayerAI && nCurrentTurn == CHECKER_TEAM_WHITE)
		return;

	if(!bSelected)
	{
		// 빈칸이라면 그냥 리턴
		if(!m_pCheckerGame->IsPieceExist(a_nRow, a_nCol))
			return;

		// 자기 것이 아닌 말이라면 그냥 리턴
		if(m_pCheckerGame->GetPieceTeam(a_nRow, a_nCol) != nCurrentTurn)
			return;

		bSelected = TRUE;
		nBeforeRow = a_nRow;
		nBeforeCol = a_nCol;
		m_grdCheckersBoard.SetItemFgColour(nBeforeRow, nBeforeCol, RGB(255, 64, 64));

		// 이동할 수 있는 곳 배경색 변경
		ColorAvalialbePath(nBeforeRow, nBeforeCol);
	}
	else
	{
		// 선택된 상태: 말이 있는 칸 선택했다면 선택 갱신
		if(m_pCheckerGame->IsPieceExist(a_nRow, a_nCol))
		{
			// 자기 것이 아닌 말이라면 선택해제 후 리턴
			if(m_pCheckerGame->GetPieceTeam(a_nRow, a_nCol) != nCurrentTurn)
			{
				bSelected = FALSE;
				return;
			}
			nBeforeRow = a_nRow;
			nBeforeCol = a_nCol;
			m_grdCheckersBoard.SetItemFgColour(nBeforeRow, nBeforeCol, RGB(255, 64, 64));

			// 이동할 수 있는 곳 배경색 변경
			ColorAvalialbePath(nBeforeRow, nBeforeCol);
			return;
		}

		// 이동 실시
		bMoveResult = m_pCheckerGame->MovePiece({ nBeforeRow , nBeforeCol }, { a_nRow, a_nCol });
		if(bMoveResult)
		{
			nBeforeRow = a_nRow;
			nBeforeCol = a_nCol;
		}

		// 선택 해제
		bSelected = FALSE;
		m_grdCheckersBoard.SetItemFgColour(nBeforeRow, nBeforeCol, RGB(0, 0, 0));	
	}
}

void CCheckersAIDlg::OnPlayerRedTurn()
{
	if(!m_bRedPlayerAI)
		return;

	if(!m_pCheckerGame->IsCurrentPlayerAI())
		return;

	SetDlgItemText(IDC_STATIC_AI_THINK, s_szSTRING_AI_THINKING);
	if(!m_pCheckerGame->PlayAITurn())
	{
		AfxMessageBox(L"Internal error! Resetting game..");
		ResetGame();
	}
}

void CCheckersAIDlg::OnPlayerWhiteTurn()
{
	if(!m_bWhitePlayerAI)
		return;

	if(!m_pCheckerGame->IsCurrentPlayerAI())
		return;

	SetDlgItemText(IDC_STATIC_AI_THINK, s_szSTRING_AI_THINKING);
	if(!m_pCheckerGame->PlayAITurn())
	{
		AfxMessageBox(L"Internal error! Resetting game..");
		ResetGame();
	}
}

void CCheckersAIDlg::OnPieceMoved(ST_PIECE_POS a_stSrc, ST_PIECE_POS a_stDst)
{
	RefreshCheckerBoard();
	CCellID cellID(a_stDst.m_nRow, a_stDst.m_nCol);
	m_grdCheckersBoard.SetFocusCell(cellID);
	CString strCurTurn = (m_pCheckerGame->GetPlayerTurn() == CHECKER_TEAM_RED) ? L"RED" : L"WHITE";
	SetDlgItemText(IDC_STATIC_AI_THINK, L"");
	SetDlgItemText(IDC_STATIC_CUR_TURN, strCurTurn);
}

void CCheckersAIDlg::OnGameOver(INT a_nGameResult)
{
	INT nResult = a_nGameResult;
	SetDlgItemText(IDC_STATIC_AI_THINK, L"");

	if(nResult == CHECKER_TEAM_RED)
		AfxMessageBox(L"Red WIN!!");
	else if(nResult == CHECKER_TEAM_WHITE)
		AfxMessageBox(L"White WIN!!");
	else if(nResult == CHECKER_GAME_TIE)
		AfxMessageBox(L"Match is TIE!!");

	ResetGame();
	CString strCurTurn = (m_pCheckerGame->GetPlayerTurn() == CHECKER_TEAM_RED) ? L"RED" : L"WHITE";
	SetDlgItemText(IDC_STATIC_CUR_TURN, strCurTurn);
}

void CCheckersAIDlg::RefreshCheckerBoard()
{
	// 체커보드 내용 그대로 그린다.
	for(INT c = 0; c < 8; c++)
		for(INT r = 0; r < 8; r++)
		{
			// 원래 있던 내용 삭제
			m_grdCheckersBoard.SetItemImage(r, c, -1);

			if(!m_pCheckerGame->IsPieceExist(r,c))
				continue;

			if(m_pCheckerGame->GetPieceTeam(r, c) == CHECKER_TEAM_RED)
			{
				if(m_pCheckerGame->IsPiecePromoted(r, c))
				{ 
					//m_grdCheckersBoard.SetItemText(r, c, L"R_K"); // 이미지가 로딩이 안될때를 대비해 텍스트 삽입
					m_grdCheckersBoard.SetItemImage(r, c, IMAGE_CHECKER_RED_K);
				}
				else
				{
					//m_grdCheckersBoard.SetItemText(r, c, L"R");
					m_grdCheckersBoard.SetItemImage(r, c, IMAGE_CHECKER_RED);
				}
			}
			else
			{
				if(m_pCheckerGame->IsPiecePromoted(r, c))
				{ 
					//m_grdCheckersBoard.SetItemText(r, c, L"W_K");
					m_grdCheckersBoard.SetItemImage(r, c, IMAGE_CHECKER_WHITE_K);
				}
				else
				{ 
					//m_grdCheckersBoard.SetItemText(r, c, L"W");
					m_grdCheckersBoard.SetItemImage(r, c, IMAGE_CHECKER_WHITE);
				}
			}
		}

	m_grdCheckersBoard.Invalidate();
}

void CCheckersAIDlg::ReColorCheckerBoard()
{
	for(INT c = 0; c < 8; c++)
		for(INT r = 0; r < 8; r++)
		{
			if((r + c) % 2 == 1)
				m_grdCheckersBoard.SetItemBkColour(r, c, RGB(48, 48, 255));
		}

	m_grdCheckersBoard.Invalidate();
}

void CCheckersAIDlg::ColorAvalialbePath(INT a_nRow, INT a_nCol)
{
	BOOL bColorFlag = FALSE;

	if(!m_pCheckerGame->IsPieceExist(a_nRow, a_nCol))
		return;

	ST_PIECE_POS stCurPos = { a_nRow, a_nCol };
	ST_PIECE_POS stNextPos = { 0, };

	if(m_pCheckerGame->GetPieceTeam(a_nRow, a_nCol) == CHECKER_TEAM_RED)
	{
		// 왼쪽 위로 이동 가능한지 검사
		stNextPos.m_nRow = stCurPos.m_nRow - 1;
		stNextPos.m_nCol = stCurPos.m_nCol - 1;
		if(m_pCheckerGame->CheckValidMovement(stCurPos, stNextPos))
		{
			m_grdCheckersBoard.SetItemBkColour(stNextPos.m_nRow, stNextPos.m_nCol, RGB(255, 64, 64));
			bColorFlag = TRUE;
		}

		if(!bColorFlag)
		{
			stNextPos.m_nRow = stCurPos.m_nRow - 2;
			stNextPos.m_nCol = stCurPos.m_nCol - 2;
			if(m_pCheckerGame->CheckValidMovement(stCurPos, stNextPos))
				m_grdCheckersBoard.SetItemBkColour(stNextPos.m_nRow, stNextPos.m_nCol, RGB(255, 64, 64));
		}

		// 오른쪽 위로 이동 가능한지 검사
		bColorFlag = FALSE;
		stNextPos.m_nRow = stCurPos.m_nRow - 1;
		stNextPos.m_nCol = stCurPos.m_nCol + 1;
		if(m_pCheckerGame->CheckValidMovement(stCurPos, stNextPos))
		{
			m_grdCheckersBoard.SetItemBkColour(stNextPos.m_nRow, stNextPos.m_nCol, RGB(255, 64, 64));
			bColorFlag = TRUE;
		}

		if(!bColorFlag)
		{
			stNextPos.m_nRow = stCurPos.m_nRow - 2;
			stNextPos.m_nCol = stCurPos.m_nCol + 2;
			if(m_pCheckerGame->CheckValidMovement(stCurPos, stNextPos))
				m_grdCheckersBoard.SetItemBkColour(stNextPos.m_nRow, stNextPos.m_nCol, RGB(255, 64, 64));
		}

		// 승격되지 않았다면 리턴
		if(!m_pCheckerGame->IsPiecePromoted(a_nRow, a_nCol))
		{
			m_grdCheckersBoard.Invalidate();
			return;
		}

		// 왼쪽 아래로 이동 가능한지 검사
		bColorFlag = FALSE;
		stNextPos.m_nRow = stCurPos.m_nRow + 1;
		stNextPos.m_nCol = stCurPos.m_nCol - 1;
		if(m_pCheckerGame->CheckValidMovement(stCurPos, stNextPos))
		{
			m_grdCheckersBoard.SetItemBkColour(stNextPos.m_nRow, stNextPos.m_nCol, RGB(255, 64, 64));
			bColorFlag = TRUE;
		}

		if(!bColorFlag)
		{
			stNextPos.m_nRow = stCurPos.m_nRow + 2;
			stNextPos.m_nCol = stCurPos.m_nCol - 2;
			if(m_pCheckerGame->CheckValidMovement(stCurPos, stNextPos))
				m_grdCheckersBoard.SetItemBkColour(stNextPos.m_nRow, stNextPos.m_nCol, RGB(255, 64, 64));
		}

		// 오른쪽 아래로 이동 가능한지 검사
		bColorFlag = FALSE;
		stNextPos.m_nRow = stCurPos.m_nRow + 1;
		stNextPos.m_nCol = stCurPos.m_nCol + 1;
		if(m_pCheckerGame->CheckValidMovement(stCurPos, stNextPos))
		{
			m_grdCheckersBoard.SetItemBkColour(stNextPos.m_nRow, stNextPos.m_nCol, RGB(255, 64, 64));
			bColorFlag = TRUE;
		}

		if(!bColorFlag)
		{
			stNextPos.m_nRow = stCurPos.m_nRow + 2;
			stNextPos.m_nCol = stCurPos.m_nCol + 2;
			if(m_pCheckerGame->CheckValidMovement(stCurPos, stNextPos))
				m_grdCheckersBoard.SetItemBkColour(stNextPos.m_nRow, stNextPos.m_nCol, RGB(255, 64, 64));
		}

		m_grdCheckersBoard.Invalidate();
		return;
	}

	// 왼쪽 아래로 이동 가능한지 검사
	stNextPos.m_nRow = stCurPos.m_nRow + 1;
	stNextPos.m_nCol = stCurPos.m_nCol - 1;
	if(m_pCheckerGame->CheckValidMovement(stCurPos, stNextPos))
	{
		m_grdCheckersBoard.SetItemBkColour(stNextPos.m_nRow, stNextPos.m_nCol, RGB(255, 64, 64));
		bColorFlag = TRUE;
	}

	if(!bColorFlag)
	{
		stNextPos.m_nRow = stCurPos.m_nRow + 2;
		stNextPos.m_nCol = stCurPos.m_nCol - 2;
		if(m_pCheckerGame->CheckValidMovement(stCurPos, stNextPos))
			m_grdCheckersBoard.SetItemBkColour(stNextPos.m_nRow, stNextPos.m_nCol, RGB(255, 64, 64));
	}

	// 오른쪽 아래로 이동 가능한지 검사
	stNextPos.m_nRow = stCurPos.m_nRow + 1;
	stNextPos.m_nCol = stCurPos.m_nCol + 1;
	if(m_pCheckerGame->CheckValidMovement(stCurPos, stNextPos))
	{
		m_grdCheckersBoard.SetItemBkColour(stNextPos.m_nRow, stNextPos.m_nCol, RGB(255, 64, 64));
		bColorFlag = TRUE;
	}

	if(!bColorFlag)
	{
		stNextPos.m_nRow = stCurPos.m_nRow + 2;
		stNextPos.m_nCol = stCurPos.m_nCol + 2;
		if(m_pCheckerGame->CheckValidMovement(stCurPos, stNextPos))
			m_grdCheckersBoard.SetItemBkColour(stNextPos.m_nRow, stNextPos.m_nCol, RGB(255, 64, 64));
	}

	// 승격되지 않았다면 리턴
	if(!m_pCheckerGame->IsPiecePromoted(a_nRow, a_nCol))
	{
		m_grdCheckersBoard.Invalidate();
		return;
	}

	// 왼쪽 위로 이동 가능한지 검사
	stNextPos.m_nRow = stCurPos.m_nRow - 1;
	stNextPos.m_nCol = stCurPos.m_nCol - 1;
	if(m_pCheckerGame->CheckValidMovement(stCurPos, stNextPos))
	{
		m_grdCheckersBoard.SetItemBkColour(stNextPos.m_nRow, stNextPos.m_nCol, RGB(255, 64, 64));
		bColorFlag = TRUE;
	}

	if(!bColorFlag)
	{
		stNextPos.m_nRow = stCurPos.m_nRow - 2;
		stNextPos.m_nCol = stCurPos.m_nCol - 2;
		if(m_pCheckerGame->CheckValidMovement(stCurPos, stNextPos))
			m_grdCheckersBoard.SetItemBkColour(stNextPos.m_nRow, stNextPos.m_nCol, RGB(255, 64, 64));
	}

	// 오른쪽 위로 이동 가능한지 검사
	stNextPos.m_nRow = stCurPos.m_nRow - 1;
	stNextPos.m_nCol = stCurPos.m_nCol + 1;
	if(m_pCheckerGame->CheckValidMovement(stCurPos, stNextPos))
	{
		m_grdCheckersBoard.SetItemBkColour(stNextPos.m_nRow, stNextPos.m_nCol, RGB(255, 64, 64));
		bColorFlag = TRUE;
	}

	if(!bColorFlag)
	{
		stNextPos.m_nRow = stCurPos.m_nRow - 2;
		stNextPos.m_nCol = stCurPos.m_nCol + 2;
		if(m_pCheckerGame->CheckValidMovement(stCurPos, stNextPos))
			m_grdCheckersBoard.SetItemBkColour(stNextPos.m_nRow, stNextPos.m_nCol, RGB(255, 64, 64));
	}

	m_grdCheckersBoard.Invalidate();
	return;
}

VOID CCheckersAIDlg::ResetGame()
{
	m_bRedPlayerAI = (m_RedPlayer)? TRUE : FALSE;
	m_bWhitePlayerAI = (m_WhitePlayer) ? TRUE : FALSE;

	m_pCheckerGame->InitalizeGame(m_bRedPlayerAI, m_bWhitePlayerAI);
	m_pCheckerGame->SetAIDifficulty(m_SliderDifRed.GetPos(), m_SliderDifWhite.GetPos());
	ReColorCheckerBoard();
	RefreshCheckerBoard();

	if(!m_pCheckerGame->IsCurrentPlayerAI())
		return;
	
	if(!m_pCheckerGame->PlayAITurn())
	{
		AfxMessageBox(L"Internal Error!");
		return;
	}
}

void CCheckersAIDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	if(m_pCheckerGame)
		delete m_pCheckerGame;

	if(m_pImageList)
	{
		m_pImageList->DeleteImageList();
		delete m_pImageList;
	}

	__super::OnClose();
}


void CCheckersAIDlg::OnBnClickedRadioWhiteAi()
{
	// TODO: Add your control notification handler code here
	m_WhitePlayer = 1;
	m_SliderDifWhite.EnableWindow(TRUE);
	
	((CButton*)GetDlgItem(IDC_RADIO_WHITE_HUMAN))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_RADIO_WHITE_AI))->SetCheck(1);
}


void CCheckersAIDlg::OnBnClickedRadioRedHuman()
{
	// TODO: Add your control notification handler code here
	m_RedPlayer = 0;
	m_SliderDifRed.EnableWindow(FALSE);
	((CButton*)GetDlgItem(IDC_RADIO_RED_HUMAN))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_RADIO_RED_AI))->SetCheck(0);
}


void CCheckersAIDlg::OnBnClickedRadioRedAi()
{
	// TODO: Add your control notification handler code here
	m_RedPlayer = 1;
	m_SliderDifRed.EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_RADIO_RED_HUMAN))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_RADIO_RED_AI))->SetCheck(1);
}


void CCheckersAIDlg::OnBnClickedRadioWhiteHuman()
{
	// TODO: Add your control notification handler code here
	m_WhitePlayer = 0;
	m_SliderDifWhite.EnableWindow(FALSE);
	((CButton*)GetDlgItem(IDC_RADIO_WHITE_HUMAN))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_RADIO_WHITE_AI))->SetCheck(0);
}


void CCheckersAIDlg::OnBnClickedBtnRestart()
{
	// TODO: Add your control notification handler code here
	SetDlgItemText(IDC_STATIC_AI_THINK, L"");
	ResetGame();
	CString strCurTurn = (m_pCheckerGame->GetPlayerTurn() == CHECKER_TEAM_RED) ? L"RED" : L"WHITE";
	SetDlgItemText(IDC_STATIC_CUR_TURN, strCurTurn);
	//SetDlgItemText(IDC_STATIC_CUR_TURN, strCurTurn);
	//Sleep(500);
}

