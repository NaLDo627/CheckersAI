#include "stdafx.h"
#include "CheckerGame.h"

CCheckerGame::CCheckerGame()
{
	for(INT c = 0; c < 8; c++)
		for(INT r = 0; r < 8; r++)
			m_pCheckerBoard[r][c] = NULL;
	InitalizeGame();
}

CCheckerGame::~CCheckerGame()
{
	for(INT c = 0; c < 8; c++)
		for(INT r = 0; r < 8; r++)
		{
			if(m_pCheckerBoard[r][c])
				delete m_pCheckerBoard[r][c];
		}
}

VOID CCheckerGame::InitalizeGame()
{
	// Step 1. 모든 요소 초기화
	for(INT c = 0; c < 8; c++)
		for(INT r = 0; r < 8; r++)
		{
			if(m_pCheckerBoard[r][c])
				delete m_pCheckerBoard[r][c];
			m_pCheckerBoard[r][c] = NULL;
		}

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

	// Step 3. 빨간색이 선으로 시작한다.
	m_nCurrentTurn = CHECKER_TEAM_RED;
	m_bPieceTakenOccured = FALSE;
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
		m_stBonusTurnPos.m_nRow = stNextPos.m_nRow;
		m_stBonusTurnPos.m_nCol = stNextPos.m_nCol;
	}
	
	// Step 3. 이동한다.
	a_pCheckerPiece->SetPosition(stNextPos.m_nRow, stNextPos.m_nCol);
	m_pCheckerBoard[stNextPos.m_nRow][stNextPos.m_nCol] = a_pCheckerPiece;
	m_pCheckerBoard[stCurPos.m_nRow][stCurPos.m_nCol] = NULL;

	

	// 따먹은 경우 : 추가로 따먹을 수 있는 말이 있는지 검색. 있으면 턴을 바꾸지 않음
	if(!(m_bPieceTakenOccured && CheckPieceTakenAvailable(stNextPos)))
	{
		ChangeTurn();
		m_bPieceTakenOccured = FALSE;
	}

	// 만약 반대쪽 행 끝쪽으로 이동했다면 승격
	if(stNextPos.m_nRow == 0 || stNextPos.m_nRow == 7)
		a_pCheckerPiece->PromoteThis();

	return TRUE;
}

// 비기는 경우도 존재함.. 고려할것
INT CCheckerGame::GetGameResult()
{
	INT nTeamRedCount = 0;
	INT nTeamWhiteCount = 0;

	for(INT c = 0; c < 8; c++)
		for(INT r = 0; r < 8; r++)
		{
			if(m_pCheckerBoard[r][c])
			{
				if(m_pCheckerBoard[r][c]->GetTeam() == CHECKER_TEAM_RED)
					nTeamRedCount++;
				else
					nTeamWhiteCount++;
			}
		}

	// 둘 다 0이면 뭔가 이상함, 에러 리턴
	if((nTeamRedCount + nTeamWhiteCount) == 0)
		return -1;

	if(nTeamRedCount == 0)
		return CHECKER_TEAM_WHITE;
	else if(nTeamWhiteCount == 0)
		return CHECKER_TEAM_RED;

	// 비기는 경우 3 리턴 추가

	return 0;
}

VOID CCheckerGame::ChangeTurn()
{
	if(m_nCurrentTurn == CHECKER_TEAM_RED)
		m_nCurrentTurn = CHECKER_TEAM_WHITE;
	else
		m_nCurrentTurn = CHECKER_TEAM_RED;
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
	if((m_nCurrentTurn == CHECKER_TEAM_RED && nMoveValue > 0) 
		|| (m_nCurrentTurn == CHECKER_TEAM_WHITE && nMoveValue < 0))
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
		if(a_stCurPos.m_nRow != m_stBonusTurnPos.m_nRow ||
			a_stCurPos.m_nCol != m_stBonusTurnPos.m_nCol)
			return FALSE;

		// 따먹을 수 있는 경로로만 가야함. 즉 두 칸이 아니면 실패
		if(ABS(nMoveValue) != 2)
			return FALSE;
	}

	// 따먹기가 발생했다면 해당 셀만 이동가능
	

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
	if(ABS(nMoveValue) == 1)
	{
		if(m_pCheckerBoard[a_stNextPos.m_nRow][a_stNextPos.m_nCol] != NULL)
			return FALSE;

		return TRUE;
	}

	// 0칸 - 이동안함
	return FALSE;
}

BOOL CCheckerGame::CheckPieceTakenAvailable(ST_PIECE_POS a_stCurPos)
{
	INT nCurrentRow = a_stCurPos.m_nRow;
	INT nCurrentCol = a_stCurPos.m_nCol;
	CCheckerPiece* pCurPiece = m_pCheckerBoard[nCurrentRow][nCurrentCol];

	// 빨강일때 : 대각선 위쪽으로 적 말, 그 반대쪽에 말이 없다면 따먹기 가능
	if(m_nCurrentTurn == CHECKER_TEAM_RED)
	{
		// 왼위 쪽으로 가능한지 검사
		if(nCurrentRow > 1 && nCurrentCol > 1)
		{
			if(m_pCheckerBoard[nCurrentRow - 1][nCurrentCol - 1] != NULL &&		// 대각선 왼쪽 위로 말이 있고
				(m_pCheckerBoard[nCurrentRow - 1][nCurrentCol - 1]->GetTeam() != m_nCurrentTurn) && // 그 말의 팀이 같은편이 아니고
				m_pCheckerBoard[nCurrentRow - 2][nCurrentCol - 2] == NULL)		// 그 너머로 말이 없을때
				return TRUE;
		}

		// 오른위 쪽으로 가능한지 검사
		if(nCurrentRow > 1 && nCurrentCol < 6)
		{
			if(m_pCheckerBoard[nCurrentRow - 1][nCurrentCol + 1] != NULL &&		// 대각선 오른쪽 위로 말이 있고
				(m_pCheckerBoard[nCurrentRow - 1][nCurrentCol + 1]->GetTeam() != m_nCurrentTurn) && // 그 말의 팀이 같은편이 아니고
				m_pCheckerBoard[nCurrentRow - 2][nCurrentCol + 2] == NULL)		// 그 너머로 말이 없을때
				return TRUE;
		}

		// 승격되지 않은 상태라면 리턴
		if(!pCurPiece->IsPromoted())
			return FALSE;

		// 승격된 상태라면 대각선 아래쪽도 검사한다.
		// 왼아래 쪽으로 가능한지 검사
		if(nCurrentRow < 6 && nCurrentCol > 1)
		{
			if(m_pCheckerBoard[nCurrentRow + 1][nCurrentCol - 1] != NULL &&		// 대각선 왼쪽 아래로 말이 있고
				(m_pCheckerBoard[nCurrentRow + 1][nCurrentCol - 1]->GetTeam() != m_nCurrentTurn) && // 그 말의 팀이 같은편이 아니고
				m_pCheckerBoard[nCurrentRow + 2][nCurrentCol - 2] == NULL)		// 그 너머로 말이 없을때
				return TRUE;
		}

		// 오른아래 쪽으로 가능한지 검사
		if(nCurrentRow < 6 && nCurrentCol < 6)
		{
			if(m_pCheckerBoard[nCurrentRow + 1][nCurrentCol + 1] != NULL &&		// 대각선 오른쪽 아래로 말이 있고
				(m_pCheckerBoard[nCurrentRow + 1][nCurrentCol + 1]->GetTeam() != m_nCurrentTurn) && // 그 말의 팀이 같은편이 아니고
				m_pCheckerBoard[nCurrentRow + 2][nCurrentCol + 2] == NULL)		// 그 너머로 말이 없을때
				return TRUE;
		}
	}
	// 하양일때는 반대
	else
	{
		// 왼아래 쪽으로 가능한지 검사
		if(nCurrentRow < 6 && nCurrentCol > 1)
		{
			if(m_pCheckerBoard[nCurrentRow + 1][nCurrentCol - 1] != NULL &&		// 대각선 왼쪽 아래로 말이 있고
				(m_pCheckerBoard[nCurrentRow + 1][nCurrentCol - 1]->GetTeam() != m_nCurrentTurn) && // 그 말의 팀이 같은편이 아니고
				m_pCheckerBoard[nCurrentRow + 2][nCurrentCol - 2] == NULL)		// 그 너머로 말이 없을때
				return TRUE;
		}

		// 오른아래 쪽으로 가능한지 검사
		if(nCurrentRow < 6 && nCurrentCol < 6)
		{
			if(m_pCheckerBoard[nCurrentRow + 1][nCurrentCol + 1] != NULL &&		// 대각선 오른쪽 아래로 말이 있고
				(m_pCheckerBoard[nCurrentRow + 1][nCurrentCol + 1]->GetTeam() != m_nCurrentTurn) && // 그 말의 팀이 같은편이 아니고
				m_pCheckerBoard[nCurrentRow + 2][nCurrentCol + 2] == NULL)		// 그 너머로 말이 없을때
				return TRUE;
		}

		// 승격되지 않은 상태라면 리턴
		if(!pCurPiece->IsPromoted())
			return FALSE;

		// 승격된 상태라면 대각선 아래쪽도 검사한다.
		// 왼위 쪽으로 가능한지 검사
		if(nCurrentRow > 1 && nCurrentCol > 1)
		{
			if(m_pCheckerBoard[nCurrentRow - 1][nCurrentCol - 1] != NULL &&		// 대각선 왼쪽 위로 말이 있고
				(m_pCheckerBoard[nCurrentRow - 1][nCurrentCol - 1]->GetTeam() != m_nCurrentTurn) && // 그 말의 팀이 같은편이 아니고
				m_pCheckerBoard[nCurrentRow - 2][nCurrentCol - 2] == NULL)		// 그 너머로 말이 없을때
				return TRUE;
		}

		// 오른위 쪽으로 가능한지 검사
		if(nCurrentRow > 1 && nCurrentCol < 6)
		{
			if(m_pCheckerBoard[nCurrentRow - 1][nCurrentCol + 1] != NULL &&		// 대각선 오른쪽 위로 말이 있고
				(m_pCheckerBoard[nCurrentRow - 1][nCurrentCol + 1]->GetTeam() != m_nCurrentTurn) && // 그 말의 팀이 같은편이 아니고
				m_pCheckerBoard[nCurrentRow - 2][nCurrentCol + 2] == NULL)		// 그 너머로 말이 없을때
				return TRUE;
		}
	}

	return FALSE;
}
