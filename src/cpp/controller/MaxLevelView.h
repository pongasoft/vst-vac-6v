#pragma once

#include <pongasoft/VST/Messaging.h>
#include "HistoryView.h"

namespace pongasoft::VST::VAC6 {

using namespace VSTGUI;
using namespace Common;
using namespace GUI;

///////////////////////////////////////////
// MaxLevelView
///////////////////////////////////////////
class MaxLevelView : public HistoryView
{
public:
  enum Type
  {
    kForSelection,
    kSinceReset,
    kInWindow
  };

  // Constructor
  explicit MaxLevelView(const CRect &size);

  MaxLevelView(const MaxLevelView &c) = delete;

  // get/setNoDataColor
  const CColor &getNoDataColor() const { return fNoDataColor;  }
  void setNoDataColor(const CColor &iNoDataColor) { fNoDataColor = iNoDataColor; }

  // get/setFont
  FontPtr getFont() const { return fFont; }
  void setFont(FontPtr iFont) { fFont = iFont; }

  // get/setType
  MaxLevelView::Type getType() const { return fType; }
  void setType(MaxLevelView::Type iType) { fType = iType; }

  // getMaxLevel
  MaxLevel getMaxLevel() const;

  // draw => does the actual drawing job
  void draw(CDrawContext *iContext) override;

  CLASS_METHODS_NOCOPY(MaxLevelView, HistoryView)

protected:
  CColor fNoDataColor{};
  FontSPtr fFont{nullptr};
  MaxLevelView::Type fType{MaxLevelView::Type::kForSelection};

public:
  class Creator : public CustomViewCreator<MaxLevelView, HistoryView>
  {
  public:
    explicit Creator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
      CustomViewCreator(iViewName, iDisplayName)
    {
      registerColorAttribute("no-data-color",
                             &MaxLevelView::getNoDataColor,
                             &MaxLevelView::setNoDataColor);
      registerFontAttribute("font",
                            &MaxLevelView::getFont,
                            &MaxLevelView::setFont);
      registerIntegerAttribute<Type>("type",
                                     &MaxLevelView::getType,
                                     &MaxLevelView::setType);
    }
  };
};

}