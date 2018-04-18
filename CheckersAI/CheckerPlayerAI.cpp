#include "stdafx.h"
#include "CheckerGame.h"
#include "CheckerPlayerAI.h"

#include <algorithm>
#include <iostream>
#include <limits>

static BOOL s_nMoveCount = 0;
static CMutex s_mtx;
CCheckerGame CCheckerPlayerAI::s_Game(false, false);

CCheckerPlayerAI::CCheckerPlayerAI(INT a_nTeam, INT a_nDifficulty /*= 15*/)
{
	m_pCheckerGame = NULL;
	m_nTeam = a_nTeam;
	m_bThreadRunning = TRUE;
	m_stThreadParam.m_bThreadRunning = &m_bThreadRunning;
	m_stThreadParam.m_pCheckerGame = NULL;
	m_stThreadParam.m_pCheckerAI = this;
	m_stThreadParam.m_nDifficluty = a_nDifficulty;
	m_pThread = NULL;
	m_stRootNode.m_unDepth = 0;
}

CCheckerPlayerAI::~CCheckerPlayerAI()
{
	s_AppMutex.Lock();
	m_unDifficulty = 0;

	//s_mtx.Lock();
	m_bThreadRunning = FALSE;
	//s_mtx.Unlock();
	
	m_stRootNode.m_vBranches.clear();
	s_AppMutex.Unlock();
}

UINT WorkerThread(LPVOID a_pParam)
{
	PST_THREAD_PARAM pstParam = (PST_THREAD_PARAM)a_pParam;
	BOOL* pThreadRunning = pstParam->m_bThreadRunning;
	CCheckerGame* pCheckerGame = pstParam->m_pCheckerGame;
	CCheckerPlayerAI* pCheckerAI = pstParam->m_pCheckerAI;
	ST_MOVE_POS stMovePos;
	INT  nSrcIndex = 0;
	INT	 nSrcRow = 0;
	INT	 nSrcCol = 0;
	INT	 nDstIndex = 0;
	INT	 nDstRow = 0;
	INT	 nDstCol = 0;
	INT	 nMoveRow = 0;
	INT	 nMoveCol = 0;
	BOOL bResult = FALSE;

	while(1)
	{
		if(!pCheckerGame || !(s_nMoveCount > 0))
		{
			if(!*pThreadRunning)
				return 0;

			Sleep(1);
			continue;
		}

		
		pCheckerAI->SetDifficulty(pstParam->m_nDifficluty);

		// Step 2. AI가 어디로 이동할지 알려준다.
		s_mtx.Lock();
		stMovePos = pCheckerAI->EvaluateGame(*pCheckerGame);
		s_mtx.Unlock();

		// 이동할 경로가 없다면 게임 끝
		if(stMovePos.m_sSrc == 0 && stMovePos.m_sDst == 0)
			return 0;

		// Step 3. 그리로 이동하면 된다.
		s_AppMutex.Lock();
		bResult = pCheckerGame->BitSideMovePiece(stMovePos);
		s_AppMutex.Unlock();
		

		if(!bResult)
			continue;

		s_nMoveCount--;
	}

	return 0;
}

BOOL CCheckerPlayerAI::MakeMove()
{
	if(!m_pThread)
		m_pThread = ::AfxBeginThread(WorkerThread, &m_stThreadParam);

	s_nMoveCount++;
	
	return TRUE;
}

void CCheckerPlayerAI::Initialize(const GameState& a_State)
{
	s_Game.RollbackState(a_State);
	if(!m_stRootNode.m_vBranches.empty())
		m_stRootNode.m_vBranches.clear();
	m_stRootNode.m_CurState = a_State;
}

ST_MOVE_POS CCheckerPlayerAI::MustJumpThenJump(CCheckerGame& a_Game)
{
	using namespace std;

	BitBoard jumpers = a_Game.getJumpers();
	BitBoard j = BitCalculator::PickHighestBit(jumpers);

	if(a_Game.m_bCurrentTurnRed)
	{
		BitBoard victims = a_Game.m_WhitePiece;
		BitBoard vict = RotateLeft(j, 7) & victims;
		if(a_Game.canJump(j, vict))
			return { bbUMap[j], bbUMap[vict], TRUE };

		vict = RotateLeft(j, 1) & victims;
		if(a_Game.canJump(j, vict))
			return { bbUMap[j], bbUMap[vict], TRUE };

		if(j & a_Game.m_KingPiece)
		{
			vict = RotateRight(j, 7) & victims;
			if(a_Game.canJump(j, vict))
				return { bbUMap[j], bbUMap[vict], TRUE };

			vict = RotateRight(j, 1) & victims;
			if(a_Game.canJump(j, vict))
				return { bbUMap[j], bbUMap[vict], TRUE };
		}
	}
	else
	{
		BitBoard victims = a_Game.m_RedPiece;
		BitBoard vict = RotateRight(j, 7) & victims;
		if(a_Game.canJump(j, vict))
			return { bbUMap[j], bbUMap[vict], TRUE };
		vict = RotateRight(j, 1) & victims;
		if(a_Game.canJump(j, vict))
			return { bbUMap[j], bbUMap[vict], TRUE };

		if(j & a_Game.m_KingPiece)
		{
			vict = RotateLeft(j, 7) & victims;
			if(a_Game.canJump(j, vict))
				return { bbUMap[j], bbUMap[vict], TRUE };
			vict = RotateLeft(j, 1) & victims;
			if(a_Game.canJump(j, vict))
				return { bbUMap[j], bbUMap[vict], TRUE };
		}
	}

	return {0,0,FALSE};
}

ST_MOVE_POS CCheckerPlayerAI::EvaluateGame(CCheckerGame& a_Game)
{
	// 추가: 만약 점프만 가능하고, 점프할수 있는 것이 하나밖에 없다면 그것 점프
	if(a_Game.m_bCurrentTurnRed)
	{
		if((a_Game.m_RedPiece & a_Game.m_MustJumpPiece) && 
			BitCalculator::CountBit(a_Game.m_RedPiece & a_Game.m_MustJumpPiece) == 1)
			return MustJumpThenJump(a_Game);
	}
	else
	{
		if((a_Game.m_WhitePiece & a_Game.m_MustJumpPiece) && 
			BitCalculator::CountBit(a_Game.m_WhitePiece & a_Game.m_MustJumpPiece) == 1)
			return MustJumpThenJump(a_Game);
	}

	const GameState CurrentState = a_Game.GetState();
	Initialize(CurrentState);

	// 가능한한 최솟값과 최댓값 대입
	INT nAlpha = INT_MIN;
	INT nBeta = INT_MAX;

	AlphaBeta(m_stRootNode, nAlpha, nBeta, CurrentState.m_bCurrentTurnRed);

	if(m_stRootNode.m_vBranches.empty()) return { 0,0,false };

	INT nIndex = 0;
	if(a_Game.m_bCurrentTurnRed)
	{
		INT nMax = nAlpha;
		for(size_t i = 0; i < m_stRootNode.m_vBranches.size(); i++)
		{
			if(m_stRootNode.m_vBranches[i]->m_nScore > nMax)
			{
				nMax = m_stRootNode.m_vBranches[i]->m_nScore;
				nIndex = i;
			}
		}
	}
	else
	{
		INT nMin = nBeta;
		for(size_t i = 0; i < m_stRootNode.m_vBranches.size(); i++)
		{
			if(m_stRootNode.m_vBranches[i]->m_nScore < nMin)
			{
				nMin = m_stRootNode.m_vBranches[i]->m_nScore;
				nIndex = i;
			}
		}
	}

	// 가장 최선의 움직임을 리턴한다.
	return m_stRootNode.m_vBranches[nIndex]->m_stMovePos;
}

INT CCheckerPlayerAI::AlphaBeta(Node& node, INT alpha, INT beta, BOOL player)
{
	if(node.m_unDepth >= m_unDifficulty)
	{ 
		if(IsNoisy(!player))
			return Quiescence(node, alpha, beta, !player);
		return node.m_CurState.GetPoint();
	}

	if(!s_Game.getJumpers() && !s_Game.getMovers())
		return node.m_CurState.GetPoint();

	// 가능한 가짓수를 생성한다.
	GenerateOutcomes(node);

	// 플레이어에 따라 알파-베타 가지치기로 최대값 혹은 최소값을 뽑아낸다.
	if(player)
	{
		for(auto & Child : node.m_vBranches)
		{
			alpha = max(alpha, AlphaBeta(*(Child), alpha, beta, !player));

			// 가지치기
			if(beta <= alpha)
				break;
		}
		node.m_nScore = alpha;
		return alpha;
	}
	else
	{
		for(auto & Child : node.m_vBranches)
		{
			beta = min(beta, AlphaBeta(*(Child), alpha, beta, !player));

			// 가지치기
			if(beta <= alpha)
				break;
		}
		node.m_nScore = beta;
		return beta;
	}
}

INT CCheckerPlayerAI::Quiescence(Node& node, INT alpha, INT beta, BOOL player)
{
	using std::max;
	using std::min;

	if(!s_Game.getJumpers() && !s_Game.getMovers())
		return node.m_CurState.GetPoint();

	// 가능한 점프 가짓수를 생성한다.
	GenerateJumps(node);

	if(node.m_vBranches.empty())
		return node.m_CurState.GetPoint();

	// 플레이어에 따라 알파-베타 가지치기로 최대값 혹은 최소값을 뽑아낸다.
	if(player)
	{
		for(auto & child : node.m_vBranches)
		{
			alpha = max(alpha, Quiescence(*(child), alpha, beta, !player));

			// 가지치기
			if(beta <= alpha)
				break;
		}
		node.m_nScore = alpha;
		return alpha;
	}
	else
	{
		for(auto & child : node.m_vBranches)
		{
			beta = min(beta, Quiescence(*(child), alpha, beta, !player));

			// 가지치기
			if(beta <= alpha)
				break;
		}
		node.m_nScore = beta;
		return beta;
	}
}

BOOL CCheckerPlayerAI::IsNoisy(BOOL a_bCurrentTurnRed)
{
	BitBoard jumpers = s_Game.getJumpers();
	BitBoard j = BitCalculator::PickHighestBit(jumpers);

	if(a_bCurrentTurnRed)
	{
		BitBoard victims = s_Game.m_WhitePiece;
		BitBoard vict;
		vict = RotateLeft(j, 7) & victims;
		if(s_Game.canJump(j, vict))
			return TRUE;
		vict = RotateLeft(j, 1) & victims;
		if(s_Game.canJump(j, vict))
			return TRUE;

		if(j & s_Game.m_KingPiece)
		{
			vict = RotateRight(j, 7) & victims;
			if(s_Game.canJump(j, vict))
				return TRUE;
			vict = RotateRight(j, 1) & victims;
			if(s_Game.canJump(j, vict))
				return TRUE;
		}
	}
	else
	{
		BitBoard victims = s_Game.m_RedPiece;

		BitBoard vict = RotateRight(j, 7) & victims;
		if(s_Game.canJump(j, vict))
			return TRUE;
		vict = RotateRight(j, 1) & victims;
		if(s_Game.canJump(j, vict))
			return TRUE;

		if(j & s_Game.m_KingPiece)
		{
			vict = RotateLeft(j, 7) & victims;
			if(s_Game.canJump(j, vict))
				return TRUE;
			vict = RotateLeft(j, 1) & victims;
			if(s_Game.canJump(j, vict))
				return TRUE;
		}
	}

	return FALSE;	
}

VOID CCheckerPlayerAI::GenerateOutcomes(Node& n)
{
	GenerateJumps(n);
	if(n.m_vBranches.empty())
		GenerateMoves(n);
}

VOID CCheckerPlayerAI::GenerateRedMove(Node& n)
{
	BitBoard Movers = s_Game.getMovers();

	BitBoard empty = s_Game.getEmpty();
	BitBoard target;
	while(Movers)
	{
		const BitBoard mover = BitCalculator::PickHighestBit(Movers);
		Movers ^= mover;
		if((target = ((RotateLeft(mover & CAN_UPLEFT, 7) & empty))))
			n.MoveAndRollback({ bbUMap[mover], bbUMap[target], FALSE });
		if((target = ((RotateLeft(mover & CAN_UPRIGHT, 1) & empty))))
			n.MoveAndRollback({ bbUMap[mover], bbUMap[target], FALSE });

		if(mover & s_Game.m_KingPiece)
		{
			if((target = ((RotateRight(mover & CAN_DOWNRIGHT, 7) & empty))))
				n.MoveAndRollback({ bbUMap[mover], bbUMap[target], FALSE });
			if((target = ((RotateRight(mover & CAN_DOWNLEFT, 1) & empty))))
				n.MoveAndRollback({ bbUMap[mover], bbUMap[target], FALSE });
		}
	}
}

VOID CCheckerPlayerAI::GenerateWhiteMove(Node& n)
{
	using namespace std;

	BitBoard Movers = s_Game.getMovers();
	BitBoard empty = s_Game.getEmpty();
	BitBoard target;
	while(Movers)
	{
		BitBoard mover = BitCalculator::PickHighestBit(Movers);
		Movers ^= mover;

		if((target = ((RotateRight(mover & CAN_DOWNRIGHT, 7) & empty))))
			n.MoveAndRollback({ bbUMap[mover], bbUMap[target], FALSE });

		if((target = ((RotateRight(mover & CAN_DOWNLEFT, 1) & empty))))
			n.MoveAndRollback({ bbUMap[mover], bbUMap[target], FALSE });

		if(mover & s_Game.m_KingPiece)
		{
			if((target = ((RotateLeft(mover & CAN_UPLEFT, 7) & empty))))
				n.MoveAndRollback({ bbUMap[mover], bbUMap[target], FALSE });
			if((target = ((RotateLeft(mover & CAN_UPRIGHT, 1) & empty))))
				n.MoveAndRollback({ bbUMap[mover], bbUMap[target], FALSE });
		}
	}
}

VOID CCheckerPlayerAI::GenerateRedJump(Node& n)
{
	using namespace std;

	BitBoard jumpers = s_Game.getJumpers();

	while(jumpers)
	{
		BitBoard j = BitCalculator::PickHighestBit(jumpers);
		jumpers ^= j;
		BitBoard victims = s_Game.m_WhitePiece;
		BitBoard vict;
		vict = RotateLeft(j, 7) & victims;
		if(s_Game.canJump(j, vict))
			n.MoveAndRollback({ bbUMap[j], bbUMap[vict], TRUE });
		vict = RotateLeft(j, 1) & victims;
		if(s_Game.canJump(j, vict))
			n.MoveAndRollback({ bbUMap[j], bbUMap[vict], TRUE });

		if(j & s_Game.m_KingPiece)
		{
			vict = RotateRight(j, 7) & victims;
			if(s_Game.canJump(j, vict))
				n.MoveAndRollback({ bbUMap[j], bbUMap[vict], TRUE });
			vict = RotateRight(j, 1) & victims;
			if(s_Game.canJump(j, vict))
				n.MoveAndRollback({ bbUMap[j], bbUMap[vict], TRUE });
		}
	}
}

VOID CCheckerPlayerAI::GenerateWhiteJump(Node& n)
{
	using namespace std;

	BitBoard jumpers = s_Game.getJumpers();

	while(jumpers)
	{
		BitBoard j = BitCalculator::PickHighestBit(jumpers);
		jumpers ^= j;
		BitBoard victims = s_Game.m_RedPiece;

		BitBoard vict = RotateRight(j, 7) & victims;
		if(s_Game.canJump(j, vict))
			n.MoveAndRollback({ bbUMap[j], bbUMap[vict], TRUE });
		vict = RotateRight(j, 1) & victims;
		if(s_Game.canJump(j, vict))
			n.MoveAndRollback({ bbUMap[j], bbUMap[vict], TRUE });

		if(j & s_Game.m_KingPiece)
		{
			vict = RotateLeft(j, 7) & victims;
			if(s_Game.canJump(j, vict))
				n.MoveAndRollback({ bbUMap[j], bbUMap[vict], TRUE });
			vict = RotateLeft(j, 1) & victims;
			if(s_Game.canJump(j, vict))
				n.MoveAndRollback({ bbUMap[j], bbUMap[vict], TRUE });
		}
	}
}

VOID Node::MoveAndRollback(const ST_MOVE_POS& a_stMovePos)
{
	CCheckerPlayerAI::s_Game.BitSideMovePiece(a_stMovePos);
	
	m_vBranches.push_back(
		std::unique_ptr<Node>(
			new Node(CCheckerPlayerAI::s_Game.GetState(), a_stMovePos, m_unDepth + 1, FALSE)));
	CCheckerPlayerAI::s_Game.RollbackState(m_CurState);
}
