#include "stdafx.h"
#include "CheckerGameBase.h"
#include <algorithm>

VOID Swap(GameState& first, GameState& second)
{
	using std::swap;

	swap(first.m_bCurrentTurnRed, second.m_bCurrentTurnRed);
	swap(first.m_MustJumpPiece, second.m_MustJumpPiece);
	swap(first.m_WhitePiece, second.m_WhitePiece);
	swap(first.m_RedPiece, second.m_RedPiece);
	swap(first.m_KingPiece, second.m_KingPiece);
}
