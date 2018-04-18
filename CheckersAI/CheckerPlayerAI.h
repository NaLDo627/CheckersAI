#pragma once

#ifndef __CHECKER_PLAYER_AI__
#define __CHECKER_PLAYER_AI__
#include <utility>
#include <vector>
#include <memory>

struct Node 
{
	GameState m_CurState;
	INT m_nScore;
	std::vector<std::unique_ptr<Node>> m_vBranches;
	ST_MOVE_POS m_stMovePos;
	UINT m_unDepth;
	BOOL m_bChained;

	Node()  {}
	Node(const GameState a_State, const ST_MOVE_POS& a_Move, UINT a_unDepth, BOOL a_bChained)
		: m_CurState(a_State), m_stMovePos(a_Move) , m_unDepth(a_unDepth), m_bChained(a_bChained), m_nScore(0){}
	VOID MoveAndRollback(const ST_MOVE_POS&);
};

class CCheckerGame;

typedef struct _ST_THREAD_PARAM
{
	BOOL* m_bThreadRunning;
	UINT m_nDifficluty;
	CCheckerGame* m_pCheckerGame;
	CCheckerPlayerAI* m_pCheckerAI;
} ST_THREAD_PARAM, *PST_THREAD_PARAM;

class CCheckerPlayerAI
{
public:
	friend struct Node;
	ST_MOVE_POS EvaluateGame(CCheckerGame&);
	BOOL MakeMove();

public:
	VOID SetCheckerGame(CCheckerGame* a_pCheckerGame)
	{
		m_pCheckerGame = a_pCheckerGame; 
		m_stThreadParam.m_pCheckerGame = m_pCheckerGame;
	}

	VOID SetDifficulty(UINT a_unDifficulty)
	{
		m_unDifficulty = a_unDifficulty;
	}

	UINT GetDifficulty() const
	{
		return m_unDifficulty;
	}

public:
	CCheckerPlayerAI(INT a_nTeam, INT a_nDifficulty = 10);
	virtual ~CCheckerPlayerAI();

private:
	VOID GenerateRedMove(Node&);
	VOID GenerateWhiteMove(Node&);
	VOID GenerateMoves(Node& n)
	{
		if(s_Game.m_bCurrentTurnRed)
			GenerateRedMove(n);
		else
			GenerateWhiteMove(n);
	}

	VOID GenerateRedJump(Node&);
	VOID GenerateWhiteJump(Node&);
	VOID GenerateJumps(Node& n)
	{
		if(s_Game.m_bCurrentTurnRed)
			GenerateRedJump(n);
		else
			GenerateWhiteJump(n);
	}

	VOID GenerateOutcomes(Node&);
	ST_MOVE_POS MustJumpThenJump(CCheckerGame&);

	VOID Initialize(const GameState&);
	BOOL IsNoisy(BOOL a_bCurrentTurnRed);

	INT AlphaBeta(Node& node, INT alpha, INT beta, BOOL player);
	INT Quiescence(Node& node, INT alpha, INT beta, BOOL player);

private:
	ST_THREAD_PARAM	m_stThreadParam;
	CWinThread* m_pThread;
	BOOL		m_bThreadRunning;

private:
	CCheckerGame* m_pCheckerGame;
	INT	m_nTeam;

private:
	Node m_stRootNode;
	UINT m_unDifficulty;
	static CCheckerGame s_Game;
};


#endif // !__CHECKER_GAME_AI
