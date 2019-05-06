/**
 * @file LD44.cpp
 * @date 26-Apr-2019
 */
#define __SCROLL_IMPL__
#include "ld44.h"
#undef __SCROLL_IMPL__

static orxBOOL sbRestart    = orxTRUE;
static orxBOOL sbDecoration = orxFALSE;
static orxBOOL sbSplash     = orxTRUE;

orxSTATUS LD44::Bootstrap() const
{
  orxResource_AddStorage(orxCONFIG_KZ_RESOURCE_GROUP, "data/config", orxFALSE);
  orxResource_AddStorage(orxCONFIG_KZ_RESOURCE_GROUP, "../data/config", orxFALSE);

  // Updates decoration
  orxConfig_PushSection("Display");
  orxConfig_SetBool("Decoration", sbDecoration);
  orxConfig_PopSection();

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
    case GameStateStart:
    {
      meGameState = GameStateSplash;
      if(sbSplash)
      {
        // Only once
        sbSplash = orxFALSE;
        CreateObject("Splash");
        break;
      }
      else
      {
        // Skips to menu
        orxInput_SetValue("Menu", orxFLOAT_1);

        // Falls through
      }
    }

    case GameStateSplash:
    {
      if(orxInput_IsActive("Menu"))
      {
        CreateObject("Menu");
        meGameState = GameStateMenu;
      }
      break;
    }

    case GameStateMenu:
    {
      orxConfig_PushSection("Runtime");
      if(orxInput_HasBeenActivated("1PStart"))
      {
        // Creates the scene
        CreateObject("Scene");
        orxConfig_SetU32("PlayerCount", 1);
        orxConfig_SetU32("GameID", 0);
        orxConfig_PushSection("Game");
        mastGames[0].poPreview = CreateObject<Tetro>(orxConfig_GetString("TetroList"));
        orxConfig_PopSection();
        meGameState = GameStateRun;
      }
      else if(orxInput_HasBeenActivated("2PStart"))
      {
        // Creates the scene
        CreateObject("DualScene");
        orxConfig_SetU32("PlayerCount", 2);
        orxConfig_SetU32("GameID", 0);
        orxConfig_PushSection("Game");
        mastGames[0].poPreview = CreateObject<Tetro>(orxConfig_GetString("TetroList"));
        orxConfig_PopSection();
        orxConfig_SetU32("GameID", 1);
        orxConfig_PushSection("Game");
        mastGames[1].poPreview = CreateObject<Tetro>(orxConfig_GetString("TetroList"));
        orxConfig_PopSection();
        meGameState = GameStateRun;
      }
      orxConfig_PopSection();
      break;
    }

    case GameStateRun:
    {
      // Updates in-game
      orxConfig_PushSection("Runtime");
      for(orxU32 i = 0, iCount = orxConfig_GetU32("PlayerCount"); i < iCount; i++)
      {
        if(!UpdateGame(_rstInfo, i))
        {
          break;
        }
      }
      orxConfig_PopSection();
      break;
    }

    case GameStatePause:
    {
      break;
    }

    case GameStateEnd:
    {
      orxU32 u32ID;

      // Adds game over screen
      orxConfig_PushSection("Runtime");
      u32ID = orxConfig_GetU32("GameID");
      orxConfig_PopSection();
      orxConfig_PushSection("Game");
      CreateObject(orxConfig_GetListString("GameOverList", u32ID));
      orxConfig_PopSection();
      meGameState = GameStateGameOver;
      break;
    }

    case GameStateGameOver:
    {
      // Reset?
      if(orxInput_IsActive("Reset"))
      {
        // Restarts
        Exit();
        Init();
      }

      break;
    }
  }
}

orxBOOL LD44::UpdateGame(const orxCLOCK_INFO &_rstInfo, orxU32 _u32ID)
{
  orxFLOAT  fFallDelay;
  orxU64    u64GUID;
  orxS32    s32Width, s32Height;
  orxS32    s32ClearedLines = 0;

  // Updates in game time
  mastGames[_u32ID].fTime += _rstInfo.fDT;

  // Stores game ID
  orxConfig_PushSection("Runtime");
  orxConfig_SetU32("GameID", _u32ID);
  orxConfig_PopSection();

  // Pushes game config section
  orxConfig_PushSection("Game");

  // No selection?
  if(!mastGames[_u32ID].poSelection)
  {
    // Gets preview tetromino
    mastGames[_u32ID].poSelection = mastGames[_u32ID].poPreview;

    // Activates it
    mastGames[_u32ID].poSelection->Activate();

    // Updates fall time
    mastGames[_u32ID].fFallTime = GetTime(_u32ID);

    // Moves it
    if(!mastGames[_u32ID].poSelection->Move(orxVECTOR_0, 0))
    {
      meGameState = GameStateEnd;
      return orxFALSE;
    }

    // Creates new preview tetromino
    mastGames[_u32ID].poPreview = CreateObject<Tetro>(orxConfig_GetString("TetroList"));
  }

  // Gets fall delay
  fFallDelay = orxConfig_GetListFloat("FallDelayList", ((orxObject_GetActiveTime(mastGames[_u32ID].poPreview->GetOrxObject()) >= orxConfig_GetFloat("InitialFallDelay")) && orxInput_IsActive(GetGameInput("SpeedUp", _u32ID))) ? 1 : 0);

  // Left?
  if(orxInput_IsActive(GetGameInput("MoveLeft", _u32ID)))
  {
    // Updates delay
    mastGames[_u32ID].fLeftTime -= _rstInfo.fDT;

    // Can move?
    if(mastGames[_u32ID].fLeftTime <= orxFLOAT_0)
    {
      orxVECTOR vMove;

      // Moves selection left
      orxVector_Set(&vMove, -orxFLOAT_1, orxFLOAT_0, orxFLOAT_0);

      // Moves it
      if(mastGames[_u32ID].poSelection->Move(vMove, 0))
      {
        // Creates move event object
        CreateObject("MoveEvent");
      }

      // Updates delay
      mastGames[_u32ID].fLeftTime = orxConfig_GetListFloat("MoveDelayList", orxInput_HasNewStatus(GetGameInput("MoveLeft", _u32ID)) ? 0 : 1);
    }

    // Resets right delay
    mastGames[_u32ID].fRightTime = orxFLOAT_0;
  }
  else
  {
    // Resets left delay
    mastGames[_u32ID].fLeftTime = orxFLOAT_0;

    // Right?
    if(orxInput_IsActive(GetGameInput("MoveRight", _u32ID)))
    {
      // Updates delay
      mastGames[_u32ID].fRightTime -= _rstInfo.fDT;

      // Can move?
      if(mastGames[_u32ID].fRightTime <= orxFLOAT_0)
      {
        orxVECTOR vMove;

        // Moves selection right
        orxVector_Set(&vMove, orxFLOAT_1, orxFLOAT_0, orxFLOAT_0);

        // Moves it
        if(mastGames[_u32ID].poSelection->Move(vMove, 0))
        {
          // Creates move event object
          CreateObject("MoveEvent");
        }

        // Updates delay
        mastGames[_u32ID].fRightTime = orxConfig_GetListFloat("MoveDelayList", orxInput_HasNewStatus(GetGameInput("MoveRight", _u32ID)) ? 0 : 1);
      }
    }
    else
    {
      // Resets right delay
      mastGames[_u32ID].fRightTime = orxFLOAT_0;
    }
  }

  // Rotate?
  if(orxInput_HasBeenActivated(GetGameInput("RotateCW", _u32ID)) || orxInput_HasBeenActivated(GetGameInput("RotateCCW", _u32ID)))
  {
    orxS32 s32Rotation;

    // Gets current rotation
    s32Rotation = mastGames[_u32ID].poSelection->ms32Rotation;

    // Moves it
    if(mastGames[_u32ID].poSelection->Move(orxVECTOR_0, orxInput_IsActive(GetGameInput("RotateCW", _u32ID)) ? 1 : -1)
    || mastGames[_u32ID].poSelection->Move((orxVECTOR){-orxFLOAT_1, orxFLOAT_0, orxFLOAT_0}, orxInput_IsActive(GetGameInput("RotateCW", _u32ID)) ? 1 : -1)
    || mastGames[_u32ID].poSelection->Move((orxVECTOR){orx2F(-2.0), orxFLOAT_0, orxFLOAT_0}, orxInput_IsActive(GetGameInput("RotateCW", _u32ID)) ? 1 : -1)
    || mastGames[_u32ID].poSelection->Move((orxVECTOR){orxFLOAT_1, orxFLOAT_0, orxFLOAT_0}, orxInput_IsActive(GetGameInput("RotateCW", _u32ID)) ? 1 : -1)
    || mastGames[_u32ID].poSelection->Move((orxVECTOR){orx2F(2.0), orxFLOAT_0, orxFLOAT_0}, orxInput_IsActive(GetGameInput("RotateCW", _u32ID)) ? 1 : -1))
    {
      // Creates rotate event object
      CreateObject("RotateEvent");

      // Adds line
      if(AddLine(ms32GridHeight - 1, _u32ID))
      {
        // Creates add line event object
        CreateObject("AddLineEvent");
      }
      else
      {
        meGameState = GameStateEnd;
        return orxFALSE;
      }
    }
  }

  // Should fall one step?
  if(GetTime(_u32ID) - mastGames[_u32ID].fFallTime >= fFallDelay)
  {
    orxVECTOR vMove;

    // Moves selection down
    orxVector_Set(&vMove, orxFLOAT_0, orxFLOAT_1, orxFLOAT_0);

    // Moves it
    if(!mastGames[_u32ID].poSelection->Move(vMove, 0))
    {
      // Land!
      mastGames[_u32ID].poSelection->Land();
      mastGames[_u32ID].poSelection = orxNULL;
    }

    // Updates fall time
    mastGames[_u32ID].fFallTime = GetTime(_u32ID);
  }

  // Gets grid size
  GetGridSize(s32Width, s32Height);

  // Gets selection GUID
  u64GUID = mastGames[_u32ID].poSelection ? mastGames[_u32ID].poSelection->GetGUID() : 0;

  // For all lines
  for(orxS32 i = s32Height - 1; i >= 0; i--)
  {
    orxBOOL bClear = orxTRUE;

    // For all columns
    for(orxS32 j = 0; j < s32Width; j++)
    {
      orxU64 u64BlockID;

      // Gets block ID
      u64BlockID = GetGridValue(j, i, _u32ID);

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
      ClearLine(i, _u32ID);

      // Updates line index due to cleared line
      i++;
    }
  }

  // Any line cleared?
  if(s32ClearedLines > 0)
  {
    const orxSTRING zScore;
    orxS32          s32ClearLineScore;

    // Updates config
    orxConfig_PushSection("Game");
    zScore = orxConfig_GetListString("ScoreList", _u32ID);
    s32ClearLineScore = orxConfig_GetS32("ClearLineScore");
    orxConfig_PopSection();
    orxConfig_PushSection("Runtime");
    orxConfig_SetS32("ClearedLines", s32ClearedLines);
    orxConfig_SetS32(zScore, orxConfig_GetS32(zScore) + s32ClearLineScore * s32ClearedLines);
    orxConfig_PopSection();

    // Creates clear line event object
    CreateObject("ClearLineEvent");
  }

  // Pops config section
  orxConfig_PopSection();

  // Done!
  return orxTRUE;
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

  // Gets grid info
  orxConfig_GetVector("GridSize", &vGridSize);
  ms32GridWidth  = orxF2S(vGridSize.fX);
  ms32GridHeight = orxF2S(vGridSize.fY);

  // Gets block size
  orxConfig_GetVector("BlockSize", &mvBlockSize);

  // For all games
  for(orxU32 i = 0; i < su32MaxPlayer; i++)
  {
    // Creates the grid
    mastGames[i].au64Grid = (orxU64 *)orxMemory_Allocate(ms32GridWidth * ms32GridHeight * sizeof(orxU64), orxMEMORY_TYPE_MAIN);
    orxMemory_Zero(mastGames[i].au64Grid, ms32GridWidth * ms32GridHeight * sizeof(orxU64));

    // Inits variables
    mastGames[i].poPreview = mastGames[i].poSelection = orxNULL;
    mastGames[i].fTime = mastGames[i].fFallTime = mastGames[i].fLeftTime = mastGames[i].fRightTime = orxFLOAT_0;
  }

  // Clears runtime section
  orxConfig_ClearSection("Runtime");
  orxConfig_PushSection("Runtime");
  orxConfig_SetU32("Score1", 0);
  orxConfig_SetU32("Score2", 0);
  orxConfig_PopSection();

  // Inits state
  meGameState = GameStateStart;

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
    // Updates restart status
    sbRestart = orxInput_HasBeenActivated("Reset");

    // Toggles decoration
    sbDecoration = !sbDecoration;

    // Updates result
    eResult = orxSTATUS_FAILURE;
  }

  // Done!
  return eResult;
}

void LD44::Exit()
{
  // Deletes all objects
  for(ScrollObject *poObject = GetNextObject();
      poObject;
      poObject = GetNextObject())
  {
    DeleteObject(poObject);
  }

  // For all games
  for(orxU32 i = 0; i < su32MaxPlayer; i++)
  {
    // Deletes grid
    orxMemory_Free(mastGames[i].au64Grid);
    mastGames[i].au64Grid = orxNULL;
  }
}

void LD44::BindObjects()
{
  // Binds objects
  ScrollBindObject<Tetro>("Tetro");
  ScrollBindObject<Block>("TetroBlock");
}

void LD44::GetGridSize(orxS32 &_rs32Width, orxS32 &_rs32Height) const
{
  _rs32Width  = ms32GridWidth;
  _rs32Height = ms32GridHeight;
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

orxU64 LD44::GetGridValue(orxS32 _s32X, orxS32 _s32Y, orxU32 _u32ID) const
{
  return mastGames[_u32ID].au64Grid[_s32X + _s32Y * ms32GridWidth];
}

void LD44::SetGridValue(orxS32 _s32X, orxS32 _s32Y, orxU64 _u64Value, orxU32 _u32ID)
{
  mastGames[_u32ID].au64Grid[_s32X + _s32Y * ms32GridWidth] = _u64Value;
}

void LD44::ClearLine(orxS32 _s32Line, orxU32 _u32ID)
{
  // For all lines at or above cleared one
  for(orxS32 i = _s32Line; i > 0; i--)
  {
    // For all columns
    for(orxS32 j = 0; j < ms32GridWidth; j++)
    {
      // Copies value from above
      SetGridValue(j, i, GetGridValue(j, i - 1, _u32ID), _u32ID);
    }
  }

  // For all columns (top line)
  for(orxS32 j = 0; j < ms32GridWidth; j++)
  {
    // Clears it
    SetGridValue(j, 0, 0, _u32ID);
  }

  // For all blocks
  for(Block *poBlock = GetNextObject<Block>();
      poBlock;
      poBlock = GetNextObject<Block>(poBlock))
  {
    // Same ID?
    if(poBlock->mu32ID == _u32ID)
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
}

void LD44::DumpGrid(orxU32 _u32ID)
{
  orxLOG("=== Dump Begin ===");
  for(orxS32 i = 0; i < ms32GridHeight; i++)
  {
    orxLOG("%d %d %d %d %d %d %d %d %d %d", GetGridValue(0, i, _u32ID) != 0, GetGridValue(1, i, _u32ID) != 0, GetGridValue(2, i, _u32ID) != 0, GetGridValue(3, i, _u32ID) != 0, GetGridValue(4, i, _u32ID) != 0, GetGridValue(5, i, _u32ID) != 0, GetGridValue(6, i, _u32ID) != 0, GetGridValue(7, i, _u32ID) != 0, GetGridValue(8, i, _u32ID) != 0, GetGridValue(9, i, _u32ID) != 0);
  }
  orxLOG("=== Dump End ===");
}

orxBOOL LD44::AddLine(orxS32 _s32Line, orxU32 _u32ID)
{
  orxU64  u64GUID;
  orxS32  s32Hole;
  orxU32  u32GroupID;
  orxBOOL bResult = orxTRUE;

  // Gets selection GUID
  u64GUID = mastGames[_u32ID].poSelection ? mastGames[_u32ID].poSelection->GetGUID() : 0;

  // For all columns
  for(orxS32 i = 0; i < ms32GridWidth; i++)
  {
    orxU64 u64BlockID;

    // Gets its block ID on top line
    u64BlockID = GetGridValue(i, 0, _u32ID);

    // Is a block?
    if((u64BlockID != 0) && (u64BlockID != u64GUID))
    {
      // Stops
      return orxFALSE;
    }
  }

  // For all lines above added one
  for(orxS32 i = 0; i < _s32Line; i++)
  {
    // For all columns
    for(orxS32 j = 0; j < ms32GridWidth; j++)
    {
      // Moves block one line up
      SetGridValue(j, i, GetGridValue(j, i + 1, _u32ID), _u32ID);
    }
  }

  // For all blocks
  for(Block *poBlock = GetNextObject<Block>();
      poBlock;
      poBlock = GetNextObject<Block>(poBlock))
  {
    // Same ID?
    if(poBlock->mu32ID == _u32ID)
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
  }

  // Gets hole columns
  s32Hole = orxMath_GetRandomS32(0, ms32GridWidth - 1);

  // Sets dead tetro as current
  orxConfig_PushSection("Runtime");
  orxConfig_SetString("Tetro", "TetroDead");
  orxConfig_SetU32("GameID", _u32ID);
  orxConfig_PopSection();
  orxConfig_PushSection("Game");
  u32GroupID = orxString_ToCRC(orxConfig_GetListString("GroupList", _u32ID));
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
      poBlock->SetGroupID(u32GroupID);
      poBlock->mu32ID = _u32ID;
      SetGridValue(j, _s32Line, poBlock->GetGUID(), _u32ID);
    }
    else
    {
      // Clears it
      SetGridValue(j, _s32Line, 0, _u32ID);
    }
  }

  // Try to maintain current position for selection
  if(!mastGames[_u32ID].poSelection->Move(orxVECTOR_0, 0))
  {
    orxVECTOR vUp;

    // Try to move it up
    orxVector_Set(&vUp, orxFLOAT_0, -orxFLOAT_1, orxFLOAT_0);
    bResult = mastGames[_u32ID].poSelection->Move(vUp, 0);
  }

  // Done!
  return bResult;
}

const orxSTRING LD44::GetGameInput(const orxSTRING _zInput, orxU32 _u32ID) const
{
  static orxCHAR acBuffer[64] = {};

  // Updates buffer
  orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s%u", _zInput, _u32ID + 1);

  // Done!
  return acBuffer;
}

int main(int argc, char **argv)
{
  // Should restart?
  while(sbRestart)
  {
    // Clears restart
    sbRestart = orxFALSE;

    // Execute our game
    LD44::GetInstance().Execute(argc, argv);
  }

  // Done!
  return EXIT_SUCCESS;
}
