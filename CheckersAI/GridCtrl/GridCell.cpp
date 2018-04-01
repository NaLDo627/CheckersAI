// GridCell.cpp : implementation file
//
// MFC Grid Control - Main grid cell class
//
// Provides the implementation for the "default" cell type of the
// grid control. Adds in cell editing.
//
// Written by Chris Maunder <chris@codeproject.com>
// Copyright (c) 1998-2005. All Rights Reserved.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name and all copyright 
// notices remains intact. 
//
// An email letting me know how you are using it would be nice as well. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage/loss of business that
// this product may cause.
//
// For use with CGridCtrl v2.20+
//
// History:
// Eric Woodruff - 20 Feb 2000 - Added PrintCell() plus other minor changes
// Ken Bertelson - 12 Apr 2000 - Split CGridCell into CGridCell and CGridCellBase
// <kenbertelson@hotmail.com>
// C Maunder     - 17 Jun 2000 - Font handling optimsed, Added CGridDefaultCell
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GridCell.h"
#include "InPlaceEdit.h"
#include "GridCtrl.h"

// For Checker
static CCheckerGame s_CheckerGame;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CGridCell, CGridCellBase)
IMPLEMENT_DYNCREATE(CGridDefaultCell, CGridCell)

/////////////////////////////////////////////////////////////////////////////
// GridCell

CGridCell::CGridCell()
{
    m_plfFont = NULL;
	CGridCell::Reset();

	// üĿ���� ����
	//InitCheckersBoard();
	//m_bSelected = FALSE;
}

CGridCell::~CGridCell()
{
    delete m_plfFont;
}

/////////////////////////////////////////////////////////////////////////////
// GridCell Attributes

void CGridCell::operator=(const CGridCell& cell)
{
    if (this != &cell) CGridCellBase::operator=(cell);
}

void CGridCell::Reset()
{
    CGridCellBase::Reset();

    m_strText.Empty();
    m_nImage   = -1;
    m_lParam   = NULL;           // BUG FIX J. Bloggs 20/10/03
    m_pGrid    = NULL;
    m_bEditing = FALSE;
    m_pEditWnd = NULL;

    m_nFormat = (DWORD)-1;       // Use default from CGridDefaultCell
    m_crBkClr = CLR_DEFAULT;     // Background colour (or CLR_DEFAULT)
    m_crFgClr = CLR_DEFAULT;     // Forground colour (or CLR_DEFAULT)
    m_nMargin = (UINT)-1;        // Use default from CGridDefaultCell

    delete m_plfFont;
    m_plfFont = NULL;            // Cell font
}

void CGridCell::SetFont(const LOGFONT* plf)
{
    if (plf == NULL)
    {
        delete m_plfFont;
        m_plfFont = NULL;
    }
    else
    {
        if (!m_plfFont)
            m_plfFont = new LOGFONT;
        if (m_plfFont)
            memcpy(m_plfFont, plf, sizeof(LOGFONT)); 
    }
}

LOGFONT* CGridCell::GetFont() const
{
    if (m_plfFont == NULL)
    {
        CGridDefaultCell *pDefaultCell = (CGridDefaultCell*) GetDefaultCell();
        if (!pDefaultCell)
            return NULL;

        return pDefaultCell->GetFont();
    }

    return m_plfFont; 
}

CFont* CGridCell::GetFontObject() const
{
    // If the default font is specified, use the default cell implementation
    if (m_plfFont == NULL)
    {
        CGridDefaultCell *pDefaultCell = (CGridDefaultCell*) GetDefaultCell();
        if (!pDefaultCell)
            return NULL;

        return pDefaultCell->GetFontObject();
    }
    else
    {
        static CFont Font;
        Font.DeleteObject();
        Font.CreateFontIndirect(m_plfFont);
        return &Font;
    }
}

DWORD CGridCell::GetFormat() const
{
    if (m_nFormat == (DWORD)-1)
    {
        CGridDefaultCell *pDefaultCell = (CGridDefaultCell*) GetDefaultCell();
        if (!pDefaultCell)
            return 0;

        return pDefaultCell->GetFormat();
    }

    return m_nFormat; 
}

UINT CGridCell::GetMargin() const           
{
    if (m_nMargin == (UINT)-1)
    {
        CGridDefaultCell *pDefaultCell = (CGridDefaultCell*) GetDefaultCell();
        if (!pDefaultCell)
            return 0;

        return pDefaultCell->GetMargin();
    }

    return m_nMargin; 
}

/////////////////////////////////////////////////////////////////////////////
// GridCell Operations

BOOL CGridCell::Edit(int nRow, int nCol, CRect rect, CPoint /* point */, UINT nID, UINT nChar)
{
    if ( m_bEditing )
	{      
        if (m_pEditWnd)
		    m_pEditWnd->SendMessage ( WM_CHAR, nChar );    
    }  
	else  
	{   
		DWORD dwStyle = ES_LEFT;
		if (GetFormat() & DT_RIGHT) 
			dwStyle = ES_RIGHT;
		else if (GetFormat() & DT_CENTER) 
			dwStyle = ES_CENTER;
		
		m_bEditing = TRUE;
		
		// InPlaceEdit auto-deletes itself
		CGridCtrl* pGrid = GetGrid();
		m_pEditWnd = new CInPlaceEdit(pGrid, rect, dwStyle, nID, nRow, nCol, GetText(), nChar);
    }
    return TRUE;
}

void CGridCell::EndEdit()
{
    if (m_pEditWnd)
        ((CInPlaceEdit*)m_pEditWnd)->EndEdit();
}

void CGridCell::OnEndEdit()
{
    m_bEditing = FALSE;
    m_pEditWnd = NULL;
}

void CGridCell::RefreshCheckerBoard(CCheckerGame* a_pCheckerGame)
{
	CCheckerPiece* pPiece = NULL;
	CGridCtrl* pGrid = GetGrid();

	// üĿ���� ���� �״�� �׸���.
	for(INT c = 0; c < 8; c++)
		for(INT r = 0; r < 8; r++)
		{
			// ���� �ִ� ���� ����
			pGrid->SetItemText(r, c, _T(""));

			pPiece = a_pCheckerGame->GetPieceByPos(r, c);
			if(!pPiece)
				continue;

			if(pPiece->GetTeam() == CHECKER_TEAM_RED)
			{
				if(pPiece->IsPromoted())
					pGrid->SetItemText(r, c, L"King Red");
				else
					pGrid->SetItemText(r, c, L"Red");
			}
			else
			{ 
				if(pPiece->IsPromoted())
					pGrid->SetItemText(r, c, L"King White");
				else
					pGrid->SetItemText(r, c, L"White");
			}
		}

	pGrid->Invalidate();
}

void CGridCell::ReColorCheckerBoard()
{
	CGridCtrl* pGrid = GetGrid();

	for(INT c = 0; c < 8; c++)
		for(INT r = 0; r < 8; r++)
		{
			if((r + c) % 2 == 1)
				pGrid->SetItemBkColour(r, c, RGB(64, 64, 255));
		}

	pGrid->Invalidate();
}

void CGridCell::ColorAvalialbePath(CCheckerGame* a_pCheckerGame, INT a_nRow, INT a_nCol)
{
	CCheckerPiece* pPiece = a_pCheckerGame->GetPieceByPos(a_nRow, a_nCol);
	CGridCtrl* pGrid = GetGrid();

	// ������ ��: ���� -> ���� ��, ������ �� , �Ͼ� -> ���� �Ʒ�, ������ �Ʒ�
	// �°ݵ� ���¶�� �ݴ뵵 ����
	if(pPiece->GetTeam() == CHECKER_TEAM_RED)
	{
		// ���� ������ �������� �˻�
		if(a_nRow > 0 && a_nCol > 0)
		{
			if(a_pCheckerGame->GetPieceByPos(a_nRow - 1, a_nCol - 1) == NULL)
				pGrid->SetItemBkColour(a_nRow - 1, a_nCol - 1, RGB(255, 64, 64));
		}
		if(a_nRow > 1 && a_nCol > 1)
		{
			if(a_pCheckerGame->GetPieceByPos(a_nRow - 1, a_nCol - 1) != NULL &&		// �밢�� ���� ���� ���� �ְ�
				(a_pCheckerGame->GetPieceByPos(a_nRow - 1, a_nCol - 1)->GetTeam() != pPiece->GetTeam()) && // �� ���� ���� �������� �ƴϰ�
				a_pCheckerGame->GetPieceByPos(a_nRow - 2, a_nCol - 2) == NULL)		// �� �ʸӷ� ���� ������
				pGrid->SetItemBkColour(a_nRow - 2, a_nCol - 2, RGB(255, 64, 64));
		}

		// ������ ������ �������� �˻�
		if(a_nRow > 0 && a_nCol < 7)
		{
			if(a_pCheckerGame->GetPieceByPos(a_nRow - 1, a_nCol + 1) == NULL)
				pGrid->SetItemBkColour(a_nRow - 1, a_nCol + 1, RGB(255, 64, 64));
		}
		if(a_nRow > 1 && a_nCol < 6)
		{
			if(a_pCheckerGame->GetPieceByPos(a_nRow - 1, a_nCol + 1) != NULL &&		// �밢�� ������ ���� ���� �ְ�
				(a_pCheckerGame->GetPieceByPos(a_nRow - 1, a_nCol + 1)->GetTeam() != pPiece->GetTeam()) && // �� ���� ���� �������� �ƴϰ�
				a_pCheckerGame->GetPieceByPos(a_nRow - 2, a_nCol + 2) == NULL)		// �� �ʸӷ� ���� ������
				pGrid->SetItemBkColour(a_nRow - 2, a_nCol + 2, RGB(255, 64, 64));
		}

		// �°ݵ��� ���� ���¶�� ����
		if(!a_pCheckerGame->GetPieceByPos(a_nRow, a_nCol)->IsPromoted())
		{
			pGrid->Invalidate();
			return;
		}

		// �°ݵ� ���¶�� �밢�� �Ʒ��ʵ� �˻��Ѵ�.
		// �޾Ʒ� ������ �������� �˻�
		if(a_nRow < 7 && a_nCol > 0)
		{
			if(a_pCheckerGame->GetPieceByPos(a_nRow + 1, a_nCol - 1) == NULL)
				pGrid->SetItemBkColour(a_nRow + 1, a_nCol - 1, RGB(255, 64, 64));
		}
		if(a_nRow < 6 && a_nCol > 1)
		{
			if(a_pCheckerGame->GetPieceByPos(a_nRow + 1, a_nCol - 1) != NULL &&		// �밢�� ���� �Ʒ��� ���� �ְ�
				(a_pCheckerGame->GetPieceByPos(a_nRow + 1, a_nCol - 1)->GetTeam() != pPiece->GetTeam()) && // �� ���� ���� �������� �ƴϰ�
				a_pCheckerGame->GetPieceByPos(a_nRow + 2, a_nCol - 2) == NULL)		// �� �ʸӷ� ���� ������
				pGrid->SetItemBkColour(a_nRow + 2, a_nCol - 2, RGB(255, 64, 64));
		}

		// �����Ʒ� ������ �������� �˻�
		if(a_nRow < 7 && a_nCol < 7)
		{
			if(a_pCheckerGame->GetPieceByPos(a_nRow + 1, a_nCol + 1) == NULL)
				pGrid->SetItemBkColour(a_nRow + 1, a_nCol + 1, RGB(255, 64, 64));
		}
		if(a_nRow < 6 && a_nCol < 6)
		{
			if(a_pCheckerGame->GetPieceByPos(a_nRow + 1, a_nCol + 1) != NULL &&		// �밢�� ������ �Ʒ��� ���� �ְ�
				(a_pCheckerGame->GetPieceByPos(a_nRow + 1, a_nCol + 1)->GetTeam() != pPiece->GetTeam()) && // �� ���� ���� �������� �ƴϰ�
				a_pCheckerGame->GetPieceByPos(a_nRow + 2, a_nCol + 2) == NULL)		// �� �ʸӷ� ���� ������
				pGrid->SetItemBkColour(a_nRow + 2, a_nCol + 2, RGB(255, 64, 64));
		}
	}
	else
	{
		// �޾Ʒ� ������ �������� �˻�
		if(a_nRow < 7 && a_nCol > 0)
		{
			if(a_pCheckerGame->GetPieceByPos(a_nRow + 1, a_nCol - 1) == NULL)
				pGrid->SetItemBkColour(a_nRow + 1, a_nCol - 1, RGB(255, 64, 64));
		}
		if(a_nRow < 6 && a_nCol > 1)
		{
			if(a_pCheckerGame->GetPieceByPos(a_nRow + 1, a_nCol - 1) != NULL &&		// �밢�� ���� �Ʒ��� ���� �ְ�
				(a_pCheckerGame->GetPieceByPos(a_nRow + 1, a_nCol - 1)->GetTeam() != pPiece->GetTeam()) && // �� ���� ���� �������� �ƴϰ�
				a_pCheckerGame->GetPieceByPos(a_nRow + 2, a_nCol - 2) == NULL)		// �� �ʸӷ� ���� ������
				pGrid->SetItemBkColour(a_nRow + 2, a_nCol - 2, RGB(255, 64, 64));
		}

		// �����Ʒ� ������ �������� �˻�
		if(a_nRow < 7 && a_nCol < 7)
		{
			if(a_pCheckerGame->GetPieceByPos(a_nRow + 1, a_nCol + 1) == NULL)
				pGrid->SetItemBkColour(a_nRow + 1, a_nCol + 1, RGB(255, 64, 64));
		}
		if(a_nRow < 6 && a_nCol < 6)
		{
			if(a_pCheckerGame->GetPieceByPos(a_nRow + 1, a_nCol + 1) != NULL &&		// �밢�� ������ �Ʒ��� ���� �ְ�
				(a_pCheckerGame->GetPieceByPos(a_nRow + 1, a_nCol + 1)->GetTeam() != pPiece->GetTeam()) && // �� ���� ���� �������� �ƴϰ�
				a_pCheckerGame->GetPieceByPos(a_nRow + 2, a_nCol + 2) == NULL)		// �� �ʸӷ� ���� ������
				pGrid->SetItemBkColour(a_nRow + 2, a_nCol + 2, RGB(255, 64, 64));
		}

		// �°ݵ��� ���� ���¶�� ����
		if(!a_pCheckerGame->GetPieceByPos(a_nRow, a_nCol)->IsPromoted())
		{
			pGrid->Invalidate();
			return;
		}

		// �°ݵ� ���¶�� �밢�� �Ʒ��ʵ� �˻��Ѵ�.
		// ���� ������ �������� �˻�
		if(a_nRow > 0 && a_nCol > 0)
		{
			if(a_pCheckerGame->GetPieceByPos(a_nRow - 1, a_nCol - 1) == NULL)
				pGrid->SetItemBkColour(a_nRow - 1, a_nCol - 1, RGB(255, 64, 64));
		}
		if(a_nRow > 1 && a_nCol > 1)
		{
			if(a_pCheckerGame->GetPieceByPos(a_nRow - 1, a_nCol - 1) != NULL &&		// �밢�� ���� ���� ���� �ְ�
				(a_pCheckerGame->GetPieceByPos(a_nRow - 1, a_nCol - 1)->GetTeam() != pPiece->GetTeam()) && // �� ���� ���� �������� �ƴϰ�
				a_pCheckerGame->GetPieceByPos(a_nRow - 2, a_nCol - 2) == NULL)		// �� �ʸӷ� ���� ������
				pGrid->SetItemBkColour(a_nRow - 2, a_nCol - 2, RGB(255, 64, 64));
		}

		// ������ ������ �������� �˻�
		if(a_nRow > 0 && a_nCol < 7)
		{
			if(a_pCheckerGame->GetPieceByPos(a_nRow - 1, a_nCol + 1) == NULL)
				pGrid->SetItemBkColour(a_nRow - 1, a_nCol + 1, RGB(255, 64, 64));
		}
		if(a_nRow > 1 && a_nCol < 6)
		{
			if(a_pCheckerGame->GetPieceByPos(a_nRow - 1, a_nCol + 1) != NULL &&		// �밢�� ������ ���� ���� �ְ�
				(a_pCheckerGame->GetPieceByPos(a_nRow - 1, a_nCol + 1)->GetTeam() != pPiece->GetTeam()) && // �� ���� ���� �������� �ƴϰ�
				a_pCheckerGame->GetPieceByPos(a_nRow - 2, a_nCol + 2) == NULL)		// �� �ʸӷ� ���� ������
				pGrid->SetItemBkColour(a_nRow - 2, a_nCol + 2, RGB(255, 64, 64));
		}
	}

	pGrid->Invalidate();
	return;
}

void CGridCell::OnClick(CPoint PointCellRelative)
{
	static BOOL bSelected = FALSE;
	static BOOL bBonusOccured = FALSE;
	static INT nBeforeRow = -1;
	static INT nBeforeCol = -1;
	CGridCtrl* pGrid = GetGrid();
	CCellID	SelectedCell = pGrid->GetFocusCell();
	INT nSelectedRow = SelectedCell.row;
	INT nSelectedCol = SelectedCell.col;
	CCheckerPiece* pPiece = s_CheckerGame.GetPieceByPos(nSelectedRow, nSelectedCol);
	INT nCurrentTurn = s_CheckerGame.GetPlayerTurn();
	INT nResult = 0;
	
	if(nBeforeRow >= 0 && nBeforeCol >= 0)
		pGrid->SetItemFgColour(nBeforeRow, nBeforeCol, RGB(0, 0, 0));
	ReColorCheckerBoard();

	if(!bSelected)
	{
		// ��ĭ�̶�� �׳� ����
		if(!pPiece)
			return;

		// �ڱ� ���� �ƴ� ���̶�� �׳� ����
		if(pPiece->GetTeam() != nCurrentTurn)
			return;

		// ���ʽ� ���� ���: �Ʊ� �� ���� �ƴϸ� ����
		if(bBonusOccured &&
			((nBeforeRow != nSelectedRow) ||
			(nBeforeCol != nSelectedCol)))
			return;

		bSelected = TRUE;
		nBeforeRow = nSelectedRow;
		nBeforeCol = nSelectedCol;
		pGrid->SetItemFgColour(nBeforeRow, nBeforeCol, RGB(255, 64, 64));

		// �̵��� �� �ִ� �� ���� ����
		ColorAvalialbePath(&s_CheckerGame, nBeforeRow, nBeforeCol);
	}
	else
	{
		// ���õ� ����: ���� �ִ� ĭ �����ߴٸ� ���� ����
		if(pPiece)
		{
			// ���ʽ� ���� ���: �Ʊ� �� ���� �ƴϸ� ����
			if(bBonusOccured &&
				((nBeforeRow != nSelectedRow) ||
				(nBeforeCol != nSelectedCol)))
				return;

			// �ڱ� ���� �ƴ� ���̶�� �������� �� ����
			if(pPiece->GetTeam() != nCurrentTurn)
			{
				bSelected = FALSE;
				return;
			}
			nBeforeRow = nSelectedRow;
			nBeforeCol = nSelectedCol;
			pGrid->SetItemFgColour(nBeforeRow, nBeforeCol, RGB(255, 64, 64));

			// �̵��� �� �ִ� �� ���� ����
			ColorAvalialbePath(&s_CheckerGame, nBeforeRow, nBeforeCol);
			return;
		}

		// �̵� �ǽ�
		pPiece = s_CheckerGame.GetPieceByPos(nBeforeRow, nBeforeCol);
		if(s_CheckerGame.MovePiece(pPiece, nSelectedRow, nSelectedCol))
		{
			nBeforeRow = nSelectedRow;
			nBeforeCol = nSelectedCol;
		}

		// ���� ���� �ٲ��� �ʾҴٸ� ���ʽ� ��
		if(s_CheckerGame.GetPlayerTurn() == nCurrentTurn)
			bBonusOccured = TRUE;
		else
			bBonusOccured = FALSE;

		// ���� ����
		bSelected = FALSE;
		pGrid->SetItemFgColour(nBeforeRow, nBeforeCol, RGB(0, 0, 0));
		nResult = s_CheckerGame.GetGameResult();
		if(nResult == CHECKER_TEAM_RED)
		{
			AfxMessageBox(L"Red WIN!!");
			s_CheckerGame.InitalizeGame();
		}
		else if(nResult == CHECKER_TEAM_WHITE)
		{
			AfxMessageBox(L"White WIN!!");
			s_CheckerGame.InitalizeGame();
		}
		else if(nResult == CHECKER_GAME_TIE)
		{
			AfxMessageBox(L"Match is TIE!!");
			s_CheckerGame.InitalizeGame();
		}
	}

	RefreshCheckerBoard(&s_CheckerGame);
}




/////////////////////////////////////////////////////////////////////////////
// CGridDefaultCell

CGridDefaultCell::CGridDefaultCell() 
{
#ifdef _WIN32_WCE
    m_nFormat = DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX;
#else
    m_nFormat = DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX | DT_END_ELLIPSIS;
#endif
    m_crFgClr = CLR_DEFAULT;
    m_crBkClr = CLR_DEFAULT;
    m_Size    = CSize(30,10);
    m_dwStyle = 0;

#ifdef _WIN32_WCE
    LOGFONT lf;
    GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &lf);
    SetFont(&lf);
#else // not CE
    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    VERIFY(SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0));
    SetFont(&(ncm.lfMessageFont));
#endif
}

CGridDefaultCell::~CGridDefaultCell()
{
    m_Font.DeleteObject(); 
}

void CGridDefaultCell::SetFont(const LOGFONT* plf)
{
    ASSERT(plf);

    if (!plf) return;

    m_Font.DeleteObject();
    m_Font.CreateFontIndirect(plf);

    CGridCell::SetFont(plf);

    // Get the font size and hence the default cell size
    CDC* pDC = CDC::FromHandle(::GetDC(NULL));
    if (pDC)
    {
        CFont* pOldFont = pDC->SelectObject(&m_Font);

        SetMargin(pDC->GetTextExtent(_T(" "), 1).cx);
        m_Size = pDC->GetTextExtent(_T(" XXXXXXXXXXXX "), 14);
        m_Size.cy = (m_Size.cy * 3) / 2;

        pDC->SelectObject(pOldFont);
        ReleaseDC(NULL, pDC->GetSafeHdc());
    }
    else
    {
        SetMargin(3);
        m_Size = CSize(40,16);
    }
}

LOGFONT* CGridDefaultCell::GetFont() const
{
    ASSERT(m_plfFont);  // This is the default - it CAN'T be NULL!
    return m_plfFont;
}

CFont* CGridDefaultCell::GetFontObject() const
{
    ASSERT(m_Font.GetSafeHandle());
    return (CFont*) &m_Font; 
}

