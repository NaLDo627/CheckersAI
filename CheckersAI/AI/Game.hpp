#pragma once
#ifndef GAME_HPP_
#define GAME_HPP_

#include "BitBoard.hpp"
#include "Save.hpp"

#include <map>
#include <string>
#include <vector>
#include <utility>

class SimpleAI;
class CCheckerGame;

class Game {
public:
	friend class SimpleAI;
	friend class NewAI;

	/* Constructor */
	Game(const bool debug, const bool interact);
	/* Constructor from memory */
	Game(const Save& record, const bool debug, const bool interact);
	/* Destructor */
	virtual ~Game();

	/* return save game */
	Save getSave();

	/* Movement */
	MoveCode move(const Move& move)
	{
		if (move.jump)
			return jump(move);
		else
			return makeMove(move);
	}

	/* restore game to save */
	void restoreToSave(const Save& record);

	/* Get p1 score */
	unsigned getP1score() const
	{
		return Bit::bitCount(mBP & ~mK) + 2 * Bit::bitCount(mBP & mK);
	}
	/* Get p2 score */
	unsigned getP2score() const
	{
		return Bit::bitCount(mWP & ~mK) + 2 * Bit::bitCount(mWP & mK);
	}

	int grade() const;

	unsigned p1NumPieces() const
	{
		return Bit::bitCount(mBP);
	}
	unsigned p2NumPieces() const
	{
		return Bit::bitCount(mWP);
	}

	bool isLive() const
	{

		return (getMovers() || getJumpers());
	}

	std::vector<Cell> toArr() const;

	// Added By PHK
	// 클래스 : CCheckerGame -> Game 으로 변환 후 저장
	BOOL TransformClassToBit(CCheckerGame* a_pCheckerGame);
	VOID SetTurn(INT a_nTeam);

private:
	BitBoard mWP;
	BitBoard mBP;
	BitBoard mK;

	typedef BitBoard BB;
	/* Tracks if it's P1's turn or not */
	bool mTurn;
	bool mDebug;
	Save mSave;
	bool mInteract;
	BB mMustJump;

	/* Update save game */
	void updateSave();

	inline BitBoard getJumpers() const
	{
		using namespace Bit::Masks;
		using Bit::ror;
		using Bit::rol;

		BitBoard empty = ~(mWP | mBP);
		BitBoard Temp;
		BitBoard jumpers = 0;
		if(mTurn)
		{
			BitBoard BK = mBP & mK;
			Temp = ror(empty, 7) & mWP & CAN_UPLEFT;
			jumpers |= ror(Temp, 7) & mBP & CAN_UPLEFT;
			Temp = ror(empty, 1) & mWP & CAN_UPRIGHT;
			jumpers |= ror(Temp, 1) & mBP & CAN_UPRIGHT;

			Temp = rol(empty, 7) & mWP & CAN_DOWNRIGHT;
			jumpers |= rol(Temp, 7) & BK & CAN_DOWNRIGHT;
			Temp = rol(empty, 1) & mWP & CAN_DOWNLEFT;
			jumpers |= rol(Temp, 1) & BK & CAN_DOWNLEFT;
		}
		else
		{
			BitBoard WK = mWP & mK;
			Temp = rol(empty, 7) & mBP & CAN_DOWNRIGHT;
			jumpers |= rol(Temp, 7) & mWP & CAN_DOWNRIGHT;
			Temp = rol(empty, 1) & mBP & CAN_DOWNLEFT;
			jumpers |= rol(Temp, 1) & mWP & CAN_DOWNLEFT;

			Temp = ror(empty, 7) & mBP & CAN_UPLEFT;
			jumpers |= ror(Temp, 7) & WK & CAN_UPLEFT;
			Temp = ror(empty, 1) & mBP & CAN_UPRIGHT;
			jumpers |= ror(Temp, 1) & WK & CAN_UPRIGHT;
		}

		return jumpers;
	}

	inline BitBoard getMovers() const
	{
		using namespace Bit::Masks;
		using Bit::ror;
		using Bit::rol;

		const BB empty = ~(mWP | mBP);
		BB Movers;

		if(mTurn)
		{
			const BB BK = mBP & mK;
			Movers = ror(empty, 7) & mBP & CAN_UPLEFT;
			Movers |= ror(empty, 1) & mBP & CAN_UPRIGHT;
			Movers |= rol(empty, 7) & BK & CAN_DOWNRIGHT;
			Movers |= rol(empty, 1) & BK & CAN_DOWNLEFT;
		}
		else
		{
			const BB WK = mWP & mK; // Kings
			Movers = rol(empty, 7) & mWP & CAN_DOWNRIGHT;
			Movers |= rol(empty, 1) & mWP & CAN_DOWNLEFT;
			Movers |= ror(empty, 7) & WK & CAN_UPLEFT;
			Movers |= ror(empty, 1) & WK & CAN_UPRIGHT;
		}

		return Movers;
	}

	inline BitBoard getEmpty() const
	{
		return ~(mWP | mBP);
	}
	inline BB canJump(const BB src, const BB vict);

	/* Piece Movement */
	MoveCode makeMove(const Move& move);

	/* Jumping */
	MoveCode jump(const Move& move);
};

#endif /* GAME_HPP_ */
