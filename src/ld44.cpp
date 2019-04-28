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
      if(mpoSelection->Move(vMove, 0))
      {
        // Adds move track
        mpoSelection->AddTrack("MoveTrack");
      }

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
        if(mpoSelection->Move(vMove, 0))
        {
          // Adds move track
          mpoSelection->AddTrack("MoveTrack");
        }

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

      // Adds line
      if(!AddLine(ms32GridHeight - 1))
      {
        //! TODO: Game over
        meGameState = GameStateEnd;
      }
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

      // Clears line
      ClearLine(i);

      // Updates line index due to cleared line
      i++;
    }
  }

  // Any line cleared?
  if(s32ClearedLines > 0)
  {
    // Updates config
    orxConfig_PushSection("Runtime");
    orxConfig_SetS32("ClearedLines", s32ClearedLines);
    orxConfig_PopSection();

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

  // Gets block size
  orxConfig_GetVector("BlockSize", &mvBlockSize);

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

void LD44::ClearLine(orxS32 _s32Line)
{
  // For all lines at or above cleared one
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
      if(s32Y == _s32Line)
      {
        // Asks for deletion
        poBlock->SetLifeTime(orxFLOAT_0);
      }
      // Is above it?
      else if(s32Y < _s32Line)
      {
        // Moves it downward
        vPos.fY += mvBlockSize.fY;
        poBlock->SetPosition(vPos, orxTRUE);
      }
    }
  }
}

void LD44::DumpGrid()
{
  orxLOG("=== Dump Begin ===");
  for(orxS32 i = 0; i < ms32GridHeight; i++)
  {
    orxLOG("%d %d %d %d %d %d %d %d %d %d", GetGridValue(0, i) != 0, GetGridValue(1, i) != 0, GetGridValue(2, i) != 0, GetGridValue(3, i) != 0, GetGridValue(4, i) != 0, GetGridValue(5, i) != 0, GetGridValue(6, i) != 0, GetGridValue(7, i) != 0, GetGridValue(8, i) != 0, GetGridValue(9, i) != 0);
  }
  orxLOG("=== Dump End ===");
}

orxBOOL LD44::AddLine(orxS32 _s32Line)
{
  orxU64  u64GUID;
  orxS32  s32Hole;
  orxBOOL bResult = orxTRUE;

  // Gets selection GUID
  u64GUID = mpoSelection ? mpoSelection->GetGUID() : 0;

  // For all lines above added one
  for(orxS32 i = 1; i < _s32Line; i++)
  {
    // For all columns
    for(orxS32 j = 0; j < ms32GridWidth; j++)
    {
      // Moves block one line up
      SetGridValue(j, i, GetGridValue(j, i + 1));
    }
  }

  // For all blocks
  for(Block *poBlock = GetNextObject<Block>();
      poBlock;
      poBlock = GetNextObject<Block>(poBlock))
  {
    // No owner?
    if(!orxObject_GetOwner(poBlock->GetOrxObject()))
    {
      orxVECTOR vPos;

      // Updates its position
      poBlock->GetPosition(vPos, orxTRUE);
      vPos.fY -= mvBlockSize.fY;
      poBlock->SetPosition(vPos, orxTRUE);
    }
  }

  // Gets hole columns
  s32Hole = orxMath_GetRandomS32(0, ms32GridWidth - 1);

  // Sets dead tetro as current
  orxConfig_PushSection("Runtime");
  orxConfig_SetString("Tetro", "TetroDead");
  orxConfig_PopSection();

  // For all columns (added line)
  for(orxS32 j = 0; j < ms32GridWidth; j++)
  {
    // Not hole?
    if(j != s32Hole)
    {
      orxVECTOR vPos;
      Block    *poBlock;

      // Computes position
      orxVector_Set(&vPos, (orxS2F(j) + orx2F(0.5f)) * mvBlockSize.fX, (orxS2F(_s32Line) + orx2F(0.5f)) * mvBlockSize.fY, orxFLOAT_0);

      // Adds new block
      poBlock = LD44::GetInstance().CreateObject<Block>("TetroBlock");
      poBlock->SetPosition(vPos, orxTRUE);
      SetGridValue(j, _s32Line, poBlock->GetGUID());
    }
    else
    {
      // Clears it
      SetGridValue(j, _s32Line, 0);
    }
  }

  // Try to maintain current position for selection
  if(!mpoSelection->Move(orxVECTOR_0, 0))
  {
    orxVECTOR vUp;

    // Try to move it up
    orxVector_Set(&vUp, orxFLOAT_0, -orxFLOAT_1, orxFLOAT_0);
    bResult = mpoSelection->Move(vUp, 0);
  }

  // For all columns
  for(orxS32 i = 0; i < ms32GridWidth; i++)
  {
    orxU64 u64BlockID;

    // Gets its block ID on top line
    u64BlockID = GetGridValue(i, 0);

    // Is a block?
    if((u64BlockID != 0) && (u64BlockID != u64GUID))
    {
      // Stops
      bResult = orxFALSE;
      break;
    }
  }

  // Done!
  return bResult;
}

int main(int argc, char **argv)
{
  // Execute our game
  LD44::GetInstance().Execute(argc, argv);

  // Done!
  return EXIT_SUCCESS;
}
