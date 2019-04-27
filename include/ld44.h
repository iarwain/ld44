#ifndef __LD44_H_
#define __LD44_H_

//! Includes
#define __NO_SCROLLED__ // Uncomment this define to prevent the embedded editor (ScrollEd) from being compiled
#include "Scroll.h"
#include "tetro.h"

//! LD44
class LD44 : public Scroll<LD44>
{
public:

  enum GameState
  {
    GameStateSplash = 0,
    GameStateMenu,
    GameStateRun,
    GameStatePause,
    GameStateEnd,

    GameStateNumber,

    GameStateNone = orxENUM_NONE
  };


                orxFLOAT        GetTime() const       {return mfTime;}
                GameState       GetGameState() const  {return meGameState;}

                orxSTATUS       GetGridPosition(const orxVECTOR &_rvPos, orxS32 &_rs32X, orxS32 &_rs32Y) const;
                orxU64          GetGridValue(orxS32 _s32X, orxS32 _s32Y) const;
                void            SetGridValue(orxS32 _s32X, orxS32 _s32Y, orxU64 _u64Value);
                void            GetGridSize(orxS32 &_rs32Width, orxS32 &_rs32Height) const;
                void            CleanGridLine(orxS32 _s32Line);


private:

                orxSTATUS       Bootstrap() const;

                void            Update(const orxCLOCK_INFO &_rstInfo);
                void            UpdateGame(const orxCLOCK_INFO &_rstInfo);

                orxSTATUS       Init();
                orxSTATUS       Run();
                void            Exit();
                void            BindObjects();

  static        orxSTATUS orxFASTCALL EventHandler(const orxEVENT *_pstEvent);


private:
                GameState       meGameState;
                orxFLOAT        mfTime;
                orxU64          *mau64Grid;
                orxS32          ms32GridWidth, ms32GridHeight;
                orxFLOAT        mfFallTime;
                orxFLOAT        mfLeftTime, mfRightTime;
};

#endif // __LD44_H_
