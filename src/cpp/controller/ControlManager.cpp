#include "ControlManager.h"
#include "../logging/loguru.hpp"

namespace pongasoft {
namespace VST {
namespace Common {

using namespace VSTGUI;

///////////////////////////////////////////
// ControlManager::ControlManager
///////////////////////////////////////////
ControlManager::ControlManager() : IViewListenerAdapter()
{
  DLOG_F(INFO, "ControlManager::ControlManager");
}

///////////////////////////////////////////
// ControlManager::~ControlManager
///////////////////////////////////////////
ControlManager::~ControlManager()
{
  DLOG_F(INFO, "ControlManager::~ControlManager");

  for(auto it : fControlMap)
  {
    it.second->unregisterViewListener(this);
  }

  fControlMap.clear();
}

///////////////////////////////////////////
// ControlManager::registerControl
///////////////////////////////////////////
void ControlManager::registerControl(CControl *control)
{
  if(control == nullptr)
    return;

  DLOG_F(INFO, "ControlManager::registerControl(%d)", control->getTag());

  // make sure that it is not registered yet!
  DCHECK_EQ_F(fControlMap.find(control->getTag()), fControlMap.cend());

  // add top map
  fControlMap[control->getTag()] = control;

  // make sure that it gets removed when the control is deleted (by the host)
  control->registerViewListener(this);
}

///////////////////////////////////////////
// ControlManager::unregisterControl
///////////////////////////////////////////
void ControlManager::unregisterControl(int32_t tag)
{
  DLOG_F(INFO, "ControlManager::unregisterControl(%d)", tag);

  CControlMap::const_iterator it = fControlMap.find(tag);
  if(it != fControlMap.cend())
  {
    it->second->unregisterViewListener(this);
    fControlMap.erase(it);
  }
}

///////////////////////////////////////////
// ControlManager::findByTag
///////////////////////////////////////////
CControl *ControlManager::findByTag(int32_t tag)
{
  CControlMap::const_iterator it = fControlMap.find(tag);
  if(it != fControlMap.cend())
    return it->second;
  else
    return nullptr;
}

///////////////////////////////////////////
// ControlManager::viewWillDelete
///////////////////////////////////////////
void ControlManager::viewWillDelete(CView *view)
{
  auto control = dynamic_cast<CControl *>(view);
  if(control != nullptr)
  {
    unregisterControl(control->getTag());
  }
}



}
}
}