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
	// Step 1. ��� ��� �ʱ�ȭ
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

	// Step 2. üĿ ���� �� ��ġ
	// ������ ��ġ
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

	// Step 3. AI �÷��̾� �ʱ�ȭ
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

	// Step 4. �������� ������ �����Ѵ�.
	m_nCurrentTurn = CHECKER_TEAM_RED;
	m_bPieceTakenOccured = FALSE;

	// AI�� ������ ��� AI�� ����
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

	// Step 1. ��ȿ�� �̵����� Ȯ���Ѵ�.
	stCurPos = a_pCheckerPiece->GetPosition();
	if(!CheckValidMovement(stCurPos, stNextPos))
		return FALSE;

	// Step 2. �̵� ��ο� �� ���� �ִٸ� ������.
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
	
	// Step 3. �̵��Ѵ�.
	a_pCheckerPiece->SetPosition(stNextPos.m_nRow, stNextPos.m_nCol);
	m_pCheckerBoard[stNextPos.m_nRow][stNextPos.m_nCol] = a_pCheckerPiece;
	m_pCheckerBoard[stCurPos.m_nRow][stCurPos.m_nCol] = NULL;
	m_stLastMovedPos.m_nRow = stNextPos.m_nRow;
	m_stLastMovedPos.m_nCol = stNextPos.m_nCol;

	// ������ ��� : �߰��� ������ �� �ִ� ���� �ִ��� �˻�. ������ ���� �ٲ��� ����
	if(!(m_bPieceTakenOccured && CheckPieceTakenAvailable(stNextPos)))
	{
		ChangeTurn();
		m_bPieceTakenOccured = FALSE;
		m_bBonusTurn = FALSE;
	}
	else
		m_bBonusTurn = TRUE;

	// ���� �ݴ��� �� �������� �̵��ߴٸ� �°�
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

	// Step 1. �⺻ üũ
	if(!m_pCheckerBoard[a_stCurPos.m_nRow][a_stCurPos.m_nCol])
		return FALSE;
	if(a_stNextPos.m_nRow < 0 || a_stNextPos.m_nRow > 7)
		return FALSE;
	if(a_stNextPos.m_nCol < 0 || a_stNextPos.m_nCol > 7)
		return FALSE;

	// �밢�����θ� �̵� �����ϴ�. �� (�̵��� ��ġ - ���� ��ġ)�� ��, �� ���̰��� ���ƾ���
	if(ABS(a_stNextPos.m_nRow - a_stCurPos.m_nRow) != ABS(a_stNextPos.m_nCol - a_stCurPos.m_nCol))
		return FALSE;

	// ������ �̵������� ������ �ٸ���. ������ ����, �Ͼ��� �Ʒ��� �����Ѵ�. ��, �°��� �Ǿ��ٸ� �̴� �����Ѵ�.
	nMoveValue = a_stNextPos.m_nRow - a_stCurPos.m_nRow;
	if((m_pCheckerBoard[a_stCurPos.m_nRow][a_stCurPos.m_nCol]->GetTeam() == CHECKER_TEAM_RED && nMoveValue > 0)
		|| (m_pCheckerBoard[a_stCurPos.m_nRow][a_stCurPos.m_nCol]->GetTeam() == CHECKER_TEAM_WHITE && nMoveValue < 0))
	{
		if(!m_pCheckerBoard[a_stCurPos.m_nRow][a_stCurPos.m_nCol]->IsPromoted())
			return FALSE;
	}

	// Step 2. �̵����� ���� üũ
	// ���� : �̵������� �� �̵��� �������� üũ�Ѵ�. (�� ���ص� �������)
	nMoveValue = a_stNextPos.m_nCol - a_stCurPos.m_nCol;

	// ���� ���ԱⰡ �߻��ߴٸ� �߰���:
	if(m_bPieceTakenOccured)
	{
		// �߰� ���� ���� ���� �ƴ϶�� ����
		if(a_stCurPos.m_nRow != m_stLastMovedPos.m_nRow ||
			a_stCurPos.m_nCol != m_stLastMovedPos.m_nCol)
			return FALSE;

		// ������ �� �ִ� ��ηθ� ������. �� �� ĭ�� �ƴϸ� ����
		if(ABS(nMoveValue) != 2)
			return FALSE;
	}

	// 3ĭ �̻� �� �� ����
	if(ABS(nMoveValue) > 2)
		return FALSE;

	// 2ĭ - ���̿� �� ���� �־����, �������� ��� �־����
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
	// 1ĭ - �������� ����־����
	// �߰� -> ���� ��Ƹ����� �ִ� ���� �ִٸ� �Ұ�
	if(ABS(nMoveValue) == 1)
	{
		if(CheckMustJump(m_pCheckerBoard[a_stCurPos.m_nRow][a_stCurPos.m_nCol]->GetTeam()))
			return FALSE;

		if(m_pCheckerBoard[a_stNextPos.m_nRow][a_stNextPos.m_nCol] != NULL)
			return FALSE;

		return TRUE;
	}

	// 0ĭ - �̵�����
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

	// �� �� 0�̸� ���� �̻���, ���� ����
	if((nTeamRedCount + nTeamWhiteCount) == 0)
	{
		if(m_pEventHandler)
			m_pEventHandler->OnGameOver(CHECKER_ERROR);
	}

	// �Ѵ� ������ �� ���ٸ� ���
	if(!bRedMoveable && !bWhiteMoveable)
		nGameResult = CHECKER_GAME_TIE;

	// ������ ���ų�, ������ ������ �� ���� ���� ���̶�� �Ͼ� ��
	else if((nTeamRedCount == 0) || (!bRedMoveable && m_nCurrentTurn == CHECKER_TEAM_RED))
		nGameResult = CHECKER_TEAM_WHITE;

	// �Ͼ��� ���ų�, �Ͼ��� ������ �� ���� �Ͼ� ���̶�� ������
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
		// ���� ���� �̵� �������� �˻�
		stNextPos.m_nRow = stCurPos.m_nRow - 2;
		stNextPos.m_nCol = stCurPos.m_nCol - 2;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		// ������ ���� �̵� �������� �˻�
		stNextPos.m_nRow = stCurPos.m_nRow - 2;
		stNextPos.m_nCol = stCurPos.m_nCol + 2;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		// �°ݵ��� �ʾҴٸ� ����
		if(!pCurPiece->IsPromoted())
			return FALSE;

		// ���� �Ʒ��� �̵� �������� �˻�
		stNextPos.m_nRow = stCurPos.m_nRow + 2;
		stNextPos.m_nCol = stCurPos.m_nCol - 2;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		// ������ �Ʒ��� �̵� �������� �˻�
		stNextPos.m_nRow = stCurPos.m_nRow + 2;
		stNextPos.m_nCol = stCurPos.m_nCol + 2;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		return FALSE;
	}

	// ���� �Ʒ��� �̵� �������� �˻�
	stNextPos.m_nRow = stCurPos.m_nRow + 2;
	stNextPos.m_nCol = stCurPos.m_nCol - 2;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	// ������ �Ʒ��� �̵� �������� �˻�
	stNextPos.m_nRow = stCurPos.m_nRow + 2;
	stNextPos.m_nCol = stCurPos.m_nCol + 2;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	// �°ݵ��� �ʾҴٸ� ����
	if(!pCurPiece->IsPromoted())
		return FALSE;

	// ���� ���� �̵� �������� �˻�
	stNextPos.m_nRow = stCurPos.m_nRow - 2;
	stNextPos.m_nCol = stCurPos.m_nCol - 2;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	// ������ ���� �̵� �������� �˻�
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
		// ���� ���� �̵� �������� �˻�
		stNextPos.m_nRow = stCurPos.m_nRow - 1;
		stNextPos.m_nCol = stCurPos.m_nCol - 1;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		stNextPos.m_nRow = stCurPos.m_nRow - 2;
		stNextPos.m_nCol = stCurPos.m_nCol - 2;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		// ������ ���� �̵� �������� �˻�
		stNextPos.m_nRow = stCurPos.m_nRow - 1;
		stNextPos.m_nCol = stCurPos.m_nCol + 1;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		stNextPos.m_nRow = stCurPos.m_nRow - 2;
		stNextPos.m_nCol = stCurPos.m_nCol + 2;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		// �°ݵ��� �ʾҴٸ� ����
		if(!a_pCheckerPiece->IsPromoted())
			return FALSE;

		// ���� �Ʒ��� �̵� �������� �˻�
		stNextPos.m_nRow = stCurPos.m_nRow + 1;
		stNextPos.m_nCol = stCurPos.m_nCol - 1;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		stNextPos.m_nRow = stCurPos.m_nRow + 2;
		stNextPos.m_nCol = stCurPos.m_nCol - 2;
		if(CheckValidMovement(stCurPos, stNextPos))
			return TRUE;

		// ������ �Ʒ��� �̵� �������� �˻�
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

	// ���� �Ʒ��� �̵� �������� �˻�
	stNextPos.m_nRow = stCurPos.m_nRow + 1;
	stNextPos.m_nCol = stCurPos.m_nCol - 1;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	stNextPos.m_nRow = stCurPos.m_nRow + 2;
	stNextPos.m_nCol = stCurPos.m_nCol - 2;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	// ������ �Ʒ��� �̵� �������� �˻�
	stNextPos.m_nRow = stCurPos.m_nRow + 1;
	stNextPos.m_nCol = stCurPos.m_nCol + 1;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	stNextPos.m_nRow = stCurPos.m_nRow + 2;
	stNextPos.m_nCol = stCurPos.m_nCol + 2;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	// �°ݵ��� �ʾҴٸ� ����
	if(!a_pCheckerPiece->IsPromoted())
		return FALSE;

	// ���� ���� �̵� �������� �˻�
	stNextPos.m_nRow = stCurPos.m_nRow - 1;
	stNextPos.m_nCol = stCurPos.m_nCol - 1;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	stNextPos.m_nRow = stCurPos.m_nRow - 2;
	stNextPos.m_nCol = stCurPos.m_nCol - 2;
	if(CheckValidMovement(stCurPos, stNextPos))
		return TRUE;

	// ������ ���� �̵� �������� �˻�
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
