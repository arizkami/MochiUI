#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <gui/Components/IconNode.hpp>

namespace MochiUI {

class RatingNode : public FlexNode {
public:
    RatingNode(int maxStars = 5) : maxStars(maxStars) {
        style.setFlexDirection(YGFlexDirectionRow);
        style.setGap(4);
        
        for (int i = 0; i < maxStars; ++i) {
            auto star = std::make_shared<IconNode>();
            star->setIcon("res://star.svg");
            star->style.setWidth(20);
            star->style.setHeight(20);
            star->color = Theme::TextSecondary;
            star->enableHover = true;
            
            star->onClick = [this, i]() {
                setRating(i + 1);
            };
            
            stars.push_back(star);
            addChild(star);
        }
    }

    void setRating(int r) {
        rating = r;
        for (int i = 0; i < maxStars; ++i) {
            stars[i]->color = (i < rating) ? SkColorSetRGB(255, 215, 0) : Theme::TextSecondary;
        }
    }

private:
    int maxStars;
    int rating = 0;
    std::vector<std::shared_ptr<IconNode>> stars;
};

} // namespace MochiUI
