#include "stdafx.h"
#include "AI\NewAI.hpp"
#include "CheckerGame.h"
#include "CheckerPlayerAI.h"

CCheckerPlayerAI::CCheckerPlayerAI(INT a_nTeam, INT a_nDifficulty /*= 15*/)
{
	m_pInternalGame = new Game(false, false);
	m_pInternalAI = new NewAI(a_nDifficulty);
	m_pCheckerGame = NULL;
	m_nTeam = a_nTeam;
}

CCheckerPlayerAI::~CCheckerPlayerAI()
{
	delete m_pInternalGame;
	delete m_pInternalAI;
}

BOOL CCheckerPlayerAI::MakeMove()
{
	Move InternalMove;
	INT  nSrcIndex = 0;
	INT	 nSrcRow = 0;
	INT	 nSrcCol = 0;
	INT	 nDstIndex = 0;
	INT	 nDstRow = 0;
	INT	 nDstCol = 0;
	INT	 nMoveRow = 0;
	INT	 nMoveCol = 0;

	// Step 1. 현재 상태를 AI에게 알려준다. (라고 하자)
	if(!m_pCheckerGame)
		return FALSE;

	m_pInternalGame->TransformClassToBit(m_pCheckerGame);
	m_pInternalGame->SetTurn(m_nTeam);

	// Step 2. AI가 어디로 이동할지 알려준다
	InternalMove = m_pInternalAI->evaluate_game(*m_pInternalGame);
	nSrcIndex = InternalMove.src;
	TransformIndexToRowCol(nSrcIndex, &nSrcRow, &nSrcCol);
	nDstIndex = InternalMove.dst;
	TransformIndexToRowCol(nDstIndex, &nDstRow, &nDstCol);
	// jump = true 일시 이동칸 한칸 증가
	if(InternalMove.jump)
	{
		nMoveRow = nDstRow - nSrcRow;
		nMoveCol = nDstCol - nSrcCol;
		nDstRow += nMoveRow;
		nDstCol += nMoveCol;
	}

	// Step 3. 그리로 이동하면 된다.
	if(!m_pCheckerGame->MovePiece(m_pCheckerGame->GetPieceByPos(nSrcRow, nSrcCol), nDstRow, nDstCol))
		return FALSE;

	return TRUE;
}

VOID CCheckerPlayerAI::TransformIndexToRowCol(INT a_nIndex, INT* a_nRow, INT* a_nCol)
{
	INT nRetRow = 0;
	INT nRetCol = 0;

	nRetRow = 7 - (a_nIndex / 4);
	nRetCol = (a_nIndex % 4) * 2;
	if(nRetRow % 2 == 0)
		nRetCol += 1;

	*a_nRow = nRetRow;
	*a_nCol = nRetCol;
}


