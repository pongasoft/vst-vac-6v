#pragma once

#include <vstgui4/vstgui/lib/iviewlistener.h>
#include <pongasoft/logging/loguru.hpp>

namespace pongasoft {
namespace VST {
namespace GUI {

using namespace VSTGUI;

/**
 * Encapsulates a view while managing its lifecyle: the idea is that an instance of this class is being owned by the
 * main controller (thus its lifespan is the entire life of the controller) but the view assigned to it can come and go
 * (as the user opens/closes the UI of the plugin)
 */
template<typename V>
class GUIViewState : public IViewListenerAdapter
{
public:
  // Constructor
  GUIViewState() : fView{nullptr} {};

  // Called when the view was created and needs to be assigned to this instance
  void assign(V *view) {
    DCHECK_NOTNULL_F(view, "assign should not receive null pointer");

    if(fView != nullptr)
    {
      fView->unregisterViewListener(this);
    }

    fView = view;
    fView->registerViewListener(this);

    afterAssign();
  }

  /**
   * An implementation will typically override this method to initialize the view depending on the state. It is called
   * after assigned as been done (thus fView != nullptr)
   */
  virtual void afterAssign() { }

  /**
   * Called prior to the view being deleted/unassigned in case some cleanup needs to happen */
  virtual void beforeUnassign() { }

protected:
  /**
   * Callback that is called when the host is about to delete the view and as a result it needs to be unassigned from
   * this instance
   */
  void viewWillDelete(CView *view) override {
    DCHECK_EQ_F(view, fView, "should be called with the same object!");

    beforeUnassign();

    fView->unregisterViewListener(this);
    fView = nullptr;

  };

  V* fView;
};


}
}
}