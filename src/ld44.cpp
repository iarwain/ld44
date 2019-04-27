/**
 * @file LD44.cpp
 * @date 26-Apr-2019
 */
#define __SCROLL_IMPL__
#include "ld44.h"
#undef __SCROLL_IMPL__


orxSTATUS LD44::Bootstrap() const
{
  // Add a config storage to find the initial config file
  orxResource_AddStorage(orxCONFIG_KZ_RESOURCE_GROUP, "../data/config", orxFALSE);

  // Done!
  return orxSTATUS_SUCCESS;
}

void LD44::Update(const orxCLOCK_INFO &_rstInfo)
{
  // Screenshot?
  if(orxInput_HasBeenActivated("Screenshot"))
  {
    orxScreenshot_Capture();
  }

  // Depending on game state
  switch(meGameState)
  {
    case GameStateSplash:
    {
      meGameState = GameStateMenu;
      break;
    }

    case GameStateMenu:
    {
      mfTime = orxFLOAT_0;
      meGameState = GameStateRun;
      break;
    }

    case GameStateRun:
    {
      // Updates in-game
      UpdateGame(_rstInfo);
      break;
    }

    case GameStatePause:
    {
      break;
    }

    case GameStateEnd:
    {
      // Adds game over screen
      CreateObject("GameOver");
      meGameState = GameStateNumber;
      break;
    }
  }
}

void LD44::UpdateGame(const orxCLOCK_INFO &_rstInfo)
{
  // Updates in game time
  mfTime += _rstInfo.fDT;
}

orxSTATUS LD44::Init()
{
  orxVECTOR vGridSize;
  orxConfig_PushSection("Game");

  // Creates the viewports
  for(orxS32 i = 0; i < orxConfig_GetListCount("ViewportList"); i++)
  {
    orxViewport_CreateFromConfig(orxConfig_GetListString("ViewportList", i));
  }

  // Creates the scene
  orxObject_CreateFromConfig("Scene");

  // Creates the grid
  orxConfig_GetVector("GridSize", &vGridSize);
  ms32GridWidth  = orxF2S(vGridSize.fX);
  ms32GridHeight = orxF2S(vGridSize.fY);
  mau64Grid = (orxU64 *)orxMemory_Allocate(ms32GridWidth * ms32GridHeight * sizeof(orxU64), orxMEMORY_TYPE_MAIN);
  orxMemory_Zero(mau64Grid, ms32GridWidth * ms32GridHeight * sizeof(orxU64));

  // Inits variables
  ms32GridWidth = ms32GridHeight = 0;
  mfTime = mfFallTime = mfLeftTime = mfRightTime = orxFLOAT_0;

  orxConfig_PopSection();

  // Done!
  return orxSTATUS_SUCCESS;
}

orxSTATUS LD44::Run()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Should quit?
  if(orxInput_IsActive("Quit"))
  {
    // Updates result
    eResult = orxSTATUS_FAILURE;
  }

  // Done!
  return eResult;
}

void LD44::Exit()
{
  // Deletes grid
  orxMemory_Free(mau64Grid);
  mau64Grid = orxNULL;
}

void LD44::BindObjects()
{
  // Binds objects
  ScrollBindObject<Tetro>("Tetro");
}

orxSTATUS LD44::GetGridPosition(const orxVECTOR &_rvPos, orxS32 &_rs32X, orxS32 &_rs32Y) const
{
  orxVECTOR vBlockSize, vPos;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  // Push T config section
  orxConfig_PushSection("Game");

  // Gets block size
  orxConfig_GetVector("BlockSize", &vBlockSize);

  // Normalizes it
  orxVector_Div(&vPos, &vPos, &vBlockSize);

  // Stores coords
  _rs32X = orxF2S(orxMath_Floor(vPos.fX));
  _rs32Y = orxF2S(orxMath_Floor(vPos.fY));

  // Updates result
  eResult = ((_rs32X >= 0) && (_rs32X < ms32GridWidth) && (_rs32Y >= 0) && (_rs32Y < ms32GridHeight)) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

  // Pops config section
  orxConfig_PopSection();

  // Done!
  return eResult;
}

orxU64 LD44::GetGridValue(orxS32 _s32X, orxS32 _s32Y) const
{
  return mau64Grid[_s32X + _s32Y * ms32GridWidth];
}

void LD44::SetGridValue(orxS32 _s32X, orxS32 _s32Y, orxU64 _u64Value)
{
  mau64Grid[_s32X + _s32Y * ms32GridWidth] = _u64Value;
}

void LD44::GetGridSize(orxS32 &_rs32Width, orxS32 &_rs32Height) const
{
  _rs32Width  = ms32GridWidth;
  _rs32Height = ms32GridHeight;
}

void LD44::CleanGridLine(orxS32 _s32Line)
{
  // For all lines above cleaned one
  for(orxS32 i = _s32Line; i > 0; i--)
  {
    // For all columns
    for(orxS32 j = 0; j < ms32GridWidth; j++)
    {
      // Copies value from above
      SetGridValue(j, i, GetGridValue(j, i - 1));
    }
  }

  // For all columns (top line)
  for(orxS32 j = 0; j < ms32GridWidth; j++)
  {
    // Clears it
    SetGridValue(j, 0, 0);
  }
}

int main(int argc, char **argv)
{
  // Execute our game
  LD44::GetInstance().Execute(argc, argv);

  // Done!
  return EXIT_SUCCESS;
}
