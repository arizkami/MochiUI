import { defineComponent, h } from "@vue/runtime-core";
import { styleProp } from "../payload/types";

function createHostComponent(type: string) {
  return defineComponent({
    name: type,
    inheritAttrs: false,
    props: {
      style: styleProp,
    },
    setup(props, { attrs, slots }) {
      return () => h(type, { ...attrs, ...props }, slots.default ? slots.default() : undefined);
    },
  });
}

export const View = createHostComponent("View");
export const Text = createHostComponent("Text");
export const SafeAreaView = createHostComponent("SafeAreaView");
export const ScrollView = createHostComponent("ScrollView");
export const TouchableOpacity = createHostComponent("TouchableOpacity");
export const Pressable = TouchableOpacity;
export const Image = createHostComponent("Image");

export const Button = defineComponent({
  name: "Button",
  inheritAttrs: false,
  props: {
    title: { type: String, required: true },
    style: styleProp,
  },
  setup(props, { attrs }) {
    return () => h("Button", { ...attrs, ...props });
  },
});

export const TextInput = defineComponent({
  name: "TextInput",
  inheritAttrs: false,
  props: {
    style: styleProp,
    value: { type: String, required: false },
    placeholder: { type: String, required: false },
  },
  setup(props, { attrs }) {
    return () => h("TextInput", { ...attrs, ...props });
  },
});

export const Switch = defineComponent({
  name: "Switch",
  inheritAttrs: false,
  props: {
    style: styleProp,
    value: { type: Boolean, required: true },
    label: { type: String, required: false },
    switchWidth: { type: Number, required: false },
    switchHeight: { type: Number, required: false },
    switchActiveColor: { type: String, required: false },
    switchInactiveColor: { type: String, required: false },
    switchThumbColor: { type: String, required: false },
    switchLabelColor: { type: String, required: false },
    switchBorderColor: { type: String, required: false },
    switchShadowColor: { type: String, required: false },
  },
  emits: ["update:value", "valueChange"],
  setup(props, { attrs, emit }) {
    return () => h("Switch", {
      ...attrs,
      ...props,
      onValueChange: (value: boolean) => {
        emit("update:value", value);
        emit("valueChange", value);
        const handler = attrs.onValueChange as ((next: boolean) => void) | undefined;
        handler?.(value);
      },
    });
  },
});

export const Slider = defineComponent({
  name: "Slider",
  inheritAttrs: false,
  props: {
    style: styleProp,
    value: { type: Number, required: false },
    minimumValue: { type: Number, required: false },
    maximumValue: { type: Number, required: false },
    vertical: { type: Boolean, required: false },
  },
  emits: ["update:value", "valueChange"],
  setup(props, { attrs, emit }) {
    return () => h("Slider", {
      ...attrs,
      ...props,
      onValueChange: (value: number) => {
        emit("update:value", value);
        emit("valueChange", value);
        const handler = attrs.onValueChange as ((next: number) => void) | undefined;
        handler?.(value);
      },
    });
  },
});
