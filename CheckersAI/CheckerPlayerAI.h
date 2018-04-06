#pragma once

#ifndef __CHECKER_PLAYER_AI__
#define __CHECKER_PLAYER_AI__

//#include "Precomp.h"
class CCheckerGame;
class Game;
class NewAI;

typedef struct _ST_THREAD_PARAM
{
	BOOL* m_bThreadRunning;
	UINT m_nDifficluty;
	CCheckerGame* m_pCheckerGame;
} ST_THREAD_PARAM, *PST_THREAD_PARAM;

class CCheckerPlayerAI
{
public:
	VOID SetCheckerGame(CCheckerGame* a_pCheckerGame)
	{
		m_pCheckerGame = a_pCheckerGame; 
		m_stThreadParam.m_pCheckerGame = m_pCheckerGame;
	}
	VOID SetCheckerAIDifficulty(UINT a_nDifficulty);
	BOOL MakeMove();

public:
	CCheckerPlayerAI(INT a_nTeam, INT a_nDifficulty = 15);
	virtual ~CCheckerPlayerAI();

private:
	//VOID TransformIndexToRowCol(INT a_nIndex, INT* a_nRow, INT* a_nCol);

private:
	ST_THREAD_PARAM	m_stThreadParam;
	CWinThread* m_pThread;
	BOOL		m_bThreadRunning;

private:
	CCheckerGame* m_pCheckerGame;
	INT	m_nTeam;
};


#endif // !__CHECKER_GAME_AI
