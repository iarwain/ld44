//! Includes
#include "ld44.h"

//! Code
void Tetro::OnCreate()
{
  orxVECTOR vOffset, vPos;
  orxCOLOR stColor;

  // Sets our start pos
  orxVector_Mul(&vPos, orxConfig_GetVector("SpawnOffset", &vPos), orxConfig_GetVector("BlockSize", &vOffset));
  SetPosition(vPos);

  // For all blocks
  for(orxS32 i = 0; i < orxConfig_GetListCount("TetroPosList"); i++)
  {
    orxVECTOR     vPos;
    ScrollObject *poBlock;

    // Creates it
    orxConfig_SetParent("TetroBlock", GetModelName());
    poBlock = LD44::GetInstance().CreateObject("TetroBlock");

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
  s32Rotation = orxConfig_GetS32("RotationList");

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
  orxOBJECT *pstChild;

  // Removes all children
  while((pstChild = orxObject_GetOwnedChild(GetOrxObject())) != orxNULL)
  {
    orxObject_SetOwner(pstChild, orxNULL);
  }

  // Adds land track
  orxObject_AddTimeLineTrack(GetOrxObject(), "TetrominoLand");
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

orxBOOL Tetro::IsValid(const orxVECTOR &_rvPos, orxS32 _s32Rotation) const
{
  orxVECTOR vPos;
  orxU64    u64GUID;
  orxS32    i = 0;
  orxBOOL   bResult = orxTRUE;

  // Gets our GUID
  u64GUID = GetGUID();

  // Transforms
  const_cast<Tetro *>(this)->Transform(_rvPos, _s32Rotation);

  // For all children
  for(orxOBJECT *pstChild = orxObject_GetOwnedChild(GetOrxObject());
      pstChild != orxNULL;
      pstChild = orxObject_GetOwnedSibling(pstChild), i++)
  {
    orxVECTOR vPos;
    orxS32    s32X, s32Y;

    // Gets its position
    orxObject_GetWorldPosition(pstChild, &vPos);

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

  // Transforms back
  const_cast<Tetro *>(this)->Transform(*orxVector_Mulf(&vPos, &_rvPos, -orxFLOAT_1), -_s32Rotation);

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
  for(orxOBJECT *pstChild = orxObject_GetOwnedChild(GetOrxObject());
      pstChild != orxNULL;
      pstChild = orxObject_GetOwnedSibling(pstChild))
  {
    orxVECTOR vPos;
    orxS32    s32X, s32Y;

    // Gets its position
    orxObject_GetWorldPosition(pstChild, &vPos);

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

  // Valid?
  if((bResult = IsValid(_rvPos, _s32Rotation)) != orxFALSE)
  {
    // Transforms
    Transform(_rvPos, _s32Rotation);

    // Validate
    Validate();
  }

  // Done!
  return bResult;
}
