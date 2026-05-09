import React from "react";
import type {
  ButtonProps,
  FlatListProps,
  ImageProps,
  ScrollViewProps,
  SliderProps,
  SwitchProps,
  TextInputProps,
  TextProps,
  TouchableOpacityProps,
  ViewProps,
} from "../payload/types";

export const View = (props: ViewProps) =>
  React.createElement("View", props as any);

export const Text = (props: TextProps) =>
  React.createElement("Text", props as any);

export const Button = ({ title, onPress, style }: ButtonProps) =>
  React.createElement("Button", { title, onPress, style } as any);

export const TouchableOpacity = (props: TouchableOpacityProps) =>
  React.createElement("TouchableOpacity", props as any);

export const Pressable = TouchableOpacity;

export const SafeAreaView = (props: ViewProps) =>
  React.createElement("SafeAreaView", props as any);

export const ScrollView = ({ horizontal, style, contentContainerStyle, children }: ScrollViewProps) =>
  React.createElement(
    horizontal ? "HScrollView" : "ScrollView",
    { style: { ...(style ?? {}), ...(contentContainerStyle ?? {}) } } as any,
    children,
  );

export const TextInput = (props: TextInputProps) =>
  React.createElement("TextInput", props as any);

export const Switch = (props: SwitchProps) =>
  React.createElement("Switch", props as any);

export const Slider = (props: SliderProps) =>
  React.createElement("Slider", props as any);

export const Image = (props: ImageProps) =>
  React.createElement("Image", props as any);

export function FlatList<T>({
  data,
  renderItem,
  style,
  horizontal,
}: FlatListProps<T>): React.ReactElement {
  const nativeType = horizontal ? "HScrollView" : "FlatList";
  const items = data
    .map((item, index) => renderItem({ item, index }))
    .filter((el): el is React.ReactElement => el != null);
  return React.createElement(nativeType as any, { style } as any, ...items);
}
