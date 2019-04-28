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
    GameStateStart= 0,
    GameStateSplash,
    GameStateMenu,
    GameStateRun,
    GameStatePause,
    GameStateEnd,
    GameStateGameOver,

    GameStateNumber,

    GameStateNone = orxENUM_NONE
  };


                orxFLOAT        GetTime() const       {return mfTime;}
                GameState       GetGameState() const  {return meGameState;}

                orxSTATUS       GetGridPosition(const orxVECTOR &_rvPos, orxS32 &_rs32X, orxS32 &_rs32Y) const;
                orxU64          GetGridValue(orxS32 _s32X, orxS32 _s32Y) const;
                void            SetGridValue(orxS32 _s32X, orxS32 _s32Y, orxU64 _u64Value);
                void            GetGridSize(orxS32 &_rs32Width, orxS32 &_rs32Height) const;
                void            ClearLine(orxS32 _s32Line);
                orxBOOL         AddLine(orxS32 _s32Line);

                void            DumpGrid();

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
                orxVECTOR       mvBlockSize;
                orxU64         *mau64Grid;
                ScrollObject   *mpoScene;
                Tetro          *mpoPreview;
                Tetro          *mpoSelection;
                GameState       meGameState;
                orxFLOAT        mfTime;
                orxS32          ms32GridWidth, ms32GridHeight;
                orxFLOAT        mfFallTime;
                orxFLOAT        mfLeftTime, mfRightTime;
};

#endif // __LD44_H_
