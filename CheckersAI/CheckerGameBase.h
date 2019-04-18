#pragma once

#include "stdafx.h"
#include <string>
#include <map>
#include <iostream>
#include <limits>
#include <fstream>

const unsigned BOARD_SIZE = 8;

typedef unsigned BitBoard;

UINT inline RotateLeft(UINT a_nNum, UINT a_nCount)
{
	return (a_nNum << a_nCount) | (a_nNum >> (32 - a_nCount));
}
UINT inline RotateRight(UINT a_nNum, UINT a_nCount)
{
	return (a_nNum >> a_nCount) | (a_nNum << (32 - a_nCount));
}

/*
extern const BitBoard WP_INIT, BP_INIT;
extern const BitBoard ROW_2, ROW_7;
extern const BitBoard ROW_1, ROW_8;
extern const BitBoard CAN_UPLEFT, CAN_UPRIGHT;
extern const BitBoard CAN_DOWNLEFT, CAN_DOWNRIGHT;*/
const BitBoard Board[32] = {
	1u << 18, 1u << 12, 1u << 6, 1u << 0,
	1u << 19, 1u << 13, 1u << 7, 1u << 1,
	1u << 26, 1u << 20, 1u << 14, 1u << 8,
	1u << 27, 1u << 21, 1u << 15, 1u << 9,
	1u << 2, 1u << 28, 1u << 22, 1u << 16,
	1u << 3, 1u << 29, 1u << 23, 1u << 17,
	1u << 10, 1u << 4, 1u << 30, 1u << 24,
	1u << 11, 1u << 05, 1u << 31, 1u << 25 };

const INT ScoreBoard[32] = {
	4, 4, 4, 4,
	3, 3, 3, 4,
	4, 2, 2, 3,
	3, 1, 2, 4,
	4, 2, 1, 3,
	3, 2, 2, 4,
	4, 3, 3, 3,
	4, 4, 4, 4 };

const BitBoard WHITE_PIECE_INIT = Board[20] | Board[21] | Board[22] | Board[23] | Board[24] | Board[25] | Board[26]
								| Board[27] | Board[28] | Board[29] | Board[30] | Board[31];
const BitBoard RED_PIECE_INIT = Board[0] | Board[1] | Board[2] | Board[3] | Board[4] | Board[5] | Board[6] | Board[7]
								| Board[8] | Board[9] | Board[10] | Board[11]; 
const BitBoard ROW_1 = Board[0] | Board[1] | Board[2] | Board[3];
const BitBoard ROW_2 = Board[4] | Board[5] | Board[6] | Board[7];
const BitBoard ROW_7 = Board[24] | Board[25] | Board[26] | Board[27];
const BitBoard ROW_8 = Board[28] | Board[29] | Board[30] | Board[31];
const BitBoard CAN_UPLEFT = ~(Board[0] | Board[8] | Board[16] | Board[24] | ROW_8);
const BitBoard CAN_UPRIGHT = ~(Board[7] | Board[15] | Board[23] | Board[31] | ROW_8);
const BitBoard CAN_DOWNLEFT = ~(Board[0] | Board[8] | Board[16] | Board[24] | ROW_1);
const BitBoard CAN_DOWNRIGHT = ~(Board[7] | Board[15] | Board[23] | Board[31] | ROW_1);

const BitBoard BOARD_SCORE_4 = Board[0] | Board[1] | Board[2] | Board[3] | Board[7] | 
								Board[8] | Board[15] | Board[16] | Board[23] | Board[24] |
								Board[28] | Board[29] | Board[30] | Board[31];

const BitBoard BOARD_SCORE_3 = Board[4] | Board[5] | Board[6] | Board[11] | Board[12] |
								Board[19] | Board[20] | Board[25] | Board[26] | Board[27];

const BitBoard BOARD_SCORE_2 = Board[9] | Board[10] | Board[14] | Board[17] | Board[21] | Board[22];

const BitBoard BOARD_SCORE_1 = Board[13] | Board[18];

const BitBoard WHITE_HIGH_POINT = Board[4] | Board[5] | Board[6] | Board[7];
const BitBoard RED_HIGH_POINT = Board[24] | Board[25] | Board[26] | Board[27];

static std::map<BitBoard, unsigned short> bbUMap = {
		{ Board[0],0 },{ Board[1],1 },{ Board[2],2 },{ Board[3],3 },
	{ Board[4],4 },{ Board[5],5 },{ Board[6],6 },{ Board[7],7 },
	{ Board[8],8 },{ Board[9],9 },{ Board[10],10 },{ Board[11],11 },
	{ Board[12],12 },{ Board[13],13 },{ Board[14],14 },{ Board[15],15 },
	{ Board[16],16 },{ Board[17],17 },{ Board[18],18 },{ Board[19],19 },
	{ Board[20],20 },{ Board[21],21 },{ Board[22],22 },{ Board[23],23 },
	{ Board[24],24 },{ Board[25],25 },{ Board[26],26 },{ Board[27],27 },
	{ Board[28],28 },{ Board[29],29 },{ Board[30],30 },{ Board[31],31 } };



typedef struct _ST_PIECE_POS
{
	INT m_nRow;
	INT m_nCol;
	INT m_nIndex;

} ST_PIECE_POS, *PST_PIECE_POS;

typedef struct _ST_MOVE_POS
{
	USHORT	m_sSrc;
	USHORT	m_sDst;
	BOOL	m_bJump;
} ST_MOVE_POS, *PST_MOVE_POS;

class BitCalculator
{
public:
	static inline INT CountBit(BitBoard a_Board)
	{
		INT nCount = 0;
		a_Board = a_Board - ((a_Board >> 1) & 0x55555555); // reuse input as temporary
		a_Board = (a_Board & 0x33333333) + ((a_Board >> 2) & 0x33333333); // temp
		nCount = ((a_Board + (a_Board >> 4) & 0xF0F0F0F) * 0x1010101) >> 24; // count
		return nCount;
	}
	static inline BitBoard PickHighestBit(BitBoard a_Board)
	{
		a_Board |= (a_Board >> 1);
		a_Board |= (a_Board >> 2);
		a_Board |= (a_Board >> 4);
		a_Board |= (a_Board >> 8);
		a_Board |= (a_Board >> 16);
		return a_Board - (a_Board >> 1);
	}
};

// 게임의 현재 상태를 저장하는 구조체
struct GameState
{
	BOOL m_bCurrentTurnRed;
	BitBoard m_MustJumpPiece;
	BitBoard m_WhitePiece;
	BitBoard m_RedPiece;
	BitBoard m_KingPiece;

	friend VOID Swap(GameState&, GameState&);

	/* Equality test */
	//friend bool operator==(const GameState&, const GameState&);

	inline INT GetPoint()
	{
		//INT p1 = (BitCalculator::CountBit(m_RedPiece & ~m_KingPiece)) + ((BitCalculator::CountBit(m_RedPiece & m_KingPiece) << 1) + 1);
		//INT p2 = (BitCalculator::CountBit(m_WhitePiece & ~m_KingPiece)) + ((BitCalculator::CountBit(m_WhitePiece & m_KingPiece) << 1) + 1);
		INT p1 = 0;
		INT p2 = 0;

		/*
		for(INT i = 0; i < 32; i++)
		{
			if((~(m_RedPiece | m_WhitePiece) & Board[i]))
				continue;

			p1 += EvaluateValue(TRUE, i);
			p2 += EvaluateValue(FALSE, i);
		}*/

		p1 = EvaluateValue(TRUE);
		p2 = EvaluateValue(FALSE);

		p2 *= -1;

		return p1 + p2;
	}

	inline INT EvaluateValue(BOOL a_bTeamRed)
	{
		INT nEvalValue = 0;
		// exception
		
		if(a_bTeamRed)
		{
			nEvalValue += BitCalculator::CountBit(m_KingPiece & m_RedPiece & BOARD_SCORE_4) * 40;
			nEvalValue += BitCalculator::CountBit(m_KingPiece & m_RedPiece & BOARD_SCORE_3) * 30;
			nEvalValue += BitCalculator::CountBit(m_KingPiece & m_RedPiece & BOARD_SCORE_2) * 20;
			nEvalValue += BitCalculator::CountBit(m_KingPiece & m_RedPiece & BOARD_SCORE_1) * 10;
			nEvalValue += BitCalculator::CountBit(~m_KingPiece & m_RedPiece & RED_HIGH_POINT & BOARD_SCORE_4) * 28;
			nEvalValue += BitCalculator::CountBit(~m_KingPiece & m_RedPiece & RED_HIGH_POINT & BOARD_SCORE_3) * 21;
			nEvalValue += BitCalculator::CountBit(~m_KingPiece & m_RedPiece & RED_HIGH_POINT & BOARD_SCORE_2) * 14;
			nEvalValue += BitCalculator::CountBit(~m_KingPiece & m_RedPiece & RED_HIGH_POINT & BOARD_SCORE_1) * 7;
			nEvalValue += BitCalculator::CountBit(~m_KingPiece & m_RedPiece & ~RED_HIGH_POINT & BOARD_SCORE_4) * 20;
			nEvalValue += BitCalculator::CountBit(~m_KingPiece & m_RedPiece & ~RED_HIGH_POINT & BOARD_SCORE_3) * 15;
			nEvalValue += BitCalculator::CountBit(~m_KingPiece & m_RedPiece & ~RED_HIGH_POINT & BOARD_SCORE_2) * 10;
			nEvalValue += BitCalculator::CountBit(~m_KingPiece & m_RedPiece & ~RED_HIGH_POINT & BOARD_SCORE_1) * 5;
		}
		else
		{
			nEvalValue += BitCalculator::CountBit(m_KingPiece & m_WhitePiece & BOARD_SCORE_4) * 40;
			nEvalValue += BitCalculator::CountBit(m_KingPiece & m_WhitePiece & BOARD_SCORE_3) * 30;
			nEvalValue += BitCalculator::CountBit(m_KingPiece & m_WhitePiece & BOARD_SCORE_2) * 20;
			nEvalValue += BitCalculator::CountBit(m_KingPiece & m_WhitePiece & BOARD_SCORE_1) * 10;
			nEvalValue += BitCalculator::CountBit(~m_KingPiece & m_WhitePiece & WHITE_HIGH_POINT & BOARD_SCORE_4) * 28;
			nEvalValue += BitCalculator::CountBit(~m_KingPiece & m_WhitePiece & WHITE_HIGH_POINT & BOARD_SCORE_3) * 21;
			nEvalValue += BitCalculator::CountBit(~m_KingPiece & m_WhitePiece & WHITE_HIGH_POINT & BOARD_SCORE_2) * 14;
			nEvalValue += BitCalculator::CountBit(~m_KingPiece & m_WhitePiece & WHITE_HIGH_POINT & BOARD_SCORE_1) * 7;
			nEvalValue += BitCalculator::CountBit(~m_KingPiece & m_WhitePiece & ~WHITE_HIGH_POINT & BOARD_SCORE_4) * 20;
			nEvalValue += BitCalculator::CountBit(~m_KingPiece & m_WhitePiece & ~WHITE_HIGH_POINT & BOARD_SCORE_3) * 15;
			nEvalValue += BitCalculator::CountBit(~m_KingPiece & m_WhitePiece & ~WHITE_HIGH_POINT & BOARD_SCORE_2) * 10;
			nEvalValue += BitCalculator::CountBit(~m_KingPiece & m_WhitePiece & ~WHITE_HIGH_POINT & BOARD_SCORE_1) * 5;
		}
		return nEvalValue;

		/*
		if(a_bTeamRed)
		{
			if(m_KingPiece & m_RedPiece & Board[a_nPos])
				nEvalValue = 10;
			else if((a_nPos >= 24 && a_nPos <= 27) && (m_WhitePiece & Board[a_nPos]))
				nEvalValue = 7;
			else if(m_RedPiece & Board[a_nPos])
				nEvalValue = 5;
		}
		else
		{
			if(m_KingPiece & m_WhitePiece & Board[a_nPos])
				nEvalValue = 10;
			else if((a_nPos >= 4 && a_nPos <= 7) && (m_WhitePiece & Board[a_nPos]))
				nEvalValue = 7;
			else if(m_RedPiece & Board[a_nPos])
				nEvalValue = 5;
		}
		
		return nEvalValue * ScoreBoard[a_nPos];*/
	}
};


class CCheckerGameBase
{
public:
	virtual VOID InitalizeGame(BOOL m_bRedAI, BOOL m_bWhiteAI) = 0;
	virtual BOOL MovePiece(ST_PIECE_POS a_stCurPos, ST_PIECE_POS a_stNextPos) = 0;
	virtual BOOL CheckValidMovement(ST_PIECE_POS a_stCurPos, ST_PIECE_POS a_stNextPos) = 0;
};

