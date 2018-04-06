#include <iostream>
#include <vector>
#include <sstream>

#include "stdafx.h"
#include "CheckerGame.h"
#include "Game.hpp"

Game::Game(const bool debug, const bool interact) :
				mWP(Bit::Masks::WP_INIT),
				mBP(Bit::Masks::BP_INIT),
				mK(0),
				mTurn(true),
				mDebug(debug),
				mSave(),
				mInteract(interact),
				mMustJump(0)
{
}

Game::Game(const Save& save, const bool debug, const bool interact) :
				mDebug(debug),
				mInteract(interact)
{
	restoreToSave(save);
}

Game::~Game()
{
}

void Game::restoreToSave(const Save& save)
{
	using namespace std;

	mSave = save;
	mWP = save.WP;
	mBP = save.BP;
	mK = save.K;
	mTurn = save.turn;
	mMustJump = save.mustJump;
}

inline void Game::updateSave()
{
	mSave.WP = mWP;
	mSave.BP = mBP;
	mSave.K = mK;
	mSave.turn = mTurn;
	mSave.mustJump = mMustJump;
}

Save Game::getSave()
{
	updateSave();
	return mSave;
}

std::vector<Cell> Game::toArr() const
{
	using Bit::Masks::S;

	std::vector<Cell> b(64);

	for (unsigned i = 0; i < 32; i++) {
		unsigned odd = (i / 4) & 1;
		if (mWP & S[i])
			b[i * 2 + odd] = P_W;
	}

	for (unsigned i = 0; i < 32; i++) {
		unsigned odd = (i / 4) & 1;
		if (mBP & S[i])
			b[i * 2 + odd] = P_B;
	}

	BitBoard WK = mWP & mK;
	if (WK) {
		for (unsigned i = 0; i < 32; i++) {
			unsigned odd = (i / 4) & 1;
			if (WK & S[i])
				b[i * 2 + odd] = K_W;
		}
	}

	BitBoard BK = mBP & mK;
	if (BK) {
		for (unsigned i = 0; i < 32; i++) {
			unsigned odd = (i / 4) & 1;
			if (BK & S[i])
				b[i * 2 + odd] = K_B;
		}
	}

	return b;
}

//inline BitBoard Game::getJumpers() const


//inline BitBoard Game::getMovers() const


/* Piece movement */
MoveCode Game::makeMove(const Move& move)
{
	using namespace Bit::Masks;
	using Bit::rol;
	using Bit::ror;
	using Bit::Masks::S;

	if (move.src > 31 || move.dst > 31)
		return ILLEGAL_MOVE;

	if (mMustJump)
		return WRONG_PIECE;

	const BitBoard src = (mTurn ? mBP & S[move.src] : mWP & S[move.src]);

	if (!src)
		return VOID_PIECE;

	const BitBoard empty = ~(mWP | mBP);

	BitBoard valMoves = 0;
	BB SK;
	if (mTurn) {
		SK = src & mK;
		valMoves = empty & rol(src & CAN_UPLEFT, 7);
		valMoves |= empty & rol(src & CAN_UPRIGHT, 1);
		valMoves |= empty & ror(SK & CAN_DOWNRIGHT, 7);
		valMoves |= empty & ror(SK & CAN_DOWNLEFT, 1);
	} else {
		SK = src & mK;
		valMoves = empty & ror(src & CAN_DOWNRIGHT, 7);
		valMoves |= empty & ror(src & CAN_DOWNLEFT, 1);
		valMoves |= empty & rol(SK & CAN_UPLEFT, 7);
		valMoves |= empty & rol(SK & CAN_UPRIGHT, 1);
	}

	const BitBoard dst = S[move.dst];

	if (!(valMoves & dst))
		return ILLEGAL_MOVE;

	if (mTurn) {
		mBP ^= src;
		mBP |= dst;
	} else {
		mWP ^= src;
		mWP |= dst;
	}

	if (SK) {
		mK ^= src;
		mK ^= dst;
	}

	if ((dst & ROW_1) || (dst & ROW_8))
		mK |= dst;

	mMustJump = 0;
	mTurn = !mTurn;
	return SUCCESS;
}

BB Game::canJump(const BB src, const BB vict)
{
	using Bit::rol;
	using Bit::ror;
	using namespace Bit::Masks;

	if (mMustJump)
		if (src != mMustJump)
			return 0u;
	BB Temp;
	BB SK;
	BB empty = ~(mWP | mBP);

	if (mTurn) {
		SK = src & mK;

		Temp = rol(src & CAN_UPLEFT, 7) & vict;
		if (Temp)
			return rol(Temp & CAN_UPLEFT, 7) & empty;
		Temp = rol(src & CAN_UPRIGHT, 1) & vict;
		if (Temp)
			return rol(Temp & CAN_UPRIGHT, 1) & empty;

		Temp = ror(SK & CAN_DOWNRIGHT, 7) & vict;
		if (Temp)
			return ror(Temp & CAN_DOWNRIGHT, 7) & empty;
		Temp = ror(SK & CAN_DOWNLEFT, 1) & vict;
		if (Temp)
			return ror(Temp & CAN_DOWNLEFT, 1) & empty;
	} else {
		SK = src & mK;

		Temp = ror(src & CAN_DOWNRIGHT, 7) & vict;
		if (Temp)
			return ror(Temp & CAN_DOWNRIGHT, 7) & empty;
		Temp = ror(src & CAN_DOWNLEFT, 1) & vict;
		if (Temp)
			return ror(Temp & CAN_DOWNLEFT, 1) & empty;

		Temp = rol(SK & CAN_UPLEFT, 7) & vict;
		if (Temp)
			return rol(Temp & CAN_UPLEFT, 7) & empty;
		Temp = rol(SK & CAN_UPRIGHT, 1) & vict;
		if (Temp)
			return rol(Temp & CAN_UPRIGHT, 1) & empty;
	}

	return 0u;
}

MoveCode Game::jump(const Move& move)
{
	using namespace Bit::Masks;
	using Bit::rol;
	using Bit::ror;
	if (move.src > 31 || move.dst > 31)
		return ILLEGAL_MOVE;

	const BitBoard src = (mTurn ? mBP & S[move.src] : mWP & S[move.src]);
	const BitBoard vict = (mTurn ? mWP & S[move.dst] : mBP & S[move.dst]);

	if (mMustJump)
		if (src != mMustJump)
			return WRONG_PIECE;

	if (!vict || !src)
		return VOID_PIECE;

	BitBoard jumpers = getJumpers();
	if (!(jumpers & src))
		return ILLEGAL_MOVE;

	BB nextLoc = canJump(src, vict);
	if (!nextLoc)
		return ILLEGAL_MOVE;

	if (mTurn) {
		mBP ^= src;
		mWP ^= vict;
		mBP ^= nextLoc;

	} else {
		mWP ^= src;
		mBP ^= vict;
		mWP ^= nextLoc;
	}

	if (mK & src) {
		mK ^= src;
		mK ^= nextLoc;
	} else if ((nextLoc & ROW_8) || (nextLoc & ROW_1))
		mK ^= nextLoc;

	if (mK & vict) {
		mK ^= vict;
	}

	if (nextLoc & getJumpers()) {
		mMustJump = nextLoc;
	} else {
		mMustJump = 0;
		mTurn = !mTurn;
	}

	return SUCCESS;
}

int Game::grade() const
{
	int p1 = Bit::bitCount(mBP & ~mK) + 2 * Bit::bitCount(mBP & mK);
	int p2 = Bit::bitCount(mWP & ~mK) + 2 * Bit::bitCount(mWP & mK);
	p2 *= -1;

	return p1 + p2;
}

BOOL Game::TransformClassToBit(CCheckerGame* a_pCheckerGame)
{
	using namespace Bit::Masks;

	CCheckerPiece*	pPiece = NULL;
	BitBoard		nTmpWP = 0;
	BitBoard		nTmpBP = 0;
	BitBoard		nTmpK = 0;
	INT		nPlayerTurn = 0;

	for(INT r = 0; r < 8; r++)
		for(INT c = 0; c < 8; c++)
		{
			pPiece = a_pCheckerGame->GetPieceByPos(r, c);
			if(!pPiece)
				continue;

			nPlayerTurn = pPiece->GetTeam();

			switch(r)
			{
			case 0:
				switch(c)
				{
				case 1:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[28];
					else
						nTmpWP |= S[28];
					nTmpK |= (pPiece->IsPromoted()) ? S[28] : 0;
					break;
				case 3:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[29];
					else
						nTmpWP |= S[29];
					nTmpK |= (pPiece->IsPromoted()) ? S[29] : 0;
					break;
				case 5:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[30];
					else
						nTmpWP |= S[30];
					nTmpK |= (pPiece->IsPromoted()) ? S[30] : 0;
					break;
				case 7:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[31];
					else
						nTmpWP |= S[31];
					nTmpK |= (pPiece->IsPromoted()) ? S[31] : 0;
					break;
				default:
					return FALSE;
				}
				break;
			case 1:
				switch(c)
				{
				case 0:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[24];
					else
						nTmpWP |= S[24];
					nTmpK |= (pPiece->IsPromoted()) ? S[24] : 0;
					break;
				case 2:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[25];
					else
						nTmpWP |= S[25];
					nTmpK |= (pPiece->IsPromoted()) ? S[25] : 0;
					break;
				case 4:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[26];
					else
						nTmpWP |= S[26];
					nTmpK |= (pPiece->IsPromoted()) ? S[26] : 0;
					break;
				case 6:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[27];
					else
						nTmpWP |= S[27];
					nTmpK |= (pPiece->IsPromoted()) ? S[27] : 0;
					break;
				default:
					return FALSE;
				}
				break;
			case 2:
				switch(c)
				{
				case 1:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[20];
					else
						nTmpWP |= S[20];
					nTmpK |= (pPiece->IsPromoted()) ? S[20] : 0;
					break;
				case 3:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[21];
					else
						nTmpWP |= S[21];
					nTmpK |= (pPiece->IsPromoted()) ? S[21] : 0;
					break;
				case 5:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[22];
					else
						nTmpWP |= S[22];
					nTmpK |= (pPiece->IsPromoted()) ? S[22] : 0;
					break;
				case 7:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[23];
					else
						nTmpWP |= S[23];
					nTmpK |= (pPiece->IsPromoted()) ? S[23] : 0;
					break;
				default:
					return FALSE;
				}
				break;
			case 3:
				switch(c)
				{
				case 0:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[16];
					else
						nTmpWP |= S[16];
					nTmpK |= (pPiece->IsPromoted()) ? S[16] : 0;
					break;
				case 2:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[17];
					else
						nTmpWP |= S[17];
					nTmpK |= (pPiece->IsPromoted()) ? S[17] : 0;
					break;
				case 4:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[18];
					else
						nTmpWP |= S[18];
					nTmpK |= (pPiece->IsPromoted()) ? S[18] : 0;
					break;
				case 6:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[19];
					else
						nTmpWP |= S[19];
					nTmpK |= (pPiece->IsPromoted()) ? S[19] : 0;
					break;
				default:
					return FALSE;
				}
				break;
			case 4:
				switch(c)
				{
				case 1:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[12];
					else
						nTmpWP |= S[12];
					nTmpK |= (pPiece->IsPromoted()) ? S[12] : 0;
					break;
				case 3:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[13];
					else
						nTmpWP |= S[13];
					nTmpK |= (pPiece->IsPromoted()) ? S[13] : 0;
					break;
				case 5:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[14];
					else
						nTmpWP |= S[14];
					nTmpK |= (pPiece->IsPromoted()) ? S[14] : 0;
					break;
				case 7:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[15];
					else
						nTmpWP |= S[15];
					nTmpK |= (pPiece->IsPromoted()) ? S[15] : 0;
					break;
				default:
					return FALSE;
				}
				break;
			case 5:
				switch(c)
				{
				case 0:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[8];
					else
						nTmpWP |= S[8];
					nTmpK |= (pPiece->IsPromoted()) ? S[8] : 0;
					break;
				case 2:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[9];
					else
						nTmpWP |= S[9];
					nTmpK |= (pPiece->IsPromoted()) ? S[9] : 0;
					break;
				case 4:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[10];
					else
						nTmpWP |= S[10];
					nTmpK |= (pPiece->IsPromoted()) ? S[10] : 0;
					break;
				case 6:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[11];
					else
						nTmpWP |= S[11];
					nTmpK |= (pPiece->IsPromoted()) ? S[11] : 0;
					break;
				default:
					return FALSE;
				}
				break;
			case 6:
				switch(c)
				{
				case 1:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[4];
					else
						nTmpWP |= S[4];
					nTmpK |= (pPiece->IsPromoted()) ? S[4] : 0;
					break;
				case 3:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[5];
					else
						nTmpWP |= S[5];
					nTmpK |= (pPiece->IsPromoted()) ? S[5] : 0;
					break;
				case 5:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[6];
					else
						nTmpWP |= S[6];
					nTmpK |= (pPiece->IsPromoted()) ? S[6] : 0;
					break;
				case 7:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[7];
					else
						nTmpWP |= S[7];
					nTmpK |= (pPiece->IsPromoted()) ? S[7] : 0;
					break;
				default:
					return FALSE;
				}
				break;
			case 7:
				switch(c)
				{
				case 0:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[0];
					else
						nTmpWP |= S[0];
					nTmpK |= (pPiece->IsPromoted()) ? S[0] : 0;
					break;
				case 2:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[1];
					else
						nTmpWP |= S[1];
					nTmpK |= (pPiece->IsPromoted()) ? S[1] : 0;
					break;
				case 4:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[2];
					else
						nTmpWP |= S[2];
					nTmpK |= (pPiece->IsPromoted()) ? S[2] : 0;
					break;
				case 6:
					if(nPlayerTurn == CHECKER_TEAM_RED)
						nTmpBP |= S[3];
					else
						nTmpWP |= S[3];
					nTmpK |= (pPiece->IsPromoted()) ? S[3] : 0;
					break;
				default:
					return FALSE;
				}
				break;
			default:
				return FALSE;
			}
		}

	mBP = nTmpBP;
	mWP = nTmpWP;
	mK = nTmpK;

	return TRUE;
}

VOID Game::SetTurn(INT a_nTeam)
{
	if(a_nTeam == CHECKER_TEAM_RED)
		mTurn = TRUE;
	else
		mTurn = FALSE;
}