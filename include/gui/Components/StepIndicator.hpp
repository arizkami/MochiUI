#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <gui/Components/TextNode.hpp>

namespace SphereUI {

class StepIndicator : public FlexNode {
public:
    StepIndicator(const std::vector<std::string>& steps) : steps(steps) {
        style.setFlexDirection(YGFlexDirectionRow);
        style.setAlignItems(YGAlignCenter);
        style.setGap(10);
        setCurrentStep(0);
    }

    void setCurrentStep(size_t index) {
        currentStep = index;
        removeAllChildren();
        for (size_t i = 0; i < steps.size(); ++i) {
            if (i > 0) {
                auto line = FlexNode::Create();
                line->style.setHeight(2);
                line->style.setWidth(30);
                line->style.backgroundColor = (i <= currentStep) ? Theme::Accent : Theme::Border;
                addChild(line);
            }

            auto circle = std::make_shared<FlexNode>();
            circle->style.setWidth(24);
            circle->style.setHeight(24);
            circle->style.borderRadius = 12;
            circle->style.backgroundColor = (i <= currentStep) ? Theme::Accent : Theme::Card;
            circle->style.setAlignItems(YGAlignCenter);

            auto num = std::make_shared<TextNode>(std::to_string(i + 1));
            num->fontSize = 12;
            num->color = (i <= currentStep) ? SPHXColor::white() : SPHXColor(Theme::TextSecondary);
            circle->addChild(num);

            addChild(circle);
        }
    }

private:
    std::vector<std::string> steps;
    size_t currentStep = 0;
};

} // namespace SphereUI
