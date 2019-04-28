//! Includes
#include "ld44.h"

//! Code
void Tetro::OnCreate()
{
  orxVECTOR vOffset, vPos;
  orxU32    u32GroupID;

  // Sets as current tetromino
  orxConfig_PushSection("Runtime");
  orxConfig_SetString("Tetro", GetModelName());
  mu32ID = orxConfig_GetU32("GameID");
  orxConfig_PopSection();

  // Sets our start pos
  orxVector_Mul(&vPos, orxConfig_GetVector("SpawnOffset", &vPos), orxConfig_GetVector("BlockSize", &vOffset));
  SetPosition(vPos);

  // For all blocks
  for(orxS32 i = 0; i < orxConfig_GetListCount("TetroPosList"); i++)
  {
    orxVECTOR vPos;
    Block    *poBlock;

    // Creates it
    poBlock = LD44::GetInstance().CreateObject<Block>("TetroBlock");
    poBlock->mu32ID = mu32ID;

    // Sets as owner/parent
    orxObject_SetOwner(poBlock->GetOrxObject(), GetOrxObject());
    orxObject_SetParent(poBlock->GetOrxObject(), GetOrxObject());

    // Gets its position
    orxConfig_GetListVector("TetroPosList", i, &vPos);
    orxVector_Mul(&vPos, &vPos, &vOffset);

    // Updates its position
    poBlock->SetPosition(vPos);
  }

  // Sets group
  orxConfig_PushSection("Game");
  u32GroupID = orxString_ToCRC(orxConfig_GetListString("GroupList", mu32ID));
  SetGroupID(u32GroupID);
  orxConfig_PopSection();

  // Sets rotation
  ms32Rotation = orxConfig_GetS32("InitRotation");

  // Applies it
  Transform(orxVECTOR_0, 0);
}

void Tetro::Activate()
{
  orxVECTOR vPos;

  // Resets its depth
  GetPosition(vPos);
  vPos.fZ = orxFLOAT_0;
  SetPosition(vPos);
}

void Tetro::Update(const orxCLOCK_INFO &_rstInfo)
{
}

void Tetro::OnDelete()
{
}

void Tetro::Land()
{
  const orxSTRING zScore;
  ScrollObject   *poChild;
  orxS32          s32LandScore;

  // Removes all children
  while(poChild = GetOwnedChild())
  {
    orxObject_SetOwner(poChild->GetOrxObject(), orxNULL);
  }

  // Updates config
  orxConfig_PushSection("Game");
  zScore = orxConfig_GetListString("ScoreList", mu32ID);
  s32LandScore = orxConfig_GetS32("LandScore");
  orxConfig_PopSection();
  orxConfig_PushSection("Runtime");
  orxConfig_SetS32(zScore, orxConfig_GetS32(zScore) + s32LandScore);
  orxConfig_PopSection();

  // Creates land even object
  LD44::GetInstance().CreateObject("LandEvent");
}

void Tetro::Transform(const orxVECTOR &_rvPos, orxS32 _s32Rotation)
{
  orxVECTOR vPos, vOffset;

  // Pushes config section
  PushConfigSection();

  // Position
  orxConfig_GetVector("BlockSize", &vOffset);
  GetPosition(vPos);
  orxVector_Mul(&vOffset, &vOffset, &_rvPos);
  orxVector_Add(&vPos, &vPos, &vOffset);
  SetPosition(vPos);

  // Rotation
  ms32Rotation += _s32Rotation;
  while(ms32Rotation < 0)
  {
    ms32Rotation += orxConfig_GetListCount("RotationList");
  }
  ms32Rotation = ms32Rotation % orxConfig_GetListCount("RotationList");
  SetRotation(orxConfig_GetListFloat("RotationList", ms32Rotation) * orxMATH_KF_DEG_TO_RAD);

  // Pops config section
  PopConfigSection();
}

orxBOOL Tetro::IsValid() const
{
  orxU64  u64GUID;
  orxS32  i = 0;
  orxBOOL bResult = orxTRUE;

  // Gets our GUID
  u64GUID = GetGUID();

  // For all children
  for(ScrollObject *poChild = GetOwnedChild();
      poChild;
      poChild = poChild->GetOwnedSibling(), i++)
  {
    orxVECTOR vPos;
    orxS32    s32X, s32Y;

    // Gets its position
    poChild->GetPosition(vPos, orxTRUE);

    // Gets its grid index
    if(LD44::GetInstance().GetGridPosition(vPos, s32X, s32Y) != orxSTATUS_FAILURE)
    {
      orxU64 u64Value;

      // Gets its value
      u64Value = LD44::GetInstance().GetGridValue(s32X, s32Y, mu32ID);

      // Is not us and not empty?
      if((u64Value != 0) && (u64Value != u64GUID))
      {
        // Updates result
        bResult = orxFALSE;

        break;
      }
    }
    else
    {
      // Updates result
      bResult = orxFALSE;

      break;
    }
  }

  // Done!
  return bResult;
}

void Tetro::Validate()
{
  orxS32 s32Width, s32Height;
  orxU64 u64GUID;

  // Gets GUID
  u64GUID = GetGUID();

  // Gets grid size
  LD44::GetInstance().GetGridSize(s32Width, s32Height);

  // For all grid elements
  for(orxS32 i = 0; i < s32Height; i++)
  {
    for(orxS32 j = 0; j < s32Width; j++)
    {
      // Is us?
      if(LD44::GetInstance().GetGridValue(j, i, mu32ID) == u64GUID)
      {
        // Clears it
        LD44::GetInstance().SetGridValue(j, i, 0, mu32ID);
      }
    }
  }

  // For all children
  for(ScrollObject *poChild = GetOwnedChild();
      poChild;
      poChild = poChild->GetOwnedSibling())
  {
    orxVECTOR vPos;
    orxS32    s32X, s32Y;

    // Gets its position
    poChild->GetPosition(vPos, orxTRUE);

    // Gets its grid position
    if(LD44::GetInstance().GetGridPosition(vPos, s32X, s32Y) != orxSTATUS_FAILURE)
    {
      // Stores GUID
      LD44::GetInstance().SetGridValue(s32X, s32Y, GetGUID(), mu32ID);
    }
  }
}

orxBOOL Tetro::Move(const orxVECTOR &_rvPos, orxS32 _s32Rotation)
{
  orxBOOL bResult;

  // Transforms
  Transform(_rvPos, _s32Rotation);

  // Valid?
  if(bResult = IsValid())
  {
    // Validate
    Validate();
  }
  else
  {
    orxVECTOR vPos;

    // Transforms back
    Transform(*orxVector_Mulf(&vPos, &_rvPos, -orxFLOAT_1), -_s32Rotation);
  }

  // Done!
  return bResult;
}
