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


                orxFLOAT        GetTime(orxU32 _u32ID) const  {return mastGames[_u32ID].fTime;}
                GameState       GetGameState() const          {return meGameState;}

                void            GetGridSize(orxS32 &_rs32Width, orxS32 &_rs32Height) const;
                orxSTATUS       GetGridPosition(const orxVECTOR &_rvPos, orxS32 &_rs32X, orxS32 &_rs32Y) const;
                orxU64          GetGridValue(orxS32 _s32X, orxS32 _s32Y, orxU32 _u32ID) const;
                void            SetGridValue(orxS32 _s32X, orxS32 _s32Y, orxU64 _u64Value, orxU32 _u32ID);
                void            ClearLine(orxS32 _s32Line, orxU32 _u32ID);
                orxBOOL         AddLine(orxS32 _s32Line, orxU32 _u32ID);
          const orxSTRING       GetGameInput(const orxSTRING _zInput, orxU32 _u32ID) const;

                void            DumpGrid(orxU32 _u32ID);

private:

                orxSTATUS       Bootstrap() const;

                void            Update(const orxCLOCK_INFO &_rstInfo);
                void            UpdateGame(const orxCLOCK_INFO &_rstInfo, orxU32 _u32ID);

                orxSTATUS       Init();
                orxSTATUS       Run();
                void            Exit();
                void            BindObjects();

  static        orxSTATUS orxFASTCALL EventHandler(const orxEVENT *_pstEvent);


private:
  static const  orxU32          su32MaxPlayer = 2;
                orxVECTOR       mvBlockSize;
                GameState       meGameState;
                orxS32          ms32GridWidth, ms32GridHeight;

                struct Game {
                  orxFLOAT        fTime;
                  orxU64         *au64Grid;
                  Tetro          *poPreview;
                  Tetro          *poSelection;
                  orxFLOAT        fFallTime;
                  orxFLOAT        fLeftTime, fRightTime;
                }mastGames[su32MaxPlayer];
};

#endif // __LD44_H_
