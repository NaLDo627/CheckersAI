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
	// Step 1. ��� ��� �ʱ�ȭ
	for(INT c = 0; c < 8; c++)
		for(INT r = 0; r < 8; r++)
		{
			if(m_pCheckerBoard[r][c])
				delete m_pCheckerBoard[r][c];
			m_pCheckerBoard[r][c] = NULL;
		}

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

	// Step 3. �������� ������ �����Ѵ�.
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
		m_stBonusTurnPos.m_nRow = stNextPos.m_nRow;
		m_stBonusTurnPos.m_nCol = stNextPos.m_nCol;
	}
	
	// Step 3. �̵��Ѵ�.
	a_pCheckerPiece->SetPosition(stNextPos.m_nRow, stNextPos.m_nCol);
	m_pCheckerBoard[stNextPos.m_nRow][stNextPos.m_nCol] = a_pCheckerPiece;
	m_pCheckerBoard[stCurPos.m_nRow][stCurPos.m_nCol] = NULL;

	

	// ������ ��� : �߰��� ������ �� �ִ� ���� �ִ��� �˻�. ������ ���� �ٲ��� ����
	if(!(m_bPieceTakenOccured && CheckPieceTakenAvailable(stNextPos)))
	{
		ChangeTurn();
		m_bPieceTakenOccured = FALSE;
	}

	// ���� �ݴ��� �� �������� �̵��ߴٸ� �°�
	if(stNextPos.m_nRow == 0 || stNextPos.m_nRow == 7)
		a_pCheckerPiece->PromoteThis();

	return TRUE;
}

// ���� ��쵵 ������.. ����Ұ�
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

	// �� �� 0�̸� ���� �̻���, ���� ����
	if((nTeamRedCount + nTeamWhiteCount) == 0)
		return -1;

	if(nTeamRedCount == 0)
		return CHECKER_TEAM_WHITE;
	else if(nTeamWhiteCount == 0)
		return CHECKER_TEAM_RED;

	// ���� ��� 3 ���� �߰�

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
	if((m_nCurrentTurn == CHECKER_TEAM_RED && nMoveValue > 0) 
		|| (m_nCurrentTurn == CHECKER_TEAM_WHITE && nMoveValue < 0))
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
		if(a_stCurPos.m_nRow != m_stBonusTurnPos.m_nRow ||
			a_stCurPos.m_nCol != m_stBonusTurnPos.m_nCol)
			return FALSE;

		// ������ �� �ִ� ��ηθ� ������. �� �� ĭ�� �ƴϸ� ����
		if(ABS(nMoveValue) != 2)
			return FALSE;
	}

	// ���ԱⰡ �߻��ߴٸ� �ش� ���� �̵�����
	

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
	if(ABS(nMoveValue) == 1)
	{
		if(m_pCheckerBoard[a_stNextPos.m_nRow][a_stNextPos.m_nCol] != NULL)
			return FALSE;

		return TRUE;
	}

	// 0ĭ - �̵�����
	return FALSE;
}

BOOL CCheckerGame::CheckPieceTakenAvailable(ST_PIECE_POS a_stCurPos)
{
	INT nCurrentRow = a_stCurPos.m_nRow;
	INT nCurrentCol = a_stCurPos.m_nCol;
	CCheckerPiece* pCurPiece = m_pCheckerBoard[nCurrentRow][nCurrentCol];

	// �����϶� : �밢�� �������� �� ��, �� �ݴ��ʿ� ���� ���ٸ� ���Ա� ����
	if(m_nCurrentTurn == CHECKER_TEAM_RED)
	{
		// ���� ������ �������� �˻�
		if(nCurrentRow > 1 && nCurrentCol > 1)
		{
			if(m_pCheckerBoard[nCurrentRow - 1][nCurrentCol - 1] != NULL &&		// �밢�� ���� ���� ���� �ְ�
				(m_pCheckerBoard[nCurrentRow - 1][nCurrentCol - 1]->GetTeam() != m_nCurrentTurn) && // �� ���� ���� �������� �ƴϰ�
				m_pCheckerBoard[nCurrentRow - 2][nCurrentCol - 2] == NULL)		// �� �ʸӷ� ���� ������
				return TRUE;
		}

		// ������ ������ �������� �˻�
		if(nCurrentRow > 1 && nCurrentCol < 6)
		{
			if(m_pCheckerBoard[nCurrentRow - 1][nCurrentCol + 1] != NULL &&		// �밢�� ������ ���� ���� �ְ�
				(m_pCheckerBoard[nCurrentRow - 1][nCurrentCol + 1]->GetTeam() != m_nCurrentTurn) && // �� ���� ���� �������� �ƴϰ�
				m_pCheckerBoard[nCurrentRow - 2][nCurrentCol + 2] == NULL)		// �� �ʸӷ� ���� ������
				return TRUE;
		}

		// �°ݵ��� ���� ���¶�� ����
		if(!pCurPiece->IsPromoted())
			return FALSE;

		// �°ݵ� ���¶�� �밢�� �Ʒ��ʵ� �˻��Ѵ�.
		// �޾Ʒ� ������ �������� �˻�
		if(nCurrentRow < 6 && nCurrentCol > 1)
		{
			if(m_pCheckerBoard[nCurrentRow + 1][nCurrentCol - 1] != NULL &&		// �밢�� ���� �Ʒ��� ���� �ְ�
				(m_pCheckerBoard[nCurrentRow + 1][nCurrentCol - 1]->GetTeam() != m_nCurrentTurn) && // �� ���� ���� �������� �ƴϰ�
				m_pCheckerBoard[nCurrentRow + 2][nCurrentCol - 2] == NULL)		// �� �ʸӷ� ���� ������
				return TRUE;
		}

		// �����Ʒ� ������ �������� �˻�
		if(nCurrentRow < 6 && nCurrentCol < 6)
		{
			if(m_pCheckerBoard[nCurrentRow + 1][nCurrentCol + 1] != NULL &&		// �밢�� ������ �Ʒ��� ���� �ְ�
				(m_pCheckerBoard[nCurrentRow + 1][nCurrentCol + 1]->GetTeam() != m_nCurrentTurn) && // �� ���� ���� �������� �ƴϰ�
				m_pCheckerBoard[nCurrentRow + 2][nCurrentCol + 2] == NULL)		// �� �ʸӷ� ���� ������
				return TRUE;
		}
	}
	// �Ͼ��϶��� �ݴ�
	else
	{
		// �޾Ʒ� ������ �������� �˻�
		if(nCurrentRow < 6 && nCurrentCol > 1)
		{
			if(m_pCheckerBoard[nCurrentRow + 1][nCurrentCol - 1] != NULL &&		// �밢�� ���� �Ʒ��� ���� �ְ�
				(m_pCheckerBoard[nCurrentRow + 1][nCurrentCol - 1]->GetTeam() != m_nCurrentTurn) && // �� ���� ���� �������� �ƴϰ�
				m_pCheckerBoard[nCurrentRow + 2][nCurrentCol - 2] == NULL)		// �� �ʸӷ� ���� ������
				return TRUE;
		}

		// �����Ʒ� ������ �������� �˻�
		if(nCurrentRow < 6 && nCurrentCol < 6)
		{
			if(m_pCheckerBoard[nCurrentRow + 1][nCurrentCol + 1] != NULL &&		// �밢�� ������ �Ʒ��� ���� �ְ�
				(m_pCheckerBoard[nCurrentRow + 1][nCurrentCol + 1]->GetTeam() != m_nCurrentTurn) && // �� ���� ���� �������� �ƴϰ�
				m_pCheckerBoard[nCurrentRow + 2][nCurrentCol + 2] == NULL)		// �� �ʸӷ� ���� ������
				return TRUE;
		}

		// �°ݵ��� ���� ���¶�� ����
		if(!pCurPiece->IsPromoted())
			return FALSE;

		// �°ݵ� ���¶�� �밢�� �Ʒ��ʵ� �˻��Ѵ�.
		// ���� ������ �������� �˻�
		if(nCurrentRow > 1 && nCurrentCol > 1)
		{
			if(m_pCheckerBoard[nCurrentRow - 1][nCurrentCol - 1] != NULL &&		// �밢�� ���� ���� ���� �ְ�
				(m_pCheckerBoard[nCurrentRow - 1][nCurrentCol - 1]->GetTeam() != m_nCurrentTurn) && // �� ���� ���� �������� �ƴϰ�
				m_pCheckerBoard[nCurrentRow - 2][nCurrentCol - 2] == NULL)		// �� �ʸӷ� ���� ������
				return TRUE;
		}

		// ������ ������ �������� �˻�
		if(nCurrentRow > 1 && nCurrentCol < 6)
		{
			if(m_pCheckerBoard[nCurrentRow - 1][nCurrentCol + 1] != NULL &&		// �밢�� ������ ���� ���� �ְ�
				(m_pCheckerBoard[nCurrentRow - 1][nCurrentCol + 1]->GetTeam() != m_nCurrentTurn) && // �� ���� ���� �������� �ƴϰ�
				m_pCheckerBoard[nCurrentRow - 2][nCurrentCol + 2] == NULL)		// �� �ʸӷ� ���� ������
				return TRUE;
		}
	}

	return FALSE;
}
