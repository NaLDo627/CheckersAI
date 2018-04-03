#pragma once

#ifndef __CHECKER_PLAYER_AI__
#define __CHECKER_PLAYER_AI__

//#include "Precomp.h"
class CCheckerGame;
class Game;
class NewAI;

class CCheckerPlayerAI
{
public:
	VOID SetCheckerGame(CCheckerGame* a_pCheckerGame) { m_pCheckerGame = a_pCheckerGame; };
	BOOL MakeMove();

public:
	CCheckerPlayerAI(INT a_nTeam, INT a_nDifficulty = 15);
	virtual ~CCheckerPlayerAI();

private:
	VOID TransformIndexToRowCol(INT a_nIndex, INT* a_nRow, INT* a_nCol);

private:
	CCheckerGame* m_pCheckerGame;
	INT	m_nTeam;

private:
	Game* m_pInternalGame;
	NewAI* m_pInternalAI;
};


#endif // !__CHECKER_GAME_AI
