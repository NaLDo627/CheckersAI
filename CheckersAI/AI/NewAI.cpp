/*
 * NewAI.cpp
 *
 *  Created on: Nov 18, 2012
 *      Author: mark
 */

#include "stdafx.h"
#include "NewAI.hpp"

#include <algorithm>
#include <iostream>
#include <limits>

Game NewAI::s_game(false, false);

NewAI::NewAI(const unsigned char difficulty) :
				_difficulty(difficulty)
{
	_root.depth = 0;
	// TODO Auto-generated constructor stub

}

NewAI::~NewAI()
{
	// TODO Auto-generated destructor stub
	_difficulty = 0;
	_root.branches.clear();
	Sleep(100);
}

void NewAI::initialize(const Save& s)
{
	s_game.restoreToSave(s);
	_root.branches.clear();
	_root.save = s;
}

Move NewAI::evaluate_game(Game& g)
{
	const Save old_save = g.getSave();
	initialize(old_save);

	//int alpha = std::numeric_limits<int>::min();
	// 가능한한 최솟값과 최댓값 대입
	int alpha = INT_MIN;
	int beta = INT_MAX;

	alphabeta(_root, alpha, beta, old_save.turn);

	if (_root.branches.empty()) return {0,0,false};

	int idx = 0;
	if (g.mTurn) {
		int max = alpha;
		for (unsigned int i = 0; i < _root.branches.size(); i++) {
			if (_root.branches[i]->val > max) {
				max = _root.branches[i]->val;
				idx = i;
			}
//			_root.branches[i]->print();
		}
	} else {
		int min = beta;
		for (unsigned int i = 0; i < _root.branches.size(); i++) {
			if (_root.branches[i]->val < min) {
				min = _root.branches[i]->val;
				idx = i;
			}
//			_root.branches[i]->print();
		}
	}

	// 가장 최선의 움직임을 리턴한다.
	return _root.branches[idx]->move;

}

int NewAI::alphabeta(Node& node, int alpha, int beta, bool player)
{
	using std::max;
	using std::min;

	if(node.depth >= _difficulty)
		return node.save.grade();

	if (!s_game.getJumpers() && !s_game.getMovers())
		return node.save.grade();

	//gen_outcomes(node);

	// 가능한 가짓수를 생성한다.
	if(s_game.mTurn)
		gen_jumps_red(node);
	else
		gen_jumps_white(node);
	if(node.branches.empty())
	{
		if(s_game.mTurn)
			gen_moves_red(node);
		else
			gen_moves_white(node);
	}

	// 플레이어에 따라 알파-베타 가지치기로 최대값 혹은 최소값을 뽑아낸다.
	if (player) {
		for (auto & child : node.branches) {
			alpha = max(alpha, alphabeta(*(child), alpha, beta, !player));

			// 가지치기
			if (beta <= alpha)
				break;
		}
		node.val = alpha;
		return alpha;
	} else {
		for (auto & child : node.branches) {
			beta = min(beta, alphabeta(*(child), alpha, beta, !player));

			// 가지치기
			if (beta <= alpha)
				break;
		}
		node.val = beta;
		return beta;
	}

}

void NewAI::gen_outcomes(Node& n)
{
	gen_jumps(n);
	if (n.branches.empty())
		gen_moves(n);
}

void NewAI::gen_moves_red(Node& n)
{
//	using namespace std;
	using namespace Bit::Masks;
	using Bit::rol;
	using Bit::ror;
	using Bit::highBit;

	BB Movers = s_game.getMovers();
//	std::cout << "++++++++++++" << hex << Movers << dec <<std::endl;

	BB empty = s_game.getEmpty();
	BB target;
	while (Movers) {
		const BB mover = highBit(Movers);
//		std::cout << "++++++++++++mover = :" << hex << mover << dec <<std::endl;
		Movers ^= mover;
		if ((target = ((rol(mover & CAN_UPLEFT, 7) & empty))))
			n.move_and_restore( { bbUMap[mover], bbUMap[target], false });
		if ((target = ((rol(mover & CAN_UPRIGHT, 1) & empty))))
//			mMoves.push_back( { bbUMap[mover], bbUMap[target], false });
			n.move_and_restore( { bbUMap[mover], bbUMap[target], false });

		if (mover & s_game.mK) {
			if ((target = ((ror(mover & CAN_DOWNRIGHT, 7) & empty))))
				n.move_and_restore( { bbUMap[mover], bbUMap[target], false });
			if ((target = ((ror(mover & CAN_DOWNLEFT, 1) & empty))))
				n.move_and_restore( { bbUMap[mover], bbUMap[target], false });
		}
	}
}

void NewAI::gen_moves_white(Node& n)
{
	using namespace std;
	using namespace Bit::Masks;
	using Bit::ror;
	using Bit::rol;
	using Bit::highBit;

	BB Movers = s_game.getMovers();
//	std::cout << "++++++++++++" << hex << Movers << dec <<std::endl;

	BB empty = s_game.getEmpty();
	BB target;
	while (Movers) {
		BB mover = highBit(Movers);
//		std::cout << "++++++++++++mover = :" << hex << mover << dec <<std::endl;
		Movers ^= mover;

		if ((target = ((ror(mover & CAN_DOWNRIGHT, 7) & empty))))
//			mMoves.push_back( { bbUMap[mover], bbUMap[target], false });
			n.move_and_restore( { bbUMap[mover], bbUMap[target], false });

		if ((target = ((ror(mover & CAN_DOWNLEFT, 1) & empty))))
			n.move_and_restore( { bbUMap[mover], bbUMap[target], false });

		if (mover & s_game.mK) {
			if ((target = ((rol(mover & CAN_UPLEFT, 7) & empty))))
				n.move_and_restore( { bbUMap[mover], bbUMap[target], false });
			if ((target = ((rol(mover & CAN_UPRIGHT, 1) & empty))))
				n.move_and_restore( { bbUMap[mover], bbUMap[target], false });
		}
	}
}

void NewAI::gen_jumps_red(Node& n)
{
	using namespace std;
	using Bit::Masks::bbUMap;
	using Bit::rol;
	using Bit::ror;
	using Bit::highBit;

	BB jumpers = s_game.getJumpers();
//	_game->print();
//	std::cout << "++++++++++++" << hex << jumpers << dec << std::endl;

	while (jumpers) {
		BB j = highBit(jumpers);
//		std::cout << "++++++++++++jumper = :" << hex << j << dec <<std::endl;
		jumpers ^= j;
		BB victims = s_game.mWP;
		BB vict;
		vict = rol(j, 7) & victims;
		if (s_game.canJump(j, vict)) {
//			mMoves.push_back( { bbUMap[j], bbUMap[vict], true });
			n.move_and_restore( { bbUMap[j], bbUMap[vict], true });
		}
		vict = rol(j, 1) & victims;
		if (s_game.canJump(j, vict)) {
//			mMoves.push_back( { bbUMap[j], bbUMap[vict], true });
			n.move_and_restore( { bbUMap[j], bbUMap[vict], true });
		}

		if (j & s_game.mK) {
			vict = ror(j, 7) & victims;
			if (s_game.canJump(j, vict)) {
				n.move_and_restore( { bbUMap[j], bbUMap[vict], true });
			}
			vict = ror(j, 1) & victims;
			if (s_game.canJump(j, vict)) {
				n.move_and_restore( { bbUMap[j], bbUMap[vict], true });
			}
		}
	}
}

void NewAI::gen_jumps_white(Node& n)
{
	using namespace std;
	using Bit::Masks::bbUMap;
	using Bit::rol;
	using Bit::ror;
	using Bit::highBit;

	BB jumpers = s_game.getJumpers();

	while (jumpers) {
		BB j = highBit(jumpers);
		jumpers ^= j;
		BB victims = s_game.mBP;

		BB vict = ror(j, 7) & victims;
		if (s_game.canJump(j, vict)) {
//			mMoves.push_back( { bbUMap[j], bbUMap[vict], true });
			n.move_and_restore( { bbUMap[j], bbUMap[vict], true });
		}
		vict = ror(j, 1) & victims;
		if (s_game.canJump(j, vict)) {
			n.move_and_restore( { bbUMap[j], bbUMap[vict], true });
		}

		if (j & s_game.mK) {
			vict = rol(j, 7) & victims;
			if (s_game.canJump(j, vict)) {
				n.move_and_restore( { bbUMap[j], bbUMap[vict], true });
			}
			vict = rol(j, 1) & victims;
			if (s_game.canJump(j, vict)) {
				n.move_and_restore( { bbUMap[j], bbUMap[vict], true });
			}
		}
	}
}

Move NewAI::get_random_move() const
{
	unsigned index = rand() % _root.branches.size();

	return _root.branches[index]->move;
}

void Node::move_and_restore(const Move& m)
{
	NewAI::s_game.move(m);

	branches.push_back(
			std::unique_ptr<Node>(
					new Node(NewAI::s_game.getSave(), m, depth + 1)));
	NewAI::s_game.restoreToSave(save);
}
