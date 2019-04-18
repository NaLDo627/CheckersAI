#include "stdafx.h"
#include "CheckerGame.h"
#include "CheckerPlayerAI.h"
#include <iostream>
#include <vector>
#include <sstream>

CCheckerGame::CCheckerGame(BOOL a_bRedAI, BOOL a_bWhiteAI)
{
	m_pPlayerAI = new CCheckerPlayerAI();
	m_pPlayerAI->SetCheckerGame(this);
	
	m_RedPiece = RED_PIECE_INIT;
	m_WhitePiece = WHITE_PIECE_INIT;
	m_KingPiece = 0;
	m_MustJumpPiece = 0;
	m_State = {0, };

	InitalizeGame(a_bRedAI, a_bRedAI);
}

CCheckerGame::~CCheckerGame()
{
	if(m_pPlayerAI)
		delete m_pPlayerAI;

	m_pEventHandler = NULL;	
}

VOID CCheckerGame::InitalizeGame(BOOL a_bRedAI, BOOL a_bWhiteAI)
{
	m_RedPiece = RED_PIECE_INIT;
	m_WhitePiece = WHITE_PIECE_INIT;
	m_MustJumpPiece = 0;
	m_KingPiece = 0;
	m_bRedPlayerAI = a_bRedAI;
	m_bWhitePlayerAI = a_bWhiteAI;

	// 빨간색이 선으로 시작한다.
	m_bCurrentTurnRed = TRUE;
	if(a_bRedAI || a_bWhiteAI)
		m_pPlayerAI->InitializeAI();
}

BOOL CCheckerGame::MovePiece(ST_PIECE_POS a_stCurPos, ST_PIECE_POS a_stNextPos)
{
	INT nMoveValue = 0;
	ST_MOVE_POS stMovePos = {0,};
	BOOL bIsJump = FALSE;

	// Step 1. 3칸이상 이동인지, 점프 이동인지 검사한다.
	nMoveValue = a_stNextPos.m_nCol - a_stCurPos.m_nCol;

	// 3칸 이상 갈 수 없음
	if(ABS(nMoveValue) > 2)
		return FALSE;

	if(ABS(nMoveValue) == 2)
	{
		a_stNextPos.m_nRow = (a_stNextPos.m_nRow + a_stCurPos.m_nRow) / 2;
		a_stNextPos.m_nCol = (a_stNextPos.m_nCol + a_stCurPos.m_nCol) / 2;
		bIsJump = TRUE;
	}

	stMovePos.m_sSrc = ChangeRowColToIndex(a_stCurPos.m_nRow, a_stCurPos.m_nCol);
	stMovePos.m_sDst = ChangeRowColToIndex(a_stNextPos.m_nRow, a_stNextPos.m_nCol);
	stMovePos.m_bJump = bIsJump;
	
	if(!BitSideMovePiece(stMovePos))
		return FALSE;

	return TRUE;
}

BOOL CCheckerGame::CheckValidMovement(ST_PIECE_POS a_stCurPos, ST_PIECE_POS a_stNextPos)
{
	INT nMoveValue = 0;
	ST_MOVE_POS stMovePos = { 0, };
	BOOL bIsJump = FALSE;

	// Step 1. 3칸이상 이동인지, 점프 이동인지 검사한다.
	nMoveValue = a_stNextPos.m_nCol - a_stCurPos.m_nCol;

	// 3칸 이상 갈 수 없음
	if(ABS(nMoveValue) > 2)
		return FALSE;

	if(ABS(nMoveValue) == 2)
	{
		a_stNextPos.m_nRow = (a_stNextPos.m_nRow + a_stCurPos.m_nRow) / 2;
		a_stNextPos.m_nCol = (a_stNextPos.m_nCol + a_stCurPos.m_nCol) / 2;
		bIsJump = TRUE;
	}

	stMovePos.m_sSrc = ChangeRowColToIndex(a_stCurPos.m_nRow, a_stCurPos.m_nCol);
	stMovePos.m_sDst = ChangeRowColToIndex(a_stNextPos.m_nRow, a_stNextPos.m_nCol);
	stMovePos.m_bJump = bIsJump;

	return BitSideCheckValidMovement(stMovePos);
}

BOOL CCheckerGame::BitSideMovePiece(const ST_MOVE_POS& move)
{
	BOOL bResult = FALSE;

	ST_PIECE_POS stSrcPos = ChangeIndexToRowCol(move.m_sSrc);
	ST_PIECE_POS stDstPos = ChangeIndexToRowCol(move.m_sDst);

	if(move.m_bJump)
		bResult = PieceJump(move);
	else
		bResult = PieceMove(move);

	if(!bResult)
		return FALSE;

	m_stLastMovedPos = stDstPos;

	if(m_pEventHandler)
	{
		m_pEventHandler->OnPieceMoved(stSrcPos, stDstPos);

		if(m_bCurrentTurnRed)
			m_pEventHandler->OnPlayerRedTurn();
		else
			m_pEventHandler->OnPlayerWhiteTurn();
	}
	
	if(!isLive())
		CheckGameResult();

	return TRUE;
}

BOOL CCheckerGame::IsPieceExist(INT a_nRow, INT a_nCol)
{
	ST_MOVE_POS stMovePos = { 0, };
	INT nIndex = ChangeRowColToIndex(a_nRow, a_nCol);

	if(!((m_RedPiece & Board[nIndex]) | (m_WhitePiece & Board[nIndex])))
		return FALSE;

	return TRUE;
}

INT CCheckerGame::GetPieceTeam(INT a_nRow, INT a_nCol)
{
	ST_MOVE_POS stMovePos = { 0, };
	INT nIndex = ChangeRowColToIndex(a_nRow, a_nCol);

	if(m_RedPiece & Board[nIndex])
		return CHECKER_TEAM_RED;

	if(m_WhitePiece & Board[nIndex])
		return CHECKER_TEAM_WHITE;

	return -1;
}

BOOL CCheckerGame::IsPiecePromoted(INT a_nRow, INT a_nCol)
{
	ST_MOVE_POS stMovePos = { 0, };
	INT nIndex = ChangeRowColToIndex(a_nRow, a_nCol);

	if(!(m_KingPiece & Board[nIndex]))
		return FALSE;

	return TRUE;
}

VOID CCheckerGame::CheckGameResult()
{
	INT nGameResult = 0;
	INT nTeamRedCount = BitCalculator::CountBit(m_RedPiece);
	INT nTeamWhiteCount = BitCalculator::CountBit(m_WhitePiece);
	BOOL bRedMoveable = FALSE;
	BOOL bWhiteMoveable = FALSE;

	// 둘 다 0이면 뭔가 이상함, 에러 리턴
	if((nTeamRedCount + nTeamWhiteCount) == 0)
	{
		if(m_pEventHandler)
			m_pEventHandler->OnGameOver(CHECKER_ERROR);
	}

	BitBoard empty = ~(m_WhitePiece | m_RedPiece);
	BitBoard Temp;
	BitBoard jumpers = 0;
	BitBoard Movers = 0;

	// Red 중 움직일 수 있는것이 있는지 체크
	BitBoard RedKing = m_RedPiece & m_KingPiece;
	Temp = RotateRight(empty, 7) & m_WhitePiece & CAN_UPLEFT;
	jumpers |= RotateRight(Temp, 7) & m_RedPiece & CAN_UPLEFT;
	Temp = RotateRight(empty, 1) & m_WhitePiece & CAN_UPRIGHT;
	jumpers |= RotateRight(Temp, 1) & m_RedPiece & CAN_UPRIGHT;

	Temp = RotateLeft(empty, 7) & m_WhitePiece & CAN_DOWNRIGHT;
	jumpers |= RotateLeft(Temp, 7) & RedKing & CAN_DOWNRIGHT;
	Temp = RotateLeft(empty, 1) & m_WhitePiece & CAN_DOWNLEFT;
	jumpers |= RotateLeft(Temp, 1) & RedKing & CAN_DOWNLEFT;

	Movers = RotateRight(empty, 7) & m_RedPiece & CAN_UPLEFT;
	Movers |= RotateRight(empty, 1) & m_RedPiece & CAN_UPRIGHT;
	Movers |= RotateLeft(empty, 7) & RedKing & CAN_DOWNRIGHT;
	Movers |= RotateLeft(empty, 1) & RedKing & CAN_DOWNLEFT;

	bRedMoveable = (Movers | jumpers) ? TRUE : FALSE;

	// White 중 움직일 수 있는 것이 있는지 체크
	BitBoard WhiteKing = m_WhitePiece & m_KingPiece;
	Temp = RotateLeft(empty, 7) & m_RedPiece & CAN_DOWNRIGHT;
	jumpers |= RotateLeft(Temp, 7) & m_WhitePiece & CAN_DOWNRIGHT;
	Temp = RotateLeft(empty, 1) & m_RedPiece & CAN_DOWNLEFT;
	jumpers |= RotateLeft(Temp, 1) & m_WhitePiece & CAN_DOWNLEFT;

	Temp = RotateRight(empty, 7) & m_RedPiece & CAN_UPLEFT;
	jumpers |= RotateRight(Temp, 7) & WhiteKing & CAN_UPLEFT;
	Temp = RotateRight(empty, 1) & m_RedPiece & CAN_UPRIGHT;
	jumpers |= RotateRight(Temp, 1) & WhiteKing & CAN_UPRIGHT;

	Movers = RotateLeft(empty, 7) & m_WhitePiece & CAN_DOWNRIGHT;
	Movers |= RotateLeft(empty, 1) & m_WhitePiece & CAN_DOWNLEFT;
	Movers |= RotateRight(empty, 7) & WhiteKing & CAN_UPLEFT;
	Movers |= RotateRight(empty, 1) & WhiteKing & CAN_UPRIGHT;

	bWhiteMoveable = (Movers | jumpers) ? TRUE : FALSE;

	// 둘다 움직일 수 없다면 비김
	if(!bRedMoveable && !bWhiteMoveable)
		nGameResult = CHECKER_GAME_TIE;

	// 빨강이 없거나, 빨강이 움직일 수 없고 빨강 턴이라면 하양 승
	else if((nTeamRedCount == 0) || (!bRedMoveable && m_bCurrentTurnRed))
		nGameResult = CHECKER_TEAM_WHITE;

	// 하양이 없거나, 하양이 움직일 수 없고 하양 턴이라면 빨강승
	else if((nTeamWhiteCount == 0) || (!bWhiteMoveable && !m_bCurrentTurnRed))
		nGameResult = CHECKER_TEAM_RED;

	if(nGameResult != 0 && m_pEventHandler)
		m_pEventHandler->OnGameOver(nGameResult);
}

VOID CCheckerGame::SetAIDifficulty(INT a_nRedDifficulty, INT a_nWhiteDifficulty)
{
	if(m_pPlayerAI)
		m_pPlayerAI->InitializeAI(a_nRedDifficulty, a_nWhiteDifficulty);
}

BOOL CCheckerGame::PlayAITurn()
{
	if(m_bCurrentTurnRed && !m_bRedPlayerAI)
		return FALSE;

	if(!m_bCurrentTurnRed && !m_bWhitePlayerAI)
		return FALSE;

	if(!(m_pPlayerAI->MakeMove()))
		return FALSE;

	return TRUE;
}

VOID CCheckerGame::ChangeTurn()
{
	m_bCurrentTurnRed = !m_bCurrentTurnRed;
}

INT CCheckerGame::ChangeRowColToIndex(INT a_nRow, INT a_nCol)
{
	switch(a_nRow)
	{
	case 0:
		switch(a_nCol)
		{
		case 1:	return 28;
		case 3:	return 29;
		case 5:	return 30;
		case 7:	return 31;
		default: return -1;
		}
	case 1:
		switch(a_nCol)
		{
		case 0:	return 24;
		case 2:	return 25;
		case 4:	return 26;
		case 6:	return 27;
		default: return -1;
		}
	case 2:
		switch(a_nCol)
		{
		case 1:	return 20;
		case 3:	return 21;
		case 5:	return 22;
		case 7:	return 23;
		default: return -1;
		}
	case 3:
		switch(a_nCol)
		{
		case 0:	return 16;
		case 2:	return 17;
		case 4:	return 18;
		case 6:	return 19;
		default: return -1;
		}
	case 4:
		switch(a_nCol)
		{
		case 1:	return 12;
		case 3:	return 13;
		case 5: return 14;
		case 7: return 15;
		default: return -1;
		}
	case 5:
		switch(a_nCol)
		{
		case 0: return 8;
		case 2: return 9;
		case 4: return 10; 
		case 6: return 11;
		default: return -1;
		}
	case 6:
		switch(a_nCol)
		{
		case 1: return 4;
		case 3: return 5;
		case 5: return 6;
		case 7: return 7;
		default: return -1;
		}
	case 7:
		switch(a_nCol)
		{
		case 0: return 0;
		case 2: return 1;
		case 4: return 2;
		case 6: return 3;
		default: return -1;
		}
	default:
		return -1;
	}
	return -1;
}

ST_PIECE_POS CCheckerGame::ChangeIndexToRowCol(INT a_nIndex)
{
	ST_PIECE_POS stRetPos = {0,};

	stRetPos.m_nRow = 7 - (a_nIndex / 4);
	stRetPos.m_nCol = (a_nIndex % 4) * 2;
	if(stRetPos.m_nRow % 2 == 0)
		stRetPos.m_nCol += 1;

	return stRetPos;
}

BOOL CCheckerGame::BitSideCheckValidMovement(const ST_MOVE_POS& move)
{
	if(move.m_sSrc > 31 || move.m_sDst > 31)
		return FALSE;

	const BitBoard SrcBit = (m_bCurrentTurnRed ? m_RedPiece & Board[move.m_sSrc] : m_WhitePiece & Board[move.m_sSrc]);
	if(!SrcBit)
		return FALSE;

	if(!move.m_bJump)
	{
		if(m_MustJumpPiece)
			return FALSE;

		const BitBoard empty = ~(m_WhitePiece | m_RedPiece);
		BitBoard MoveValue = 0;
		BitBoard SourceKing;
		if(m_bCurrentTurnRed)
		{
			SourceKing = SrcBit & m_KingPiece;
			MoveValue = empty & RotateLeft(SrcBit & CAN_UPLEFT, 7);
			MoveValue |= empty & RotateLeft(SrcBit & CAN_UPRIGHT, 1);
			MoveValue |= empty & RotateRight(SourceKing & CAN_DOWNRIGHT, 7);
			MoveValue |= empty & RotateRight(SourceKing & CAN_DOWNLEFT, 1);
		}
		else
		{
			SourceKing = SrcBit & m_KingPiece;
			MoveValue = empty & RotateRight(SrcBit & CAN_DOWNRIGHT, 7);
			MoveValue |= empty & RotateRight(SrcBit & CAN_DOWNLEFT, 1);
			MoveValue |= empty & RotateLeft(SourceKing & CAN_UPLEFT, 7);
			MoveValue |= empty & RotateLeft(SourceKing & CAN_UPRIGHT, 1);
		}

		const BitBoard DstBit = Board[move.m_sDst];
		if(!(MoveValue & DstBit))
			return FALSE;

		return TRUE;
	}

	const BitBoard VictBit = (m_bCurrentTurnRed ? m_WhitePiece & Board[move.m_sDst] : m_RedPiece & Board[move.m_sDst]);
	if(!VictBit)
		return FALSE;

	if(m_MustJumpPiece)
		if(!(SrcBit & m_MustJumpPiece))
			return FALSE;

	BitBoard jumpers = getJumpers();
	if(!(jumpers & SrcBit))
		return FALSE;

	BitBoard nextLoc = canJump(SrcBit, VictBit);
	if(!nextLoc)
		return FALSE;

	return TRUE;
}

void CCheckerGame::RollbackState(const GameState& save)
{
	using namespace std;

	m_State = save;
	m_WhitePiece = save.m_WhitePiece;
	m_RedPiece = save.m_RedPiece;
	m_KingPiece = save.m_KingPiece;
	m_bCurrentTurnRed = save.m_bCurrentTurnRed;
	m_MustJumpPiece = save.m_MustJumpPiece;
}

inline void CCheckerGame::UpdateState()
{
	m_State.m_WhitePiece = m_WhitePiece;
	m_State.m_RedPiece = m_RedPiece;
	m_State.m_KingPiece = m_KingPiece;
	m_State.m_bCurrentTurnRed = m_bCurrentTurnRed;
	m_State.m_MustJumpPiece = m_MustJumpPiece;
}

GameState CCheckerGame::GetState()
{
	UpdateState();
	return m_State;
}

/* Piece movement */
BOOL CCheckerGame::PieceMove(const ST_MOVE_POS& move)
{
	if(move.m_sSrc > 31 || move.m_sDst > 31)
		return FALSE;

	if(m_MustJumpPiece)
		return FALSE;

	const BitBoard src = (m_bCurrentTurnRed ? m_RedPiece & Board[move.m_sSrc] : m_WhitePiece & Board[move.m_sSrc]);

	if(!src)
		return FALSE;

	const BitBoard empty = ~(m_WhitePiece | m_RedPiece);

	BitBoard valMoves = 0;
	BitBoard SrcKing;
	if(m_bCurrentTurnRed)
	{
		SrcKing = src & m_KingPiece;
		valMoves = empty & RotateLeft(src & CAN_UPLEFT, 7);
		valMoves |= empty & RotateLeft(src & CAN_UPRIGHT, 1);
		valMoves |= empty & RotateRight(SrcKing & CAN_DOWNRIGHT, 7);
		valMoves |= empty & RotateRight(SrcKing & CAN_DOWNLEFT, 1);
	}
	else
	{
		SrcKing = src & m_KingPiece;
		valMoves = empty & RotateRight(src & CAN_DOWNRIGHT, 7);
		valMoves |= empty & RotateRight(src & CAN_DOWNLEFT, 1);
		valMoves |= empty & RotateLeft(SrcKing & CAN_UPLEFT, 7);
		valMoves |= empty & RotateLeft(SrcKing & CAN_UPRIGHT, 1);
	}

	const BitBoard dst = Board[move.m_sDst];

	if(!(valMoves & dst))
		return FALSE;

	if(m_bCurrentTurnRed)
	{
		m_RedPiece ^= src;
		m_RedPiece |= dst;
	}
	else
	{
		m_WhitePiece ^= src;
		m_WhitePiece |= dst;
	}

	if(SrcKing)
	{
		m_KingPiece ^= src;
		m_KingPiece ^= dst;
	}

	if((dst & ROW_1) || (dst & ROW_8))
		m_KingPiece |= dst;

	m_bCurrentTurnRed = !m_bCurrentTurnRed;
	// 턴 변경 후 MustJump가 발생하는지 확인
	BitBoard NextTurn = m_bCurrentTurnRed? m_RedPiece : m_WhitePiece;

	if(NextTurn & getJumpers())
		m_MustJumpPiece = (NextTurn & getJumpers());
	else
		m_MustJumpPiece = 0;

	return TRUE;
}

BOOL CCheckerGame::PieceJump(const ST_MOVE_POS& move)
{
	if(move.m_sSrc > 31 || move.m_sDst > 31)
		return FALSE;

	const BitBoard src = (m_bCurrentTurnRed ? m_RedPiece & Board[move.m_sSrc] : m_WhitePiece & Board[move.m_sSrc]);
	const BitBoard vict = (m_bCurrentTurnRed ? m_WhitePiece & Board[move.m_sDst] : m_RedPiece & Board[move.m_sDst]);

	if(m_MustJumpPiece)
		if(!(src & m_MustJumpPiece))
			return FALSE;

	if(!vict || !src)
		return FALSE;

	BitBoard jumpers = getJumpers();
	if(!(jumpers & src))
		return FALSE;

	BitBoard nextLoc = canJump(src, vict);
	if(!nextLoc)
		return FALSE;

	if(m_bCurrentTurnRed)
	{
		m_RedPiece ^= src;
		m_WhitePiece ^= vict;
		m_RedPiece ^= nextLoc;

	}
	else
	{
		m_WhitePiece ^= src;
		m_RedPiece ^= vict;
		m_WhitePiece ^= nextLoc;
	}

	if(m_KingPiece & src)
	{
		m_KingPiece ^= src;
		m_KingPiece ^= nextLoc;
	}
	else if((nextLoc & ROW_8) || (nextLoc & ROW_1))
		m_KingPiece ^= nextLoc;

	if(m_KingPiece & vict)
	{
		m_KingPiece ^= vict;
	}

	if(nextLoc & getJumpers())
	{
		m_MustJumpPiece = nextLoc;
	}
	else
	{
		m_bCurrentTurnRed = !m_bCurrentTurnRed;
		// 턴 변경 후 MustJump가 발생하는지 확인
		BitBoard NextTurn = m_bCurrentTurnRed ? m_RedPiece : m_WhitePiece;

		if(NextTurn & getJumpers())
			m_MustJumpPiece = (NextTurn & getJumpers());
		else
			m_MustJumpPiece = 0;
	}

	return TRUE;
}

VOID CCheckerGame::SetTurn(INT a_nTeam)
{
	if(a_nTeam == CHECKER_TEAM_RED)
		m_bCurrentTurnRed = TRUE;
	else
		m_bCurrentTurnRed = FALSE;
}

