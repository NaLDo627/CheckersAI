#include "stdafx.h"
#include "CheckerGame.h"
#include "CheckerPlayerAI.h"

#include <algorithm>
#include <iostream>
#include <limits>

static INT s_nMoveCount = 0;
static BOOL s_bStopFunction = 0; // ����۽� �̹� ���ư��� �ִ� �Լ� ��� �ߴ�
static CMutex s_mtx;
CCheckerGame CCheckerPlayerAI::s_Game(false, false);

CCheckerPlayerAI::CCheckerPlayerAI(INT a_nRedDifficulty/* = 10*/, INT a_nWhiteDifficulty/* = 10*/)
{
	m_pCheckerGame = NULL;
	m_bThreadRunning = TRUE;
	m_stThreadParam.m_bThreadRunning = &m_bThreadRunning;
	m_stThreadParam.m_pCheckerGame = NULL;
	m_stThreadParam.m_pCheckerAI = this;
	m_stThreadParam.m_nRedDifficluty = a_nRedDifficulty;
	m_stThreadParam.m_nWhiteDifficluty = a_nWhiteDifficulty;
	m_pThread = NULL;
	m_stRootNode.m_unDepth = 0;
	s_nMoveCount = 0;
	s_bStopFunction = FALSE;
}

CCheckerPlayerAI::~CCheckerPlayerAI()
{
	m_bThreadRunning = FALSE;
	m_pCheckerGame = NULL;
	m_stRootNode.m_vBranches.clear();
}

UINT WorkerThread(LPVOID a_pParam)
{
	PST_THREAD_PARAM pstParam = (PST_THREAD_PARAM)a_pParam;
	BOOL* pThreadRunning = pstParam->m_bThreadRunning;
	CCheckerGame* pCheckerGame = pstParam->m_pCheckerGame;
	CCheckerPlayerAI* pCheckerAI = pstParam->m_pCheckerAI;
	ST_MOVE_POS stMovePos;
	BOOL bResult = FALSE;
	BOOL bForceRestarted = FALSE;

	while(1)
	{
		// ���� ����۽� ����ó��
		if(bForceRestarted)
		{
			// ����� �Ǿ��µ� MoveCount�� 0�̶�� 1 ����
			if(pCheckerGame->IsCurrentPlayerAI() && s_nMoveCount == 0)
				s_nMoveCount++;
		}
		bForceRestarted = FALSE;

		if(!pCheckerGame || !(s_nMoveCount > 0))
		{
			if(!*pThreadRunning)
				return 0;

			Sleep(1);
			continue;
		}

		s_AppMutex.Lock();
		if(s_nMoveCount > 0)
			s_nMoveCount--;
		s_AppMutex.Unlock();
		
		if(pCheckerGame->GetPlayerTurn() == CHECKER_TEAM_RED)
			pCheckerAI->SetDifficulty(pstParam->m_nRedDifficluty);
		else
			pCheckerAI->SetDifficulty(pstParam->m_nWhiteDifficluty);

		s_AppMutex.Lock();
		s_bStopFunction = FALSE;
		s_AppMutex.Unlock();

		// Step 2. AI�� ���� �̵����� �˷��ش�.
		stMovePos = pCheckerAI->EvaluateGame(*pCheckerGame);

		// ���� ����� ó��
		s_AppMutex.Lock();
		if(s_bStopFunction)
		{
			bForceRestarted = TRUE;
			s_bStopFunction = FALSE;
			s_nMoveCount = 0;
			s_AppMutex.Unlock();
			continue;
		}
		s_AppMutex.Unlock();

		// �̵��� ��ΰ� ���ٸ� ���� ��
		if(stMovePos.m_sSrc == 0 && stMovePos.m_sDst == 0)
		{
			s_AppMutex.Lock();
			if(s_bStopFunction)
			{
				bForceRestarted = TRUE;
				s_bStopFunction = FALSE;
				s_nMoveCount = 0;
				s_AppMutex.Unlock();
				continue;
			}
			s_AppMutex.Unlock();
		}

		// Step 3. �׸��� �̵��ϸ� �ȴ�.
		pCheckerGame->BitSideMovePiece(stMovePos);
	}

	return 0;
}

VOID CCheckerPlayerAI::InitializeAI(INT a_nRedDifficulty/* = 10*/, INT a_nWhiteDifficulty/* = 10*/)
{
	//m_bThreadRunning = FALSE;
	//s_Game.InitalizeGame(FALSE, FALSE);
	m_stThreadParam.m_nRedDifficluty = a_nRedDifficulty;
	m_stThreadParam.m_nWhiteDifficluty = a_nWhiteDifficulty;
	s_AppMutex.Lock();
//	s_nMoveCount = 0;
	s_bStopFunction = TRUE;
	s_AppMutex.Unlock();
	//m_stRootNode.m_vBranches.clear();
}

BOOL CCheckerPlayerAI::MakeMove()
{
	if(!m_pThread)
		m_pThread = ::AfxBeginThread(WorkerThread, &m_stThreadParam);

	s_AppMutex.Lock();
	if(m_pCheckerGame->IsCurrentPlayerAI())
		s_nMoveCount++;
	s_AppMutex.Unlock();
	
	return TRUE;
}

VOID CCheckerPlayerAI::Initialize(const GameState& a_State)
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
	// �߰�: ���� ������ �����ϰ�, �����Ҽ� �ִ� ���� �ϳ��ۿ� ���ٸ� �װ� ����
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

	if(s_bStopFunction)
		return {0, 0, false};

	const GameState CurrentState = a_Game.GetState();
	Initialize(CurrentState);

	// �������� �ּڰ��� �ִ� ����
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

	// ���� �ּ��� �������� �����Ѵ�.
	return m_stRootNode.m_vBranches[nIndex]->m_stMovePos;
}

INT CCheckerPlayerAI::AlphaBeta(Node& node, INT alpha, INT beta, BOOL player)
{
	if(s_bStopFunction)
		return 0;

	if(node.m_unDepth >= m_unDifficulty)
	{ 
		if(IsNoisy(!player))
			return Quiescence(node, alpha, beta, !player);
		return node.m_CurState.GetPoint();
	}

	if(!s_Game.getJumpers() && !s_Game.getMovers())
		return node.m_CurState.GetPoint();

	// ������ �������� �����Ѵ�.
	GenerateOutcomes(node);

	// �÷��̾ ���� ����-��Ÿ ����ġ��� �ִ밪 Ȥ�� �ּҰ��� �̾Ƴ���.
	if(player)
	{
		for(auto & Child : node.m_vBranches)
		{
			alpha = max(alpha, AlphaBeta(*(Child), alpha, beta, !player));

			// ����ġ��
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

			// ����ġ��
			if(beta <= alpha)
				break;
		}
		node.m_nScore = beta;
		return beta;
	}
}

INT CCheckerPlayerAI::Quiescence(Node& node, INT alpha, INT beta, BOOL player)
{
	if(s_bStopFunction)
		return 0;

	if(!s_Game.getJumpers() && !s_Game.getMovers())
		return node.m_CurState.GetPoint();

	// ������ ���� �������� �����Ѵ�.
	GenerateJumps(node);

	if(node.m_vBranches.empty())
		return node.m_CurState.GetPoint();

	// �÷��̾ ���� ����-��Ÿ ����ġ��� �ִ밪 Ȥ�� �ּҰ��� �̾Ƴ���.
	if(player)
	{
		for(auto & child : node.m_vBranches)
		{
			alpha = max(alpha, Quiescence(*(child), alpha, beta, !player));

			// ����ġ��
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

			// ����ġ��
			if(beta <= alpha)
				break;
		}
		node.m_nScore = beta;
		return beta;
	}
}

BOOL CCheckerPlayerAI::IsNoisy(BOOL a_bCurrentTurnRed)
{
	if(s_bStopFunction)
		return 0;

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
