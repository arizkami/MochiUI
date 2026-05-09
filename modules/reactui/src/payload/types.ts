import React from "react";

export interface ViewStyle {
  flex?: number;
  flexGrow?: number;
  flexShrink?: number;
  flexBasis?: number;
  flexDirection?: "row" | "column";
  flexWrap?: "wrap" | "nowrap";
  alignItems?: "flex-start" | "center" | "flex-end" | "stretch";
  justifyContent?: "flex-start" | "center" | "flex-end" | "space-between" | "space-around" | "space-evenly";
  width?: number | string;
  height?: number | string;
  minWidth?: number;
  minHeight?: number;
  margin?: number;
  marginTop?: number;
  marginBottom?: number;
  marginLeft?: number;
  marginRight?: number;
  marginHorizontal?: number;
  marginVertical?: number;
  padding?: number;
  paddingTop?: number;
  paddingBottom?: number;
  paddingLeft?: number;
  paddingRight?: number;
  paddingHorizontal?: number;
  paddingVertical?: number;
  gap?: number;
  backgroundColor?: string;
  borderColor?: string;
  borderWidth?: number;
  borderRadius?: number;
  overflow?: "hidden" | "visible";
  position?: "absolute" | "relative";
  top?: number;
  left?: number;
  right?: number;
  bottom?: number;
}

export interface TextStyle extends ViewStyle {
  color?: string;
  fontSize?: number;
  fontWeight?: "normal" | "bold" | "100" | "200" | "300" | "400" | "500" | "600" | "700" | "800" | "900";
  textAlign?: "left" | "center" | "right";
}

export interface ViewProps {
  style?: ViewStyle;
  children?: React.ReactNode;
  onPress?: () => void;
}

export interface TextProps {
  style?: TextStyle;
  children?: React.ReactNode;
}

export interface ButtonProps {
  title: string;
  onPress?: () => void;
  style?: ViewStyle;
}

export interface TouchableOpacityProps {
  onPress?: () => void;
  style?: ViewStyle;
  children?: React.ReactNode;
}

export interface ScrollViewProps {
  style?: ViewStyle;
  contentContainerStyle?: ViewStyle;
  horizontal?: boolean;
  children?: React.ReactNode;
}

export interface TextInputProps {
  value?: string;
  placeholder?: string;
  onChangeText?: (text: string) => void;
  onSubmitEditing?: () => void;
  style?: ViewStyle & TextStyle;
}

export interface SwitchProps {
  value: boolean;
  onValueChange?: (value: boolean) => void;
  style?: ViewStyle;
  label?: string;
  switchWidth?: number;
  switchHeight?: number;
  switchActiveColor?: string;
  switchInactiveColor?: string;
  switchThumbColor?: string;
  switchLabelColor?: string;
  switchBorderColor?: string;
  switchShadowColor?: string;
}

export interface SliderProps {
  value?: number;
  minimumValue?: number;
  maximumValue?: number;
  onValueChange?: (value: number) => void;
  style?: ViewStyle;
  vertical?: boolean;
}

export interface ImageProps {
  source?: { uri?: string } | number;
  style?: ViewStyle;
}

export interface FlatListProps<T> {
  data: T[];
  renderItem: (info: { item: T; index: number }) => React.ReactElement | null;
  keyExtractor?: (item: T, index: number) => string;
  style?: ViewStyle;
  contentContainerStyle?: ViewStyle;
  horizontal?: boolean;
}
