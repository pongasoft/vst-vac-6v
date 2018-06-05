#pragma once

#include <pluginterfaces/base/ftypes.h>
#include <pluginterfaces/vst/ivstmessage.h>

namespace pongasoft {
namespace VST {
namespace Common {

using namespace Steinberg;
using namespace Steinberg::Vst;

static const auto ATTR_MSG_ID = "ATTR_MSG_ID";

/**
 * Simple wrapper class with better api
 */
class Message
{
public:
  Message(IMessage *message) : fMessage(message) {}

  inline int getMessageID() const
  {
    return static_cast<int>(getInt(ATTR_MSG_ID, -1));
  }

  inline void setMessageID(int messageID)
  {
    fMessage->getAttributes()->setInt(ATTR_MSG_ID, messageID);
  }

  inline int64 getInt(IAttributeList::AttrID id, int64 defaultValue) const
  {
    int64 value;
    if(fMessage->getAttributes()->getInt(id, value) != kResultOk)
      value = defaultValue;
    return value;
  }

  inline void setInt(IAttributeList::AttrID id, int64 value)
  {
    fMessage->getAttributes()->setInt(id, value);
  }

  inline double getFloat(IAttributeList::AttrID id, double defaultValue) const
  {
    double value;
    if(fMessage->getAttributes()->getFloat(id, value) != kResultOk)
      value = defaultValue;
    return value;
  }

  inline void setFloat(IAttributeList::AttrID id, double value)
  {
    fMessage->getAttributes()->setFloat(id, value);
  }

private:
  IMessage *fMessage;
};

}
}
}