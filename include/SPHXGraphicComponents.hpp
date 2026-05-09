#pragma once
#include <SPHXFoundation.hpp>

// Undefine Win32 RGB macro so SPHXColor::RGB(...) works in component headers.
// windows.h (pulled in via SPHXGraphicInterface.hpp) defines RGB before this
// header is processed; SPHXColor.hpp's own #undef fires too early to help.
#ifdef RGB
#undef RGB
#endif

#include <gui/Components/TextNode.hpp>
#include <gui/Components/CheckboxNode.hpp>
#include <gui/Components/SliderNode.hpp>
#include <gui/Components/KnobNode.hpp>
#include <gui/Components/VUMeterNode.hpp>
#include <gui/Components/ScrollAreaNode.hpp>
#include <gui/Components/GroupBox.hpp>
#include <gui/Components/TextInput.hpp>
#include <gui/Components/NumberInput.hpp>
#include <gui/Components/ComboBox.hpp>
#include <gui/Components/ProgressBar.hpp>
#include <gui/Components/GraphNode.hpp>
#include <gui/Components/ButtonNode.hpp>
#include <gui/Components/Table.hpp>
#include <gui/Components/ColorPicker.hpp>
#include <gui/Components/Calendar.hpp>
#include <gui/Components/IconNode.hpp>
#include <gui/Components/DatePicker.hpp>
#include <gui/Components/Popover.hpp>
#include <gui/Components/SwitchNode.hpp>
#include <gui/Components/RadioButtonNode.hpp>
#include <gui/Components/SeparatorNode.hpp>
#include <gui/Components/BadgeNode.hpp>
#include <gui/Components/SpinnerNode.hpp>
#include <gui/Components/LinkNode.hpp>
#include <gui/Components/TabsNode.hpp>
#include <gui/Components/AccordionNode.hpp>
#include <gui/Components/SplitterNode.hpp>
#include <gui/Components/ModalNode.hpp>
#include <gui/Components/SidebarNode.hpp>
#include <gui/Components/SearchInputNode.hpp>
#include <gui/Components/RatingNode.hpp>
#include <gui/Components/TooltipNode.hpp>
#include <gui/Components/ToastNode.hpp>
#include <gui/Components/SkeletonNode.hpp>
#include <gui/Components/BreadcrumbsNode.hpp>
#include <gui/Components/ToolbarNode.hpp>
#include <gui/Components/EmptyStateNode.hpp>
#include <gui/Components/StepIndicator.hpp>
#include <gui/Components/TagInputNode.hpp>
#include <gui/Components/TreeView.hpp>
#include <gui/Components/FilePickerNode.hpp>
#include <gui/Components/MultiSelectNode.hpp>
#include <gui/Components/LegendNode.hpp>
#include <gui/Components/HeatmapNode.hpp>
#include <gui/Components/TimelineNode.hpp>
