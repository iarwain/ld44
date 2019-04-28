//! Includes
#include "ld44.h"

//! Code
void Tetro::OnCreate()
{
  orxVECTOR vOffset, vPos;

  // Sets as current tetromino
  orxConfig_PushSection("Runtime");
  orxConfig_SetString("Tetro", GetModelName());
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

    // Sets as owner/parent
    orxObject_SetOwner(poBlock->GetOrxObject(), GetOrxObject());
    orxObject_SetParent(poBlock->GetOrxObject(), GetOrxObject());

    // Gets its position
    orxConfig_GetListVector("TetroPosList", i, &vPos);
    orxVector_Mul(&vPos, &vPos, &vOffset);

    // Updates its position
    poBlock->SetPosition(vPos);
  }

  // Sets rotation
  s32Rotation = orxConfig_GetS32("InitRotation");

  // Moves
  Move(orxVECTOR_0, 0);
}

void Tetro::Update(const orxCLOCK_INFO &_rstInfo)
{
}

void Tetro::OnDelete()
{
}

void Tetro::Land()
{
  ScrollObject *poChild;

  // Removes all children
  while(poChild = GetOwnedChild())
  {
    orxObject_SetOwner(poChild->GetOrxObject(), orxNULL);
  }

  // Adds land track
  AddTrack("LandTrack");
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
  s32Rotation += _s32Rotation;
  while(s32Rotation < 0)
  {
    s32Rotation += orxConfig_GetListCount("RotationList");
  }
  s32Rotation = s32Rotation % orxConfig_GetListCount("RotationList");
  SetRotation(orxConfig_GetListFloat("RotationList", s32Rotation) * orxMATH_KF_DEG_TO_RAD);

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
      u64Value = LD44::GetInstance().GetGridValue(s32X, s32Y);

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
      if(LD44::GetInstance().GetGridValue(j, i) == u64GUID)
      {
        // Clears it
        LD44::GetInstance().SetGridValue(j, i, 0);
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
      LD44::GetInstance().SetGridValue(s32X, s32Y, GetGUID());
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
