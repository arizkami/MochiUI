import type { PropType, VNodeChild } from "vue";

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

export const styleProp = {
  type: [Object, Array] as PropType<ViewStyle | ViewStyle[]>,
  required: false,
};

export const childrenSlotType = {} as PropType<() => VNodeChild>;
