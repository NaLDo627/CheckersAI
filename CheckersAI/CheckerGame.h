#pragma once

/*	üĿ �� : 8x8 �̱����, �Ʒ��� ������, ���� �Ͼ�� ���� �Ѵ�. ���� ���������� �����Ѵ�.
*			  nRow, nCol�� ���� ������ 0���� ����
*
* �󼼼���
*	8x8 ���� ����ϸ�, �������� ���� �α� �����Ѵ�.
*	������ ������ ������ ������ �����ϰ� ������ �� �� ����.
*	���� ������ ������ ��� ����������, ���ó�� ������ �� ���� �� ĭ���� �̵������ϴ�.
*	������ �� �ִ� ��찡 �� �̻� ���� �� �� �� ��� ���̵� ���ð����ϴ�.
*
*/

#include "stdafx.h"
#include "resource.h"

#define CHECKER_PLAYER_HUMAN	10
#define CHECKER_PLAYER_AI		20

#define CHECKER_TEAM_RED		1
#define CHECKER_TEAM_WHITE		2
#define CHECKER_GAME_TIE		3
#define CHECKER_ERROR			(-1)

#define CHECKER_AI_RED			(CHECKER_TEAM_RED-1)
#define CHECKER_AI_WHITE		(CHECKER_TEAM_WHITE-1)

#define ABS(a)				(((a) >= 0)? (a) : (a) * (-1))

typedef struct _ST_PIECE_POS
{
	INT m_nRow;
	INT m_nCol;
} ST_PIECE_POS, *PST_PIECE_POS;

class CCheckerPlayerAI;

class CCheckerPiece
{
public:
	inline VOID PromoteThis() { m_bPromoted = TRUE; }
	inline VOID SetPosition(INT a_nRow, INT a_nCol)
	{
		m_stPos.m_nRow = a_nRow;
		m_stPos.m_nCol = a_nCol;
	}
	inline VOID SetTeam(INT a_nTeamColor) { m_nTeam = a_nTeamColor; }
	inline INT GetPosX() { return m_stPos.m_nRow; }
	inline INT GetPosY() { return m_stPos.m_nCol; }
	inline INT GetTeam() { return m_nTeam; }
	inline BOOL IsPromoted() { return m_bPromoted; }
	inline ST_PIECE_POS GetPosition() { return m_stPos; }

public:
	CCheckerPiece()
	{
		m_nTeam = 0;
		m_stPos.m_nRow = 0;
		m_stPos.m_nCol = 0;
		m_bPromoted = FALSE;
	};
	virtual ~CCheckerPiece() { }

private:
	INT		m_nTeam;
	BOOL	m_bPromoted;
	ST_PIECE_POS	m_stPos;
};

class CCheckerGame
{
public:
	VOID InitalizeGame(BOOL m_bRedAI, BOOL m_bWhiteAI);
	BOOL MovePiece(CCheckerPiece* a_pCheckerPiece, INT a_nRow, INT a_nCol);
	INT GetGameResult();

	// AI ����
public:
	BOOL PlayAITurn();

public:
	/* ������� ���� */
	CCheckerPiece* GetPieceByPos(INT a_nRow, INT a_nCol) { return m_pCheckerBoard[a_nRow][a_nCol]; }
	INT	GetPlayerTurn() { return m_nCurrentTurn; }
	BOOL IsCurrentPlayerAI()
	{
		if(!m_pPlayerAI[m_nCurrentTurn - 1])
			return FALSE;
		return TRUE;
	}

public:
	CCheckerGame(BOOL m_bRedAI, BOOL m_bWhiteAI);
	virtual ~CCheckerGame();
	
private:
	VOID ChangeTurn();
	BOOL CheckValidMovement(ST_PIECE_POS a_stCurPos, ST_PIECE_POS a_stNextPos);
	BOOL CheckPieceTakenAvailable(ST_PIECE_POS a_stCurPos);

private:
	CCheckerPiece* m_pCheckerBoard[8][8];
	INT		m_nCurrentTurn;
	BOOL	m_bPieceTakenOccured;
	BOOL	m_bBonusTurn;
	ST_PIECE_POS	m_stBonusTurnPos;
	CCheckerPlayerAI* m_pPlayerAI[2];
};
