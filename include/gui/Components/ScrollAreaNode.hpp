#pragma once
#include <include/gui/Layout.hpp>
#include <include/gui/Theme.hpp>
#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>

namespace MochiUI {

class ScrollAreaNode : public FlexNode {
public:
    FlexNode::Ptr content;
    
    // Scrollbar appearance
    SkColor scrollbarTrackColor = SkColorSetARGB(60, 255, 255, 255);
    SkColor scrollbarThumbColor = SkColorSetARGB(120, 255, 255, 255);
    SkColor scrollbarThumbHoverColor = SkColorSetARGB(180, 255, 255, 255);
    
    float scrollbarWidth = 8.0f;
    float scrollbarPadding = 2.0f;
    
    bool showVerticalScrollbar = true;
    bool showHorizontalScrollbar = false;
    
    // Scroll position
    float scrollX = 0.0f;
    float scrollY = 0.0f;
    
    void setContent(FlexNode::Ptr node);
    void scrollTo(float x, float y);
    void scrollBy(float dx, float dy);
    
    Size measure(Size available) override;
    void calculateLayout(SkRect availableSpace) override;
    void draw(SkCanvas* canvas) override;
    bool onMouseDown(float x, float y) override;
    bool onMouseMove(float x, float y) override;
    void onMouseUp(float x, float y) override;
    bool onMouseWheel(float x, float y, float delta) override;
    
    void onMouseEnter() override;
    void onMouseLeave() override;

private:
    void updateScrollBounds();
    SkRect getVerticalScrollbarRect() const;
    SkRect getHorizontalScrollbarRect() const;
    SkRect getVerticalThumbRect() const;
    SkRect getHorizontalThumbRect() const;
    bool isPointInVerticalScrollbar(float x, float y) const;
    bool isPointInHorizontalScrollbar(float x, float y) const;
    
    float maxScrollX = 0.0f;
    float maxScrollY = 0.0f;
    float contentWidth = 0.0f;
    float contentHeight = 0.0f;
    
    float scrollbarOpacity = 0.0f; // For fade in/out
    
    bool isDraggingVertical = false;
    bool isDraggingHorizontal = false;
    bool isHoveringVertical = false;
    bool isHoveringHorizontal = false;
    float dragStartY = 0.0f;
    float dragStartX = 0.0f;
    float dragStartScrollY = 0.0f;
    float dragStartScrollX = 0.0f;
};

} // namespace MochiUI
