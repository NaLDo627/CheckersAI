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
#include "CheckerGameBase.h"
#include <vector>

#define CHECKER_PLAYER_HUMAN	10
#define CHECKER_PLAYER_AI		20

#define CHECKER_TEAM_RED		1
#define CHECKER_TEAM_WHITE		2
#define CHECKER_GAME_TIE		3
#define CHECKER_ERROR			(-1)

#define CHECKER_AI_RED			(CHECKER_TEAM_RED-1)
#define CHECKER_AI_WHITE		(CHECKER_TEAM_WHITE-1)

#define ABS(a)				(((a) >= 0)? (a) : (a) * (-1))

static CMutex s_AppMutex;

class CCheckerPlayerAI;

class CCheckerEventHandler
{
public:
	virtual void OnPlayerRedTurn() = 0;
	virtual void OnPlayerWhiteTurn() = 0;
	virtual void OnPieceMoved(ST_PIECE_POS a_stSrc, ST_PIECE_POS a_stDst) = 0;
	virtual void OnGameOver(INT a_nGameResult) = 0;
};

class CCheckerGame : public CCheckerGameBase
{
public:
	friend class CCheckerPlayerAI;

	virtual VOID InitalizeGame(BOOL m_bRedAI, BOOL m_bWhiteAI);
	virtual BOOL MovePiece(ST_PIECE_POS a_stCurPos, ST_PIECE_POS a_stNextPos);
	//virtual BOOL MovePiece(ST_MOVE_POS a_stMovePos);
	virtual BOOL CheckValidMovement(ST_PIECE_POS a_stCurPos, ST_PIECE_POS a_stNextPos);

	// Dlg 에서 요구
public:
	BOOL IsPieceExist(INT a_nRow, INT a_nCol);
	BOOL IsPiecePromoted(INT a_nRow, INT a_nCol);
	INT GetPieceTeam(INT a_nRow, INT a_nCol);

	// AI 전용
public:
	VOID SetAIDifficulty(INT a_nRedDifficulty, INT a_nWhiteDifficulty);
	BOOL PlayAITurn();

public:
	BOOL BitSideMovePiece(const ST_MOVE_POS& move);
	BOOL BitSideCheckValidMovement(const ST_MOVE_POS& move);

public:
	GameState GetState();
	VOID UpdateState();
	VOID RollbackState(const GameState& record);

public:
	/* 헤더에서 구현 */
	VOID SetEventHandler(CCheckerEventHandler* a_pEventHandler) { m_pEventHandler = a_pEventHandler; }
	INT	GetPlayerTurn() { return m_bCurrentTurnRed? CHECKER_TEAM_RED : CHECKER_TEAM_WHITE; }
	BOOL IsCurrentPlayerAI()
	{
		if(m_bCurrentTurnRed)
			return m_bRedPlayerAI;
		else
			return m_bWhitePlayerAI;
		return FALSE;
	}

public:
	CCheckerGame(BOOL m_bRedAI, BOOL m_bWhiteAI);
	virtual ~CCheckerGame();
	
private:
	VOID CheckGameResult();
	VOID ChangeTurn();
	INT ChangeRowColToIndex(INT a_nRow, INT a_nCol);
	ST_PIECE_POS ChangeIndexToRowCol(INT a_nIndex);

private:
	BOOL PieceMove(const ST_MOVE_POS& move);
	BOOL PieceJump(const ST_MOVE_POS& move);

private:
	ST_PIECE_POS	m_stLastMovedPos;
	//CCheckerPlayerAI* m_pPlayerAI[2];
	CCheckerPlayerAI* m_pPlayerAI;
	CCheckerEventHandler* m_pEventHandler;
	BOOL	m_bRedPlayerAI;
	BOOL	m_bWhitePlayerAI;

private:
	BitBoard m_WhitePiece;
	BitBoard m_RedPiece;
	BitBoard m_KingPiece;

	/* Tracks if it's P1's turn or not */
	BOOL m_bCurrentTurnRed;
	GameState m_State;
	BitBoard m_MustJumpPiece;


public:
	BOOL isLive() const
	{
		return (getMovers() || getJumpers());
	}

	VOID SetTurn(INT a_nTeam);

private:
	inline BitBoard getEmpty() const
	{
		return ~(m_WhitePiece | m_RedPiece);
	}

	inline BitBoard getJumpers() const
	{
		BitBoard empty = getEmpty();
		BitBoard Temp;
		BitBoard jumpers = 0;
		if(m_bCurrentTurnRed)
		{
			BitBoard RedKing = m_RedPiece & m_KingPiece;
			Temp = RotateRight(empty, 7) & m_WhitePiece & CAN_UPLEFT;
			jumpers |= RotateRight(Temp, 7) & m_RedPiece & CAN_UPLEFT;
			Temp = RotateRight(empty, 1) & m_WhitePiece & CAN_UPRIGHT;
			jumpers |= RotateRight(Temp, 1) & m_RedPiece & CAN_UPRIGHT;

			Temp = RotateLeft(empty, 7) & m_WhitePiece & CAN_DOWNRIGHT;
			jumpers |= RotateLeft(Temp, 7) & RedKing & CAN_DOWNRIGHT;
			Temp = RotateLeft(empty, 1) & m_WhitePiece & CAN_DOWNLEFT;
			jumpers |= RotateLeft(Temp, 1) & RedKing & CAN_DOWNLEFT;
		}
		else
		{
			BitBoard WhiteKing = m_WhitePiece & m_KingPiece;
			Temp = RotateLeft(empty, 7) & m_RedPiece & CAN_DOWNRIGHT;
			jumpers |= RotateLeft(Temp, 7) & m_WhitePiece & CAN_DOWNRIGHT;
			Temp = RotateLeft(empty, 1) & m_RedPiece & CAN_DOWNLEFT;
			jumpers |= RotateLeft(Temp, 1) & m_WhitePiece & CAN_DOWNLEFT;

			Temp = RotateRight(empty, 7) & m_RedPiece & CAN_UPLEFT;
			jumpers |= RotateRight(Temp, 7) & WhiteKing & CAN_UPLEFT;
			Temp = RotateRight(empty, 1) & m_RedPiece & CAN_UPRIGHT;
			jumpers |= RotateRight(Temp, 1) & WhiteKing & CAN_UPRIGHT;
		}

		return jumpers;
	}

	inline BitBoard getMovers() const
	{
		const BitBoard empty = getEmpty();
		BitBoard Movers;

		if(m_bCurrentTurnRed)
		{
			const BitBoard RedKing = m_RedPiece & m_KingPiece;
			Movers = RotateRight(empty, 7) & m_RedPiece & CAN_UPLEFT;
			Movers |= RotateRight(empty, 1) & m_RedPiece & CAN_UPRIGHT;
			Movers |= RotateLeft(empty, 7) & RedKing & CAN_DOWNRIGHT;
			Movers |= RotateLeft(empty, 1) & RedKing & CAN_DOWNLEFT;
		}
		else
		{
			const BitBoard WhiteKing = m_WhitePiece & m_KingPiece; // Kings
			Movers = RotateLeft(empty, 7) & m_WhitePiece & CAN_DOWNRIGHT;
			Movers |= RotateLeft(empty, 1) & m_WhitePiece & CAN_DOWNLEFT;
			Movers |= RotateRight(empty, 7) & WhiteKing & CAN_UPLEFT;
			Movers |= RotateRight(empty, 1) & WhiteKing & CAN_UPRIGHT;
		}

		return Movers;
	}

	inline BitBoard canJump(const BitBoard src, const BitBoard vict)
	{
		if(m_MustJumpPiece)
			if(!(src & m_MustJumpPiece))
				return 0u;
		BitBoard Temp;
		BitBoard SourceKing;
		BitBoard empty = getEmpty();

		if(m_bCurrentTurnRed)
		{
			SourceKing = src & m_KingPiece;

			Temp = RotateLeft(src & CAN_UPLEFT, 7) & vict;
			if(Temp)
				return RotateLeft(Temp & CAN_UPLEFT, 7) & empty;
			Temp = RotateLeft(src & CAN_UPRIGHT, 1) & vict;
			if(Temp)
				return RotateLeft(Temp & CAN_UPRIGHT, 1) & empty;

			Temp = RotateRight(SourceKing & CAN_DOWNRIGHT, 7) & vict;
			if(Temp)
				return RotateRight(Temp & CAN_DOWNRIGHT, 7) & empty;
			Temp = RotateRight(SourceKing & CAN_DOWNLEFT, 1) & vict;
			if(Temp)
				return RotateRight(Temp & CAN_DOWNLEFT, 1) & empty;
		}
		else
		{
			SourceKing = src & m_KingPiece;

			Temp = RotateRight(src & CAN_DOWNRIGHT, 7) & vict;
			if(Temp)
				return RotateRight(Temp & CAN_DOWNRIGHT, 7) & empty;
			Temp = RotateRight(src & CAN_DOWNLEFT, 1) & vict;
			if(Temp)
				return RotateRight(Temp & CAN_DOWNLEFT, 1) & empty;

			Temp = RotateLeft(SourceKing & CAN_UPLEFT, 7) & vict;
			if(Temp)
				return RotateLeft(Temp & CAN_UPLEFT, 7) & empty;
			Temp = RotateLeft(SourceKing & CAN_UPRIGHT, 1) & vict;
			if(Temp)
				return RotateLeft(Temp & CAN_UPRIGHT, 1) & empty;
		}

		return 0u;
	}
};
