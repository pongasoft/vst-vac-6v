#pragma once

#include <map>
#include <vstgui4/vstgui/lib/controls/ccontrol.h>
#include <vstgui4/vstgui/lib/iviewlistener.h>

namespace pongasoft {
namespace VST {
namespace Common {

using namespace VSTGUI;

using CControlMap = std::map<int32_t, CControl *>;

/**
 * The purpose of this class is to have a quick/easy access to the control/views in the UI (findByTag)
 */
class ControlManager : public IViewListenerAdapter
{
public:
  ControlManager();

  ~ControlManager() override;

  void registerControl(CControl *control);

  template<typename C> C findControl(int32_t tag) { return dynamic_cast<C>(findByTag(tag)); }

  CControl *findByTag(int32_t tag);

  void unregisterControl(int32_t tag);

protected:
  void viewWillDelete(CView *view) override;

private:
  CControlMap fControlMap;
};

}
}
}