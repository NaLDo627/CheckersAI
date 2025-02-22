
// CheckersAIDlg.h : header file
//

#pragma once
#include "afxcmn.h"

#define IMAGE_CHECKER_RED		0
#define IMAGE_CHECKER_RED_K		1
#define IMAGE_CHECKER_WHITE		2
#define IMAGE_CHECKER_WHITE_K	3

#define STRING_AI_THINKING		
const WCHAR s_szSTRING_AI_THINKING[] = { L"AI가 생각중입니다..." };

//class CGridCtrl;
// CCheckersAIDlg dialog
class CCheckersAIDlg :	public CDialog,
						public CGridEventHandler,
						public CCheckerEventHandler
{
// Construction
public:
	CCheckersAIDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHECKERSAI_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	// Checkers 게임 관련 함수
	VOID InitImageList();
	VOID InitCheckersBoard();
	VOID RefreshCheckerBoard();
	VOID ReColorCheckerBoard();
	VOID ColorAvalialbePath(INT a_nRow, INT a_nCol);
	VOID ResetGame();

	// Grid event handler
	virtual VOID OnGridClick(INT a_nRow, INT a_nCol);

	// Game event handler
	virtual void OnPlayerRedTurn();
	virtual void OnPlayerWhiteTurn();
	virtual void OnPieceMoved(ST_PIECE_POS a_stSrc, ST_PIECE_POS a_stDst);
	virtual void OnGameOver(INT a_nGameResult);

private:
	CImageList* m_pImageList;
	CGridCtrl	m_grdCheckersBoard;
	CCheckerGame*	m_pCheckerGame;


	// 자동생성
public:
	afx_msg void OnClose();
	afx_msg void OnBnClickedRadioWhiteAi();
	afx_msg void OnBnClickedRadioRedHuman();
	afx_msg void OnBnClickedRadioRedAi();
	afx_msg void OnBnClickedRadioWhiteHuman();
	afx_msg void OnBnClickedBtnRestart();
	CSliderCtrl m_SliderDifRed;
	CSliderCtrl m_SliderDifWhite;
	INT m_RedPlayer;		// RadioBox 용
	INT m_WhitePlayer;		// RadioBox 용
	BOOL m_bRedPlayerAI;
	BOOL m_bWhitePlayerAI;
};
