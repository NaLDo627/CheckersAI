#include "stdafx.h"
#include "CheckerGame.h"
#include "CheckerPlayerAI.h"
#include "AI\NewAI.hpp"

CCheckerGame::CCheckerGame(BOOL m_bRedAI, BOOL m_bWhiteAI)
{
	for(INT c = 0; c < 8; c++)
		for(INT r = 0; r < 8; r++)
			m_pCheckerBoard[r][c] = NULL;
	m_pPlayerAI[CHECKER_AI_RED] = NULL;
	m_pPlayerAI[CHECKER_AI_WHITE] = NULL;
	m_bBonusTurn = FALSE;
	m_bPieceTakenOccured = FALSE;

	InitalizeGame(m_bRedAI, m_bWhiteAI);
}

CCheckerGame::~CCheckerGame()
{
	for(INT c = 0; c < 8; c++)
		for(INT r = 0; r < 8; r++)
		{
			if(m_pCheckerBoard[r][c])
				delete m_pCheckerBoard[r][c];
		}
	m_pPlayerAI[CHECKER_AI_RED] = NULL;
	m_pPlayerAI[CHECKER_AI_WHITE] = NULL;
	m_pEventHandler = NULL;
	
}

VOID CCheckerGame::InitalizeGame(BOOL m_bRedAI, BOOL m_bWhiteAI)
{
	// Step 1. 모든 요소 초기화
	for(INT c = 0; c < 8; c++)
		for(INT r = 0; r < 8; r++)
		{
			if(m_pCheckerBoard[r][c])
				delete m_pCheckerBoard[r][c];
			m_pCheckerBoard[r][c] = NULL;
		}
	if(m_pPlayerAI[CHECKER_AI_RED])
		delete m_pPlayerAI[CHECKER_AI_RED];
	if(m_pPlayerAI[CHECKER_AI_WHITE])
		delete m_pPlayerAI[CHECKER_AI_WHITE];
	m_pPlayerAI[CHECKER_AI_RED] = NULL;
	m_pPlayerAI[CHECKER_AI_WHITE] = NULL;

	// Step 2. 체커 룰대로 말 배치
	// 빨간팀 배치
	for(INT c = 0; c < 8; c++)
		for(INT r = 0; r < 3; r++)
		{
			if((c + r) % 2 == 1)
			{
				m_pCheckerBoard[r][c] = new CCheckerPiece;
				m_pCheckerBoard[r][c]->SetPosition(r, c);
				m_pCheckerBoard[r][c]->SetTeam(CHECKER_TEAM_WHITE);
			}
		}

	for(INT c = 0; c < 8; c++)
		for(INT r = 5; r < 8; r++)
		{
			if((r + c) % 2 == 1)
			{
				m_pCheckerBoard[r][c] = new CCheckerPiece;
				m_pCheckerBoard[r][c]->SetPosition(r, c);
				m_pCheckerBoard[r][c]->SetTeam(CHECKER_TEAM_RED);
			}
		}

	// Step 3. AI 플레이어 초기화
	if(m_bRedAI)
	{ 
		m_pPlayerAI[CHECKER_AI_RED] = new CCheckerPlayerAI(CHECKER_TEAM_RED);
		m_pPlayerAI[CHECKER_AI_RED]->SetCheckerGame(this);
	}

	if(m_bWhiteAI)
	{ 
		m_pPlayerAI[CHECKER_AI_WHITE] = new CCheckerPlayerAI(CHECKER_TEAM_WHITE);
		m_pPlayerAI[CHECKER_AI_WHITE]->SetCheckerGame(this);
	}

	// Step 4. 빨간색이 선으로 시작한다.
	m_nCurrentTurn = CHECKER_TEAM_RED;
	m_bPieceTakenOccured = FALSE;

	// AI가 레드일 경우 AI턴 시작
	if(m_bRedAI)
		m_pPlayerAI[CHECKER_AI_RED]->MakeMove(); 
}

BOOL CCheckerGame::MovePiece(CCheckerPiece* a_pCheckerPiece, INT a_nRow, INT a_nCol)
{
	INT nMiddleRow = 0;
	INT nMiddleCol = 0;
	INT nMoveValue = 0;
	ST_PIECE_POS stCurPos = {0,};
	ST_PIECE_POS stNextPos = { 0, };
	stNextPos.m_nRow = a_nRow;
	stNextPos.m_nCol = a_nCol;

	if(!a_pCheckerPiece)
		return FALSE;

	// Step 1. 유효한 이동인지 확인한다.
	stCurPos = a_pCheckerPiece->GetPosition();
	if(!CheckValidMovement(stCurPos, stNextPos))
		return FALSE;

	// Step 2. 이동 경로에 적 말이 있다면 따낸다.
	nMoveValue = a_nCol - stCurPos.m_nCol;
	if(ABS(nMoveValue) == 2)
	{
		nMiddleRow = (stCurPos.m_nRow + a_nRow) / 2;
		nMiddleCol = (stCurPos.m_nCol + a_nCol) / 2;

		if(m_pCheckerBoard[nMiddleRow][nMiddleCol])
			delete m_pCheckerBoard[nMiddleRow][nMiddleCol];
		m_pCheckerBoard[nMiddleRow][nMiddleCol] = NULL;
		m_bPieceTakenOccured = TRUE;
	}
	
	// Step 3. 이동한다.
	a_pCheckerPiece->SetPosition(stNextPos.m_nRow, stNextPos.m_nCol);
	m_pCheckerBoard[stNextPos.m_nRow][stNextPos.m_nCol] = a_pCheckerPiece;
	m_pCheckerBoard[stCurPos.m_nRow][stCurPos.m_nCol] = NULL;
	m_stLastMovedPos.m_nRow = stNextPos.m_nRow;
	m_stLastMovedPos.m_nCol = stNextPos.m_nCol;

	// 따먹은 경우 : 추가로 따먹을 수 있는 말이 있는지 검색. 있으면 턴을 바꾸지 않음
	if(!(m_bPieceTakenOccured && CheckPieceTakenAvailable(stNextPos)))
	{
		ChangeTurn();
		m_bPieceTakenOccured = FALSE;
		m_bBonusTurn = FALSE;
	}
	else
		m_bBonusTurn = TRUE;

	// 만약 반대쪽 행 끝쪽으로 이동했다면 승격
	if(stNextPos.m_nRow == 0 || stNextPos.m_nRow == 7)
		a_pCheckerPiece->PromoteThis();

	if(m_pEventHandler)
		m_pEventHandler->OnPieceMoved(stCurPos, stNextPos);

	CheckGameResult();

	if(m_pEventHandler)
	{
		if(m_nCurrentTurn == CHECKER_TEAM_RED)
			m_pEventHandler->OnPlayerRedTurn();
		else
			m_pEventHandler->OnPlayerWhiteTurn();
	}

	return TRUE;
}

BOOL CCheckerGame::CheckValidMovement(ST_PIECE_POS a_stCurPos, ST_PIECE_POS a_stNextPos)
{
	INT	nMoveValue = 0;
	INT nMiddleRow = 0;
	INT nMiddleCol = 0;

	// Step 1. 기본 체크
	if(!m_pCheckerBoard[a_stCurPos.m_nRow][a_stCurPos.m_nCol])
		return FALSE;
	if(a_stNextPos.m_nRow < 0 || a_stNextPos.m_nRow > 7)
		return FALSE;
	if(a_stNextPos.m_nCol < 0 || a_stNextPos.m_nCol > 7)
		return FALSE;

	// 대각선으로만 이동 가능하다. 즉 (이동할 위치 - 현재 위치)의 행, 열 차이값은 같아야함
	if(ABS(a_stNextPos.m_nRow - a_stCurPos.m_nRow) != ABS(a_stNextPos.m_nCol - a_stCurPos.m_nCol))
		return FALSE;

	// 팀별로 이동가능한 방향이 다르다. 빨강은 위로, 하양은 아래로 가야한다. 단, 승격이 되었다면 이는 무시한다.
	nMoveValue = a_stNextPos.m_nRow - a_stCurPos.m_nRow;
	if((m_pCheckerBoard[a_stCurPos.m_nRow][a_stCurPos.m_nCol]->GetTeam() == CHECKER_TEAM_RED && nMoveValue > 0)
		|| (m_pCheckerBoard[a_stCurPos.m_nRow][a_stCurPos.m_nCol]->GetTeam() == CHECKER_TEAM_WHITE && nMoveValue < 0))
	{
		if(!m_pCheckerBoard[a_stCurPos.m_nRow][a_stCurPos.m_nCol]->IsPromoted())
			return FALSE;
	}

	// Step 2. 이동가능 범위 체크
	// 참고 : 이동범위는 열 이동을 기준으로 체크한다. (행 기준도 상관없음)
	nMoveValue = a_stNextPos.m_nCol - a_stCurPos.m_nCol;

	// 만약 따먹기가 발생했다면 추가턴:
	if(m_bPieceTakenOccured)
	{
		// 추가 턴을 얻은 셀이 아니라면 실패
		if(a_stCurPos.m_nRow != m_stLastMovedPos.m_nRow ||
			a_stCurPos.m_nCol != m_stLastMovedPos.m_nCol)
			return FALSE;

		// 따먹을 수 있는 경로로만 가야함. 즉 두 칸이 아니면 실패
		if(ABS(nMoveValue) != 2)
			return FALSE;
	}

	// 3칸 이상 갈 수 없음
	if(ABS(nMoveValue) > 2)
		return FALSE;

	// 2칸 - 사이에 적 말이 있어야함, 도착지는 비어 있어야함
	if(ABS(nMoveValue) == 2)
	{
		nMiddleRow = (a_stCurPos.m_nRow + a_stNextPos.m_nRow) / 2;
		nMiddleCol = (a_stCurPos.m_nCol + a_stNextPos.m_nCol) / 2;

		if(m_pCheckerBoard[nMiddleRow][nMiddleCol] == NULL)
			return FALSE;

		if(m_pCheckerBoard[nMiddleRow][nMiddleCol]->GetTeam() == m_nCurrentTurn)
			return FALSE;

		if(m_pCheckerBoard[a_stNextPos.m_nRow][a_stNextPos.m_nCol] != NULL)
			return FALSE;

		return TRUE;
	}
	// 1칸 - 도착지는 비어있어야함
	// 추가 -> 만약 잡아먹을수 있는 말이 있다면 불가
	if(ABS(nMoveValue) == 1)
	{
		if(CheckMustJump(m_pCheckerBoard[a_stCurPos.m_nRow][a_stCurPos.m_nCol]->GetTeam()))
			return FALSE;

		if(m_pCheckerBoard[a_stNextPos.m_nRow][a_stNextPos.m_nCol] != NULL)
			return FALSE;

		return TRUE;
	}

	// 0칸 - 이동안함
	return FALSE;
}

VOID CCheckerGame::CheckGameResult()
{
	INT nGameResult = 0;
	INT nTeamRedCount = 0;
	INT nTeamWhiteCount = 0;
	BOOL bRedMoveable = FALSE;
	BOOL bWhiteMoveable = FALSE;

	for(INT c = 0; c < 8; c++)
		for(INT r = 0; r < 8; r++)
		{
			if(!m_pCheckerBoard[r][c])
				continue;

			if(m_pCheckerBoard[r][c]->GetTeam() == CHECKER_TEAM_RED)
			{
				nTeamRedCount++;
				if(IsMoveable(m_pCheckerBoard[r][c]))
					bRedMoveable = TRUE;
				continue;
			}

			nTeamWhiteCount++;
			if(IsMoveable(m_pCheckerBoard[r][c]))
				bWhiteMoveable = TRUE;
		}

	// 둘 다 0이면 뭔가 이상함, 에러 리턴
	if((nTeamRedCount + nTeamWhiteCount) == 0)
	{
		if(m_pEventHandler)
			m_pEventHandler->OnGameOver(CHECKER_ERROR);
	}

	// 둘다 움직일 수 없다면 비김
	if(!bRedMoveable && !bWhiteMoveable)
		nGameResult = CHECKER_GAME_TIE;

	// 빨강이 없거나, 빨강이 움직일 수 없고 빨강 턴이라면 하양 승
	else if((nTeamRedCount == 0) || (!bRedMoveable && m_nCurrentTurn == CHECKER_TEAM_RED))
		nGameResult = CHECKER_TEAM_WHITE;

	// 하양이 없거나, 하양이 움직일 수 없고 하양 턴이라면 빨강승
	else if((nTeamWhiteCount == 0) || (!bWhiteMoveable && m_nCurrentTurn == CHECKER_TEAM_WHITE))
		nGameResult = CHECKER_TEAM_RED;

	if(nGameResult != 0 && m_pEventHandler)
		m_pEventHandler->OnGameOver(nGameResult);
}

VOID CCheckerGame::SetAIDifficulty(INT a_nTeam, INT a_nDifficulty)
{
	if(!m_pPlayerAI[a_nTeam - 1])
		return;

	m_pPlayerAI[a_nTeam - 1]->SetCheckerAIDifficulty((UINT)a_nDifficulty);
}

BOOL CCheckerGame::PlayAITurn()
{
	if(!m_pPlayerAI[m_nCurrentTurn - 1])
		return FALSE;

	if(!(m_pPlayerAI[m_nCurrentTurn - 1]->MakeMove()))
		return FALSE;

	return TRUE;
}


VOID CCheckerGame::ChangeTurn()
{
	if(m_nCurrentTurn == CHECKER_TEAM_RED)
	{
		m_nCurrentTurn = CHECKER_TEAM_WHITE;
		return;
	}
	m_nCurrentTurn = CHECKER_TEAM_RED;
}

BOOL CCheckerGame::CheckPieceTakenAvailable(ST_PIECE_POS a_stCurPos)
{
	ST_PIECE_POS stCurPos = a_stCurPos;
	ST_PIECE_POS stNextPos = {0, };
	CCheckerPiece* pCurPiece = m_pCheckerBoard[stCurPos.m_nRow][stCurPos.m_nCol];

	if(m_nCurrentTurn == CHECKER_TEAM_RED)
	{
		// 왼쪽 위로 이동 가능한지 검사
		stNextPos.m_nRow = stCurPos.m_nRow - 2;
		stNextPos.m_nCol = stCurPos.m_nCol - 2;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		// 오른쪽 위로 이동 가능한지 검사
		stNextPos.m_nRow = stCurPos.m_nRow - 2;
		stNextPos.m_nCol = stCurPos.m_nCol + 2;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		// 승격되지 않았다면 리턴
		if(!pCurPiece->IsPromoted())
			return FALSE;

		// 왼쪽 아래로 이동 가능한지 검사
		stNextPos.m_nRow = stCurPos.m_nRow + 2;
		stNextPos.m_nCol = stCurPos.m_nCol - 2;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		// 오른쪽 아래로 이동 가능한지 검사
		stNextPos.m_nRow = stCurPos.m_nRow + 2;
		stNextPos.m_nCol = stCurPos.m_nCol + 2;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		return FALSE;
	}

	// 왼쪽 아래로 이동 가능한지 검사
	stNextPos.m_nRow = stCurPos.m_nRow + 2;
	stNextPos.m_nCol = stCurPos.m_nCol - 2;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	// 오른쪽 아래로 이동 가능한지 검사
	stNextPos.m_nRow = stCurPos.m_nRow + 2;
	stNextPos.m_nCol = stCurPos.m_nCol + 2;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	// 승격되지 않았다면 리턴
	if(!pCurPiece->IsPromoted())
		return FALSE;

	// 왼쪽 위로 이동 가능한지 검사
	stNextPos.m_nRow = stCurPos.m_nRow - 2;
	stNextPos.m_nCol = stCurPos.m_nCol - 2;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	// 오른쪽 위로 이동 가능한지 검사
	stNextPos.m_nRow = stCurPos.m_nRow - 2;
	stNextPos.m_nCol = stCurPos.m_nCol + 2;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	return FALSE;
}

BOOL CCheckerGame::IsMoveable(CCheckerPiece* a_pCheckerPiece)
{
	ST_PIECE_POS stCurPos = {0,};
	ST_PIECE_POS stNextPos = { 0, };

	if(!a_pCheckerPiece)
		return FALSE;

	stCurPos = a_pCheckerPiece->GetPosition();

	if(a_pCheckerPiece->GetTeam() == CHECKER_TEAM_RED)
	{
		// 왼쪽 위로 이동 가능한지 검사
		stNextPos.m_nRow = stCurPos.m_nRow - 1;
		stNextPos.m_nCol = stCurPos.m_nCol - 1;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		stNextPos.m_nRow = stCurPos.m_nRow - 2;
		stNextPos.m_nCol = stCurPos.m_nCol - 2;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		// 오른쪽 위로 이동 가능한지 검사
		stNextPos.m_nRow = stCurPos.m_nRow - 1;
		stNextPos.m_nCol = stCurPos.m_nCol + 1;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		stNextPos.m_nRow = stCurPos.m_nRow - 2;
		stNextPos.m_nCol = stCurPos.m_nCol + 2;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		// 승격되지 않았다면 리턴
		if(!a_pCheckerPiece->IsPromoted())
			return FALSE;

		// 왼쪽 아래로 이동 가능한지 검사
		stNextPos.m_nRow = stCurPos.m_nRow + 1;
		stNextPos.m_nCol = stCurPos.m_nCol - 1;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		stNextPos.m_nRow = stCurPos.m_nRow + 2;
		stNextPos.m_nCol = stCurPos.m_nCol - 2;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		// 오른쪽 아래로 이동 가능한지 검사
		stNextPos.m_nRow = stCurPos.m_nRow + 1;
		stNextPos.m_nCol = stCurPos.m_nCol + 1;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		stNextPos.m_nRow = stCurPos.m_nRow + 2;
		stNextPos.m_nCol = stCurPos.m_nCol + 2;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		return FALSE;
	}

	// 왼쪽 아래로 이동 가능한지 검사
	stNextPos.m_nRow = stCurPos.m_nRow + 1;
	stNextPos.m_nCol = stCurPos.m_nCol - 1;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	stNextPos.m_nRow = stCurPos.m_nRow + 2;
	stNextPos.m_nCol = stCurPos.m_nCol - 2;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	// 오른쪽 아래로 이동 가능한지 검사
	stNextPos.m_nRow = stCurPos.m_nRow + 1;
	stNextPos.m_nCol = stCurPos.m_nCol + 1;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	stNextPos.m_nRow = stCurPos.m_nRow + 2;
	stNextPos.m_nCol = stCurPos.m_nCol + 2;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	// 승격되지 않았다면 리턴
	if(!a_pCheckerPiece->IsPromoted())
		return FALSE;

	// 왼쪽 위로 이동 가능한지 검사
	stNextPos.m_nRow = stCurPos.m_nRow - 1;
	stNextPos.m_nCol = stCurPos.m_nCol - 1;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	stNextPos.m_nRow = stCurPos.m_nRow - 2;
	stNextPos.m_nCol = stCurPos.m_nCol - 2;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	// 오른쪽 위로 이동 가능한지 검사
	stNextPos.m_nRow = stCurPos.m_nRow - 1;
	stNextPos.m_nCol = stCurPos.m_nCol + 1;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	stNextPos.m_nRow = stCurPos.m_nRow - 2;
	stNextPos.m_nCol = stCurPos.m_nCol + 2;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	return FALSE;
}

BOOL CCheckerGame::CheckMustJump(INT a_nTeam)
{
	ST_PIECE_POS stCurPos = {0 ,};

	for(INT c = 0; c < 8; c++)
		for(INT r = 0; r < 8; r++)
		{
			if(!m_pCheckerBoard[r][c])
				continue;

			if(m_pCheckerBoard[r][c]->GetTeam() != a_nTeam)
				continue;

			stCurPos.m_nRow = r;
			stCurPos.m_nCol = c;

			if(CheckPieceTakenAvailable(stCurPos))
				return TRUE;
		}
	return FALSE;
}
