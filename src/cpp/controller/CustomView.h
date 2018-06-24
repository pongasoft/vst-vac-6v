#pragma once

#include <vstgui4/vstgui/uidescription/iviewcreator.h>
#include <vstgui4/vstgui/uidescription/uiviewcreator.h>
#include <vstgui4/vstgui/uidescription/uiviewfactory.h>
#include <vstgui4/vstgui/uidescription/uiattributes.h>
#include <vstgui4/vstgui/uidescription/detail/uiviewcreatorattributes.h>
#include <vstgui4/vstgui/lib/crect.h>
#include <vstgui4/vstgui/lib/ccolor.h>
#include <map>
#include "../logging/loguru.hpp"

namespace pongasoft {
namespace VST {
namespace GUI {

using namespace VSTGUI;


class CustomViewAttribute
{
public:
  CustomViewAttribute(std::string iName) :
    fName(std::move(iName))
  {}

  virtual ~CustomViewAttribute() = default;

  virtual IViewCreator::AttrType getType() = 0;

  std::string getName() const
  {
    return fName;
  }

  virtual bool apply(CView *iView, const UIAttributes &iAttributes, const IUIDescription *iDescription) = 0;

  virtual bool getAttributeValue(CView *iView, const IUIDescription *iDescription, std::string &oStringValue) const = 0;

private:
  std::string fName;
};

template<typename TView>
class CustomViewCreator : public ViewCreatorAdapter
{
private:
  class ColorAttribute : public CustomViewAttribute
  {
  public:
    using Getter = const CColor &(TView::*)() const;
    using Setter = void (TView::*)(CColor const &);

    ColorAttribute(std::string const &iName,
                   Getter iGetter,
                   Setter iSetter) :
      CustomViewAttribute(iName),
      fGetter{iGetter},
      fSetter{iSetter}
    {}

    IViewCreator::AttrType getType() override
    {
      return IViewCreator::kColorType;
    }

    bool apply(CView *iView, const UIAttributes &iAttributes, const IUIDescription *iDescription) override
    {
      auto *tv = dynamic_cast<TView *>(iView);
      if(tv != nullptr)
      {
        CColor color;
        if(UIViewCreator::stringToColor(iAttributes.getAttributeValue(getName()), color,
                                        iDescription))
        {
//        DLOG_F(INFO, "CustomViewCreator::Color::apply(%s,%s)", CustomViewAttribute<TView>::fName.c_str(),
//               iAttributes.getAttributeValue(CustomViewAttribute<TView>::fName)->c_str());
          (tv->*fSetter)(color);
          return true;
        }
      }
      return false;
    }

    bool getAttributeValue(CView *iView, const IUIDescription *iDescription, std::string &oStringValue) const override
    {
      auto *tv = dynamic_cast<TView *>(iView);
      if(tv != nullptr)
      {
        return UIViewCreator::colorToString((tv->*fGetter)(), oStringValue, iDescription);
      }
      return false;
    }

  private:
    Getter fGetter;
    Setter fSetter;
  };

public:
  CustomViewCreator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
    fViewName{iViewName},
    fDisplayName{iDisplayName},
    fAttributes{}
  {
    // this allows for inheritance!
    if(iViewName != nullptr && iDisplayName != nullptr)
      VSTGUI::UIViewFactory::registerViewCreator(*this);
  }

  ~CustomViewCreator() override
  {
    for(auto attribute : fAttributes)
    {
      delete attribute.second;
    }
    fAttributes.clear();
  }

  IdStringPtr getViewName() const override
  {
    return fViewName;
  }

  UTF8StringPtr getDisplayName() const override
  {
    return fDisplayName;
  }

  IdStringPtr getBaseViewName() const override
  {
    return VSTGUI::UIViewCreator::kCControl;
  }

  void registerColorAttribute(std::string const &iName, typename ColorAttribute::Getter iGetter, typename ColorAttribute::Setter iSetter)
  {
    registerAttribute(new ColorAttribute(iName, iGetter, iSetter));
  }

  CView *create(const UIAttributes &attributes, const IUIDescription *description) const override
  {
    DLOG_F(INFO, "CustomViewCreator<%s>::create()", getViewName());
    return new TView(CRect(0, 0, 0, 0), nullptr, -1, nullptr);
  }

  bool apply(CView *view, const UIAttributes &attributes, const IUIDescription *description) const override
  {
    auto *tv = dynamic_cast<TView *>(view);

    if(tv == nullptr)
      return false;

    for(auto attribute : fAttributes)
    {
      attribute.second->apply(tv, attributes, description);
    }


    return true;
  }

  bool getAttributeNames(std::list<std::string> &attributeNames) const override
  {
    for(auto attribute : fAttributes)
    {
      attributeNames.emplace_back(attribute.first);
    }
    return true;
  }

  AttrType getAttributeType(const std::string &attributeName) const override
  {
    auto iter = fAttributes.find(attributeName);
    if(iter != fAttributes.cend())
    {
      return iter->second->getType();
    }
    return kUnknownType;
  }

  bool getAttributeValue(CView *iView,
                         const std::string &iAttributeName,
                         std::string &oStringValue,
                         const IUIDescription *iDescription) const override
  {
    auto *cdv = dynamic_cast<TView *>(iView);

    if(cdv == nullptr)
      return false;

    auto iter = fAttributes.find(iAttributeName);
    if(iter != fAttributes.cend())
    {
      return iter->second->getAttributeValue(cdv, iDescription, oStringValue);
    }

    return false;
  }

private:

  void registerAttribute(CustomViewAttribute *iAttribute)
  {
    fAttributes[iAttribute->getName()] = iAttribute;
  }

  char const *fViewName;
  char const *fDisplayName;
  std::map<std::string, CustomViewAttribute *> fAttributes;
};

}
}
}