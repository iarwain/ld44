#ifndef __TETRO_H__
#define __TETRO_H__

//! Includes
#include "Scroll.h"

class Tetro : public ScrollObject
{
private:
  void                    OnCreate();
  void                    OnDelete();
  void                    Update(const orxCLOCK_INFO &_rstInfo);

  orxBOOL                 IsValid() const;
  void                    Validate();
  void                    Transform(const orxVECTOR &_rvPos, orxS32 _s32Rotation);

public:
  void                    Land();
  orxBOOL                 Move(const orxVECTOR &_rvPos, orxS32 _s32Rotation);

  orxS32                  s32Rotation;
};

#endif // __TETRO_H__
