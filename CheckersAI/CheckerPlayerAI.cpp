#include "stdafx.h"
#include "AI\NewAI.hpp"
#include "CheckerGame.h"
#include "CheckerPlayerAI.h"

static BOOL s_nMoveCount = 0;
static CMutex s_mtx;

VOID TransformIndexToRowCol(INT a_nIndex, INT* a_nRow, INT* a_nCol)
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

UINT WorkerThread(LPVOID a_pParam)
{
	PST_THREAD_PARAM pstParam = (PST_THREAD_PARAM)a_pParam;
	BOOL* pThreadRunning = pstParam->m_bThreadRunning;
	Game InternalGame(false, false);
	NewAI InternalAI(pstParam->m_nDifficluty);
	CCheckerGame* pCheckerGame = pstParam->m_pCheckerGame;
	Move InternalMove;
	INT  nSrcIndex = 0;
	INT	 nSrcRow = 0;
	INT	 nSrcCol = 0;
	INT	 nDstIndex = 0;
	INT	 nDstRow = 0;
	INT	 nDstCol = 0;
	INT	 nMoveRow = 0;
	INT	 nMoveCol = 0;

	while(1)
	{
		if(!pCheckerGame || !(s_nMoveCount > 0))
		{
			if(!*pThreadRunning)
				return 0;

			Sleep(1);
			continue;
		}

		s_mtx.Lock();
		Sleep(10);
		InternalGame.TransformClassToBit(pCheckerGame);
		InternalGame.SetTurn(pCheckerGame->GetPlayerTurn());
		InternalAI.difficulty(pstParam->m_nDifficluty);

		// Step 2. AI가 어디로 이동할지 알려준다.
		InternalMove = InternalAI.evaluate_game(InternalGame);

		// 이동할 경로가 없다면 게임 끝
		if(InternalMove.src == 0 && InternalMove.dst == 0)
			return 0;

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
		s_mtx.Unlock();

		// Step 3. 그리로 이동하면 된다.
		if(!pCheckerGame->MovePiece(pCheckerGame->GetPieceByPos(nSrcRow, nSrcCol), nDstRow, nDstCol))
			continue;

		s_nMoveCount--;
	}

	return 0;
}

CCheckerPlayerAI::CCheckerPlayerAI(INT a_nTeam, INT a_nDifficulty /*= 15*/)
{
	m_pCheckerGame = NULL;
	m_nTeam = a_nTeam;
	m_bThreadRunning = TRUE;
	m_stThreadParam.m_bThreadRunning = &m_bThreadRunning;
	m_stThreadParam.m_pCheckerGame = NULL;
	m_stThreadParam.m_nDifficluty = a_nDifficulty;
	m_pThread = NULL;
	//m_pThread = ::AfxBeginThread(WorkerThread, &m_stThreadParam);
}

CCheckerPlayerAI::~CCheckerPlayerAI()
{
	m_bThreadRunning = FALSE;
	Sleep(100); // 스레드가 종료될 틈을 줌 (솔직히 맘에는 안듬)
}

BOOL CCheckerPlayerAI::MakeMove()
{
	if(!m_pThread)
		m_pThread = ::AfxBeginThread(WorkerThread, &m_stThreadParam);

	s_nMoveCount++;
	
	return TRUE;
}

VOID CCheckerPlayerAI::SetCheckerAIDifficulty(UINT a_nDifficulty)
{ 
	m_stThreadParam.m_nDifficluty = a_nDifficulty;
}




