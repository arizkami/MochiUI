<script setup lang="ts">
import {
  Button,
  computed,
  ref,
  SafeAreaView,
  ScrollView,
  StyleSheet,
  Switch,
  Text,
  TextInput,
  View,
} from "../src";

const title = ref("Vue support");
const notes = ref("Custom renderer using SphereUI host nodes.");
const compact = ref(false);
const inspector = ref(true);
const setTitle = (value: string) => {
  title.value = value;
};
const setNotes = (value: string) => {
  notes.value = value;
};
const setCompact = (value: boolean) => {
  compact.value = value;
};
const setInspector = (value: boolean) => {
  inspector.value = value;
};

const styles = StyleSheet.create({
  root: {
    flex: 1,
    backgroundColor: "#0c1017",
    padding: 18,
  },
  scroll: {
    flex: 1,
  },
  content: {
    gap: 16,
    paddingBottom: 18,
  },
  hero: {
    backgroundColor: "#111826",
    borderColor: "#253348",
    borderWidth: 1,
    borderRadius: 20,
    padding: 18,
    gap: 14,
  },
  heading: {
    fontSize: 26,
    fontWeight: "800",
    color: "#edf4fb",
  },
  body: {
    fontSize: 13,
    color: "#93a7ba",
  },
  input: {
    backgroundColor: "#0b131c",
    borderColor: "#253348",
    borderWidth: 1,
    borderRadius: 12,
    color: "#edf4fb",
    padding: 10,
    fontSize: 13,
  },
  row: {
    flexDirection: "row",
    flexWrap: "wrap",
    gap: 12,
  },
  card: {
    backgroundColor: "#101723",
    borderColor: "#243243",
    borderWidth: 1,
    borderRadius: 16,
    padding: 14,
    gap: 10,
    flexGrow: 1,
    flexShrink: 1,
    flexBasis: 240,
  },
  cardTitle: {
    fontSize: 12,
    fontWeight: "700",
    color: "#edf4fb",
  },
  actionRow: {
    flexDirection: "row",
    flexWrap: "wrap",
    gap: 8,
  },
  badge: {
    backgroundColor: "#17304a",
    borderColor: "#2d557c",
    borderWidth: 1,
    borderRadius: 999,
    paddingLeft: 10,
    paddingRight: 10,
    paddingTop: 6,
    paddingBottom: 6,
  },
  badgeText: {
    fontSize: 11,
    fontWeight: "700",
    color: "#b9dcff",
  },
});

const densityLabel = computed(() => compact.value ? "compact" : "comfortable");
</script>

<template>
  <SafeAreaView :style="styles.root">
    <ScrollView :style="styles.scroll">
      <View :style="styles.content">
        <View :style="styles.hero">
          <Text :style="styles.heading">{{ title }}</Text>
          <Text :style="styles.body">{{ notes }}</Text>
          <TextInput :value="title" placeholder="Scene title" :style="styles.input" :onChangeText="setTitle" />
          <TextInput :value="notes" placeholder="Scene notes" :style="styles.input" :onChangeText="setNotes" />
        </View>

        <View :style="styles.row">
          <View :style="styles.card">
            <Text :style="styles.cardTitle">Renderer</Text>
            <View :style="styles.badge">
              <Text :style="styles.badgeText">SphereKit.Vue</Text>
            </View>
            <Text :style="styles.body">Vue custom renderer mapped onto SphereUI nodes with the same native bridge used by JS frameworks.</Text>
          </View>

          <View :style="styles.card">
            <Text :style="styles.cardTitle">State</Text>
            <Switch
              label="Compact mode"
              :value="compact"
              switchActiveColor="#64c37f"
              switchInactiveColor="#2d3b4b"
              switchThumbColor="#f4f8ff"
              switchBorderColor="#3a5267"
              @update:value="setCompact"
            />
            <Switch
              label="Inspector"
              :value="inspector"
              switchActiveColor="#5bb9ff"
              switchInactiveColor="#2d3b4b"
              switchThumbColor="#f4f8ff"
              switchBorderColor="#3a5267"
              @update:value="setInspector"
            />
            <Text :style="styles.body">Current density: {{ densityLabel }}</Text>
          </View>

          <View v-if="inspector" :style="styles.card">
            <Text :style="styles.cardTitle">Inspector</Text>
            <Text :style="styles.body">Title: {{ title }}</Text>
            <Text :style="styles.body">Notes length: {{ notes.length }}</Text>
            <Text :style="styles.body">Density: {{ densityLabel }}</Text>
          </View>
        </View>

        <View :style="styles.card">
          <Text :style="styles.cardTitle">Actions</Text>
          <View :style="styles.actionRow">
            <Button title="Compact" :onPress="() => setCompact(true)" />
            <Button title="Comfortable" :onPress="() => setCompact(false)" />
            <Button title="Toggle Inspector" :onPress="() => setInspector(!inspector)" />
          </View>
        </View>
      </View>
    </ScrollView>
  </SafeAreaView>
</template>
