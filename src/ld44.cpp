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
  orxFLOAT  fFallDelay;
  orxVECTOR vOffset;
  orxU64    u64GUID;
  orxS32    s32Width, s32Height;
  orxS32    s32ClearedLines = 0;

  // Updates in game time
  mfTime += _rstInfo.fDT;

  // Pushes game config section
  orxConfig_PushSection("Game");

  // No selection?
  if(!mpoSelection)
  {
    // Creates a new tetromino
    mpoSelection = CreateObject<Tetro>(orxConfig_GetString("TetroList"));

    // Moves it
    if(!mpoSelection->Move(orxVECTOR_0, 0))
    {
      //! TODO: Game over
      meGameState = GameStateEnd;
    }

    // Updates fall time
    mfFallTime = GetTime();
  }

  // Gets fall delay
  fFallDelay = orxConfig_GetListFloat("FallDelayList", orxInput_IsActive("SpeedUp") ? 1 : 0);

  // Left?
  if(orxInput_IsActive("MoveLeft"))
  {
    // Updates delay
    mfLeftTime -= _rstInfo.fDT;

    // Can move?
    if(mfLeftTime <= orxFLOAT_0)
    {
      orxVECTOR vMove;

      // Moves selection left
      orxVector_Set(&vMove, -orxFLOAT_1, orxFLOAT_0, orxFLOAT_0);

      // Moves it
      mpoSelection->Move(vMove, 0);

      // Updates delay
      mfLeftTime = orxConfig_GetListFloat("MoveDelayList", orxInput_HasNewStatus("MoveLeft") ? 0 : 1);
    }

    // Resets right delay
    mfRightTime = orxFLOAT_0;
  }
  else
  {
    // Resets left delay
    mfLeftTime = orxFLOAT_0;

    // Right?
    if(orxInput_IsActive("MoveRight"))
    {
      // Updates delay
      mfRightTime -= _rstInfo.fDT;

      // Can move?
      if(mfRightTime <= orxFLOAT_0)
      {
        orxVECTOR vMove;

        // Moves selection right
        orxVector_Set(&vMove, orxFLOAT_1, orxFLOAT_0, orxFLOAT_0);

        // Moves it
        mpoSelection->Move(vMove, 0);

        // Updates delay
        mfRightTime = orxConfig_GetListFloat("MoveDelayList", orxInput_HasNewStatus("MoveRight") ? 0 : 1);
      }
    }
    else
    {
      // Resets right delay
      mfRightTime = orxFLOAT_0;
    }
  }

  // Rotate?
  if(orxInput_HasBeenActivated("RotateCW") || orxInput_HasBeenActivated("RotateCCW"))
  {
    orxS32 s32Rotation;

    // Gets current rotation
    s32Rotation = mpoSelection->s32Rotation;

    // Moves it
    mpoSelection->Move(orxVECTOR_0, orxInput_IsActive("RotateCW") ? 1 : -1);

    // Changed?
    if(mpoSelection->s32Rotation != s32Rotation)
    {
      // Adds rotate track
      mpoSelection->AddTrack("RotateTrack");
    }
  }

  // Should fall one step?
  if(GetTime() - mfFallTime >= fFallDelay)
  {
    orxVECTOR vMove;

    // Moves selection down
    orxVector_Set(&vMove, orxFLOAT_0, orxFLOAT_1, orxFLOAT_0);

    // Moves it
    if(!mpoSelection->Move(vMove, 0))
    {
      // Land!
      mpoSelection->Land();
      mpoSelection = orxNULL;
    }

    // Updates fall time
    mfFallTime = GetTime();
  }

  // Gets grid size
  GetGridSize(s32Width, s32Height);

  // Gets block size
  orxConfig_GetVector("BlockSize", &vOffset);

  // Gets selection GUID
  u64GUID = mpoSelection ? mpoSelection->GetGUID() : 0;

  // For all lines
  for(orxS32 i = s32Height - 1; i >= 0; i--)
  {
    orxBOOL bClear = orxTRUE;

    // For all columns
    for(orxS32 j = 0; j < s32Width; j++)
    {
      orxU64 u64BlockID;

      // Gets block ID
      u64BlockID = GetGridValue(j, i);

      // Empty or selection?
      if((u64BlockID == 0) || (u64BlockID == u64GUID))
      {
        // Don't clear line
        bClear = orxFALSE;

        break;
      }
    }

    // Should clear line?
    if(bClear)
    {
      // Updates line cleared counter
      s32ClearedLines++;

      // Clears grid line
      ClearGridLine(i);

      // For all blocks
      for(Block *poBlock = GetNextObject<Block>();
          poBlock;
          poBlock = GetNextObject<Block>(poBlock))
      {
        orxVECTOR vPos;
        orxS32 s32X, s32Y;

        // Gets its world position
        poBlock->GetPosition(vPos, orxTRUE);

        // Gets its grid position
        if(GetGridPosition(vPos, s32X, s32Y) != orxSTATUS_FAILURE)
        {
          // Is on cleared line?
          if(s32Y == i)
          {
            // Asks for deletion
            poBlock->SetLifeTime(orxFLOAT_0);
          }
          // Is above it?
          else if(s32Y < i)
          {
            // Moves it downward
            vPos.fY += vOffset.fY;
            poBlock->SetPosition(vPos, orxTRUE);
          }
        }
      }

      // Updates line index due to cleared line
      i++;
    }
  }

  // Any line cleared?
  if(s32ClearedLines > 0)
  {
    // Adds clear track
    mpoScene->AddTrack("LineClearTrack");
  }

  // Pops config section
  orxConfig_PopSection();
}

orxSTATUS LD44::Init()
{
  orxVECTOR vGridSize;

  orxConfig_PushSection("Game");

  // Inits random
  orxMath_InitRandom((orxU32)orxSystem_GetRealTime());

  // Creates the viewports
  for(orxS32 i = 0; i < orxConfig_GetListCount("ViewportList"); i++)
  {
    orxViewport_CreateFromConfig(orxConfig_GetListString("ViewportList", i));
  }

  // Creates the scene
  mpoScene = CreateObject("Scene");

  // Creates the grid
  orxConfig_GetVector("GridSize", &vGridSize);
  ms32GridWidth  = orxF2S(vGridSize.fX);
  ms32GridHeight = orxF2S(vGridSize.fY);
  mau64Grid = (orxU64 *)orxMemory_Allocate(ms32GridWidth * ms32GridHeight * sizeof(orxU64), orxMEMORY_TYPE_MAIN);
  orxMemory_Zero(mau64Grid, ms32GridWidth * ms32GridHeight * sizeof(orxU64));

  // Inits variables
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
  ScrollBindObject<Block>("TetroBlock");
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
  orxVector_Div(&vPos, &_rvPos, &vBlockSize);

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

void LD44::ClearGridLine(orxS32 _s32Line)
{
  // For all lines above cleared one
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
