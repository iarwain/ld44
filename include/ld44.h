#ifndef __LD44_H_
#define __LD44_H_

//! Includes
#define __NO_SCROLLED__ // Uncomment this define to prevent the embedded editor (ScrollEd) from being compiled
#include "Scroll.h"


//! LD44
class LD44 : public Scroll<LD44>
{
public:

                ScrollObject *  GetRunTimeObject(const orxSTRING _zObjectName) const;
                void            DeleteRunTimeObject(const orxSTRING _zObjectName);


private:

                orxSTATUS       Bootstrap() const;

                void            Update(const orxCLOCK_INFO &_rstInfo);

                orxSTATUS       Init();
                orxSTATUS       Run();
                void            Exit();
                void            BindObjects();

  static        orxSTATUS orxFASTCALL EventHandler(const orxEVENT *_pstEvent);
};

#endif // __LD44_H_
