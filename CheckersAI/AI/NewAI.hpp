/*
 * NewAI.hpp
 *
 *  Created on: Nov 18, 2012
 *      Author: mark
 */

#ifndef NEWAI_HPP_
#define NEWAI_HPP_

#include <utility>
#include <vector>
#include <memory>

#include "BitBoard.hpp"
#include "Save.hpp"
#include "Game.hpp"

struct Node {
	Save save;
	int val;
	std::vector<std::unique_ptr<Node> > branches;
	Move move;
	unsigned int depth;

	Node()  {}
	Node(const Save& s, const Move& m, unsigned int depth) : save(s), move(m) , depth(depth){}
	void move_and_restore(const Move&);
	void jump_and_restore(const Move&);
};

class BaseAI
{
public:
	//	virtual void print_scene() = 0;

	virtual Move get_random_move() const = 0;

	virtual Move evaluate_game(Game&) = 0;

	virtual ~BaseAI()
	{
	};
};

class NewAI: public BaseAI {
public:
	friend struct Node;

	NewAI(const unsigned char difficulty);
	virtual ~NewAI();

//	void print_scene();

	Move get_random_move() const;

	Move evaluate_game(Game&);


	void difficulty(unsigned difficulty)
	{
		_difficulty = difficulty;
	}

	unsigned difficulty() const
	{
		return _difficulty;
	}

private:
	void gen_moves_red(Node&);
	void gen_moves_white(Node&);
	void gen_moves(Node& n) {
		if (s_game.mTurn)
			gen_moves_red(n);
		else
			gen_moves_white(n);
	}


	void gen_jumps_red(Node&);
	void gen_jumps_white(Node&);
	void gen_jumps(Node& n) {
		if (s_game.mTurn)
			gen_jumps_red(n);
		else
			gen_jumps_white(n);
	}

	void gen_outcomes(Node&);

	void initialize(const Save&);

	int alphabeta(Node& node, int alpha, int beta, bool player);

	Node _root;
	unsigned int _difficulty;
	static Game s_game;
	//INT m_nDifficulty;
};



#endif /* NEWAI_HPP_ */
