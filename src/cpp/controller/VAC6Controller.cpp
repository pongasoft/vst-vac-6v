#include <vstgui4/vstgui/plugin-bindings/vst3editor.h>
#include <pongasoft/logging/loguru.hpp>
#include "VAC6Controller.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

///////////////////////////////////////////
// VAC6Controller::VAC6Controller
///////////////////////////////////////////
VAC6Controller::VAC6Controller() : GUIController("VAC6.uidesc"),
                                   fParameters{},
                                   fState{fParameters},
                                   fHistoryState{std::make_shared<HistoryState>()}
{
  DLOG_F(INFO, "VAC6Controller::VAC6Controller()");

  registerViewState(fHistoryState);
}

///////////////////////////////////////////
// VAC6Controller::~VAC6Controller
///////////////////////////////////////////
VAC6Controller::~VAC6Controller()
{
  DLOG_F(INFO, "VAC6Controller::~VAC6Controller()");
}

///////////////////////////////////////////
// VAC6Controller::verifyView
///////////////////////////////////////////
CView *VAC6Controller::verifyView(CView *view,
                                  const UIAttributes &attributes,
                                  const IUIDescription * /*description*/,
                                  VST3Editor * /*editor*/)
{
  auto historyView = dynamic_cast<HistoryView *>(view);
  if(historyView)
  {
    historyView->setHistoryState(fHistoryState);
  }

  return view;
}

///////////////////////////////////
// VAC6Controller::notify
///////////////////////////////////
tresult VAC6Controller::notify(IMessage *message)
{
  if(!message)
    return kInvalidArgument;

  Message m{message};

  switch(static_cast<EVAC6MessageID>(m.getMessageID()))
  {
    case kLCDData_MID:
    {
      fHistoryState->onMessage(m);
      break;
    }

    default:
      DLOG_F(WARNING, "VAC6Controller::notify / unhandled message id %d", m.getMessageID());
      return kResultFalse;
  }

  return kResultOk;
}

}
}
}