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

template<typename TView>
class CustomViewAttribute
{
public:
  CustomViewAttribute(std::string const &iName) : fName(iName)
  {}

  virtual ~CustomViewAttribute() = default;

  virtual IViewCreator::AttrType getType() = 0;

  std::string fName;

  virtual void apply(TView *iView, const UIAttributes &iAttributes, const IUIDescription *iDescription) = 0;

  virtual std::string getAttributeValue(TView *iView, const IUIDescription *iDescription) const = 0;
};

struct TemplateText
{
  const char *text;
};

template<typename TView, TemplateText const &ViewName, TemplateText const &DisplayName>
class CustomViewCreator : public ViewCreatorAdapter
{
private:
  class Color : public CustomViewAttribute<TView>
  {
  public:
    using Getter = const CColor &(TView::*)() const;
    using Setter = void (TView::*)(CColor const &);

    Color(std::string const &iName,
          Getter iGetter,
          Setter iSetter) :
      CustomViewAttribute<TView>(iName),
      fGetter{iGetter},
      fSetter{iSetter}
    {}

    IViewCreator::AttrType getType() override
    {
      return IViewCreator::kColorType;
    }

    void apply(TView *iView, const UIAttributes &iAttributes, const IUIDescription *iDescription) override
    {
      CColor color;
      if(UIViewCreator::stringToColor(iAttributes.getAttributeValue(CustomViewAttribute<TView>::fName), color,
                                      iDescription))
      {
//        DLOG_F(INFO, "CustomViewCreator::Color::apply(%s,%s)", CustomViewAttribute<TView>::fName.c_str(),
//               iAttributes.getAttributeValue(CustomViewAttribute<TView>::fName)->c_str());
        (iView->*fSetter)(color);
      }
    }

    std::string getAttributeValue(TView *iView, const IUIDescription *iDescription) const override
    {
      std::string stringValue;
      UIViewCreator::colorToString((iView->*fGetter)(), stringValue, iDescription);
//      DLOG_F(INFO, "CustomViewCreator::Color::getAttributeValue(%s)=%s", CustomViewAttribute<TView>::fName.c_str(), stringValue.c_str());
      return stringValue;
    }

  private:
    Getter fGetter;
    Setter fSetter;
  };

public:
  CustomViewCreator() : fAttributes{}
  {
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
    return ViewName.text;
  }

  UTF8StringPtr getDisplayName() const override
  {
    return DisplayName.text;
  }

  IdStringPtr getBaseViewName() const override
  {
    return VSTGUI::UIViewCreator::kCControl;
  }

  void registerColorAttribute(std::string const &iName, typename Color::Getter iGetter, typename Color::Setter iSetter)
  {
    registerAttribute(new Color(iName, iGetter, iSetter));
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

  bool getAttributeValue(CView *view, const std::string &attributeName, std::string &stringValue,
                         const IUIDescription *desc) const override
  {
    auto *cdv = dynamic_cast<TView *>(view);

    if(cdv == nullptr)
      return false;

    auto iter = fAttributes.find(attributeName);
    if(iter != fAttributes.cend())
    {
      stringValue = iter->second->getAttributeValue(cdv, desc);
      return true;
    }

    return false;
  }

private:

  void registerAttribute(CustomViewAttribute<TView> *iAttribute)
  {
    fAttributes[iAttribute->fName] = iAttribute;
  }

  std::map<std::string, CustomViewAttribute<TView> *> fAttributes;
};

}
}
}