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
}

orxSTATUS LD44::Init()
{
  // Creates the viewports
  orxConfig_PushSection("Game");
  for(orxS32 i = 0; i < orxConfig_GetListCount("ViewportList"); i++)
  {
    orxViewport_CreateFromConfig(orxConfig_GetListString("ViewportList", i));
  }
  orxConfig_PopSection();

  // Creates the scene
  orxObject_CreateFromConfig("Scene");

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
}

void LD44::BindObjects()
{
  // Binds objects
  ScrollBindObject<Tetro>("Tetro");
}

int main(int argc, char **argv)
{
  // Execute our game
  LD44::GetInstance().Execute(argc, argv);

  // Done!
  return EXIT_SUCCESS;
}
