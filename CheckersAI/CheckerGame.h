#pragma once

/*	체커 룰 : 8x8 미국룰로, 아래가 빨간색, 위가 하얀색 말로 한다. 선은 빨간색으로 시작한다.
*			  nRow, nCol은 왼쪽 위부터 0부터 시작
*
* 상세설명
*	8x8 판을 사용하며, 빨간색이 먼저 두기 시작한다.
*	따먹을 때에도 오로지 전진만 가능하고 후진은 할 수 없다.
*	왕은 전진과 후진이 모두 가능하지만, 비숍처럼 움직일 수 없고 한 칸씩만 이동가능하다.
*	따먹을 수 있는 경우가 둘 이상 있을 때 그 중 어느 쪽이든 선택가능하다.
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

	// AI 전용
public:
	BOOL PlayAITurn();

public:
	/* 헤더에서 구현 */
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
