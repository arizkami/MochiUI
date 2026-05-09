import React, { useState } from "react";
import {
  View,
  Text,
  Button,
  ScrollView,
  TextInput,
  Switch,
  SafeAreaView,
  StyleSheet,
  mount,
} from "../src/index";

type Density = "compact" | "balanced" | "airy";
type RowMode = "space-between" | "center" | "space-around";
type WrapMode = "wrap" | "nowrap";

type MetricCard = {
  id: string;
  label: string;
  value: string;
  tone: string;
};

const METRICS: MetricCard[] = [
  { id: "fps", label: "Frame budget", value: "16.6 ms", tone: "#ff8e5f" },
  { id: "nodes", label: "Visible nodes", value: "124", tone: "#61c8ff" },
  { id: "wrap", label: "Wrap columns", value: "Auto", tone: "#9bde6d" },
  { id: "events", label: "Input latency", value: "4 ms", tone: "#ffd166" },
];

const TOKENS = ["hug", "fill", "basis 240", "gap 12", "wrap", "badge", "stack", "aside"];

const s = StyleSheet.create({
  root: {
    flex: 1,
    backgroundColor: "#0b1016",
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
    backgroundColor: "#101a23",
    borderColor: "#273545",
    borderWidth: 1,
    borderRadius: 20,
    padding: 18,
    gap: 14,
  },
  heroTop: {
    flexDirection: "row" as const,
    justifyContent: "space-between" as const,
    alignItems: "center" as const,
    gap: 12,
  },
  eyebrow: {
    fontSize: 11,
    fontWeight: "700" as const,
    color: "#5db6ff",
  },
  title: {
    fontSize: 28,
    fontWeight: "800" as const,
    color: "#edf4fb",
  },
  subtitle: {
    fontSize: 13,
    color: "#95a9bc",
  },
  badge: {
    backgroundColor: "#0f2635",
    borderColor: "#214761",
    borderWidth: 1,
    borderRadius: 999,
    paddingLeft: 10,
    paddingRight: 10,
    paddingTop: 6,
    paddingBottom: 6,
  },
  badgeText: {
    fontSize: 11,
    fontWeight: "700" as const,
    color: "#8fd3ff",
  },
  controlRail: {
    flexDirection: "row" as const,
    flexWrap: "wrap" as const,
    gap: 12,
  },
  controlCard: {
    backgroundColor: "#13212d",
    borderColor: "#24384a",
    borderWidth: 1,
    borderRadius: 16,
    padding: 14,
    gap: 10,
    minWidth: 220,
    flexGrow: 1,
    flexShrink: 1,
    flexBasis: 260,
  },
  controlTitle: {
    fontSize: 12,
    fontWeight: "700" as const,
    color: "#edf4fb",
  },
  controlText: {
    fontSize: 12,
    color: "#89a0b4",
  },
  input: {
    backgroundColor: "#091119",
    borderColor: "#24384a",
    borderWidth: 1,
    borderRadius: 12,
    color: "#edf4fb",
    padding: 10,
    fontSize: 13,
  },
  actionRow: {
    flexDirection: "row" as const,
    flexWrap: "wrap" as const,
    gap: 8,
  },
  section: {
    backgroundColor: "#0f1822",
    borderColor: "#1e2e3c",
    borderWidth: 1,
    borderRadius: 18,
    padding: 16,
    gap: 14,
  },
  sectionTitle: {
    fontSize: 18,
    fontWeight: "800" as const,
    color: "#edf4fb",
  },
  sectionText: {
    fontSize: 13,
    color: "#8ea2b5",
  },
  metricRow: {
    flexDirection: "row" as const,
    flexWrap: "wrap" as const,
    gap: 12,
  },
  metricCard: {
    backgroundColor: "#111e29",
    borderColor: "#233645",
    borderWidth: 1,
    borderRadius: 16,
    padding: 14,
    minWidth: 160,
    flexGrow: 1,
    flexShrink: 1,
    flexBasis: 180,
    gap: 8,
  },
  metricLabel: {
    fontSize: 12,
    color: "#7f96ab",
  },
  metricValue: {
    fontSize: 22,
    fontWeight: "800" as const,
    color: "#edf4fb",
  },
  rowPreview: {
    borderColor: "#294054",
    borderWidth: 1,
    borderRadius: 14,
    backgroundColor: "#0c131a",
    padding: 12,
    gap: 12,
  },
  previewBar: {
    flexDirection: "row" as const,
    alignItems: "center" as const,
    borderColor: "#223240",
    borderWidth: 1,
    borderRadius: 12,
    padding: 10,
    backgroundColor: "#111b24",
    minHeight: 64,
  },
  previewChip: {
    borderRadius: 999,
    paddingLeft: 10,
    paddingRight: 10,
    paddingTop: 7,
    paddingBottom: 7,
  },
  previewChipText: {
    fontSize: 11,
    fontWeight: "700" as const,
    color: "#091119",
  },
  splitLayout: {
    flexDirection: "row" as const,
    flexWrap: "wrap" as const,
    gap: 12,
  },
  stage: {
    backgroundColor: "#0c131a",
    borderColor: "#223240",
    borderWidth: 1,
    borderRadius: 16,
    padding: 12,
    minHeight: 280,
    flexGrow: 3,
    flexShrink: 1,
    flexBasis: 540,
    gap: 12,
  },
  inspector: {
    backgroundColor: "#111b24",
    borderColor: "#223240",
    borderWidth: 1,
    borderRadius: 16,
    padding: 12,
    flexGrow: 1,
    flexShrink: 1,
    flexBasis: 250,
    gap: 10,
  },
  wrapStage: {
    flexDirection: "row" as const,
    flexWrap: "wrap" as const,
    alignItems: "flex-start" as const,
    gap: 12,
  },
  wrapCard: {
    backgroundColor: "#13202b",
    borderColor: "#294054",
    borderWidth: 1,
    borderRadius: 14,
    padding: 12,
    gap: 8,
    minHeight: 116,
  },
  wrapCardTitle: {
    fontSize: 13,
    fontWeight: "800" as const,
    color: "#edf4fb",
  },
  wrapCardText: {
    fontSize: 12,
    color: "#8ba1b4",
  },
  stack: {
    gap: 8,
  },
  tokenRow: {
    flexDirection: "row" as const,
    flexWrap: "wrap" as const,
    gap: 8,
  },
  token: {
    backgroundColor: "#1a2d3d",
    borderColor: "#31506a",
    borderWidth: 1,
    borderRadius: 999,
    paddingLeft: 10,
    paddingRight: 10,
    paddingTop: 5,
    paddingBottom: 5,
  },
  tokenText: {
    fontSize: 11,
    fontWeight: "700" as const,
    color: "#c2e7ff",
  },
  tinyLabel: {
    fontSize: 11,
    color: "#6f879a",
  },
});

function toneForDensity(density: Density) {
  if (density === "compact") return "#8bd450";
  if (density === "airy") return "#ffb454";
  return "#66c7ff";
}

function metricValueForWidth(isWide: boolean) {
  return isWide ? "Wide" : "Stack";
}

function basisForDensity(density: Density) {
  if (density === "compact") return 180;
  if (density === "airy") return 300;
  return 240;
}

function gapForDensity(density: Density) {
  if (density === "compact") return 8;
  if (density === "airy") return 18;
  return 12;
}

function titleForMode(mode: RowMode) {
  if (mode === "center") return "Centered rhythm";
  if (mode === "space-around") return "Breathing band";
  return "Pinned edges";
}

function justifyForMode(mode: RowMode): "space-between" | "center" | "space-around" {
  return mode;
}

function panelWidthFor(index: number, density: Density, isWide: boolean) {
  const base = basisForDensity(density);
  if (!isWide) return base;
  return index % 3 === 0 ? base + 90 : base;
}

function App() {
  const [title, setTitle] = useState("Flexbox lab");
  const [notes, setNotes] = useState("Stress rows, wraps, basis, and nested text.");
  const [density, setDensity] = useState<Density>("balanced");
  const [rowMode, setRowMode] = useState<RowMode>("space-between");
  const [wrapMode, setWrapMode] = useState<WrapMode>("wrap");
  const [showInspector, setShowInspector] = useState(true);
  const [isWideCards, setIsWideCards] = useState(true);
  const [showBadges, setShowBadges] = useState(true);

  const basis = basisForDensity(density);
  const gap = gapForDensity(density);
  const densityTone = toneForDensity(density);

  return (
    <SafeAreaView style={s.root}>
      <ScrollView style={s.scroll}>
        <View style={s.content}>
          <View style={s.hero}>
          <View style={s.heroTop}>
            <View style={{ flexGrow: 1, flexShrink: 1, gap: 6 }}>
              <Text style={s.eyebrow}>SphereKIT REACTUI</Text>
              <Text style={s.title}>Flex Layout Proving Ground</Text>
              <Text style={s.subtitle}>
                A live layout scene for row packing, wrap behavior, fixed-plus-flex mixing, text density, and toggle polish.
              </Text>
            </View>
            <View style={s.badge}>
              <Text style={s.badgeText}>native layout bridge</Text>
            </View>
          </View>

          <View style={s.controlRail}>
            <View style={s.controlCard}>
              <Text style={s.controlTitle}>Scene label</Text>
              <TextInput value={title} onChangeText={setTitle} style={s.input} />
              <Text style={s.controlText}>{notes}</Text>
            </View>

            <View style={s.controlCard}>
              <Text style={s.controlTitle}>Narrative</Text>
              <TextInput value={notes} onChangeText={setNotes} style={s.input} />
              <View style={s.actionRow}>
                <Button title="Compact" onPress={() => setDensity("compact")} />
                <Button title="Balanced" onPress={() => setDensity("balanced")} />
                <Button title="Airy" onPress={() => setDensity("airy")} />
              </View>
            </View>

            <View style={s.controlCard}>
              <Text style={s.controlTitle}>Display controls</Text>
              <Switch
                label="Inspector"
                value={showInspector}
                onValueChange={setShowInspector}
                switchActiveColor="#59c27a"
                switchInactiveColor="#2a3743"
                switchThumbColor="#f2f8ff"
                switchBorderColor="#365063"
              />
              <Switch
                label="Wide cards"
                value={isWideCards}
                onValueChange={setIsWideCards}
                switchActiveColor="#55b7ff"
                switchInactiveColor="#2a3743"
                switchThumbColor="#f2f8ff"
                switchBorderColor="#365063"
              />
              <Switch
                label="Token badges"
                value={showBadges}
                onValueChange={setShowBadges}
                switchActiveColor="#ff9d5c"
                switchInactiveColor="#2a3743"
                switchThumbColor="#fff6ef"
                switchBorderColor="#365063"
              />
            </View>
          </View>
          </View>

          <View style={s.section}>
          <Text style={s.sectionTitle}>{title}</Text>
          <Text style={s.sectionText}>{notes}</Text>

          <View style={s.metricRow}>
            {METRICS.map((metric) => (
              <View key={metric.id} style={s.metricCard}>
                <Text style={s.metricLabel}>{metric.label}</Text>
                <Text style={{ ...s.metricValue, color: metric.id === "wrap" ? densityTone : "#edf4fb" }}>
                  {metric.id === "wrap"
                    ? `${wrapMode} / ${basis}px`
                    : metric.id === "nodes"
                    ? metricValueForWidth(isWideCards)
                    : metric.value}
                </Text>
                <View style={{ height: 4, borderRadius: 999, backgroundColor: metric.tone }} />
              </View>
            ))}
          </View>
          </View>

          <View style={s.section}>
          <Text style={s.sectionTitle}>Row Alignment Test</Text>
          <Text style={s.sectionText}>
            The band below keeps three blocks in one horizontal run while the buttons swap justification strategies.
          </Text>

          <View style={s.actionRow}>
            <Button title="Edge" onPress={() => setRowMode("space-between")} />
            <Button title="Center" onPress={() => setRowMode("center")} />
            <Button title="Around" onPress={() => setRowMode("space-around")} />
            <Button title={wrapMode === "wrap" ? "Disable wrap" : "Enable wrap"} onPress={() => setWrapMode(wrapMode === "wrap" ? "nowrap" : "wrap")} />
          </View>

          <View style={s.rowPreview}>
            <Text style={s.controlTitle}>{titleForMode(rowMode)}</Text>
            <View style={{ ...s.previewBar, justifyContent: justifyForMode(rowMode) }}>
              <View style={{ ...s.previewChip, backgroundColor: "#66c7ff" }}>
                <Text style={s.previewChipText}>hero</Text>
              </View>
              <View style={{ ...s.previewChip, backgroundColor: "#9bde6d" }}>
                <Text style={s.previewChipText}>content</Text>
              </View>
              <View style={{ ...s.previewChip, backgroundColor: "#ffd166" }}>
                <Text style={s.previewChipText}>actions</Text>
              </View>
            </View>
          </View>
          </View>

          <View style={s.section}>
          <Text style={s.sectionTitle}>Wrap and Basis Stress</Text>
          <Text style={s.sectionText}>
            Each panel mixes text blocks, fixed accents, and flexible width. This is the part intended to catch bad basis handling.
          </Text>

          <View style={s.splitLayout}>
            <View style={s.stage}>
              <View style={{ ...s.wrapStage, flexWrap: wrapMode }}>
                {["Hierarchy", "Density", "Overflow", "Spacing", "Columns", "Text flow"].map((name, index) => (
                  <View
                    key={name}
                    style={{
                      ...s.wrapCard,
                      flexGrow: index % 2 === 0 ? 1 : 0,
                      flexShrink: 1,
                      flexBasis: panelWidthFor(index, density, isWideCards),
                    }}
                  >
                    <Text style={s.wrapCardTitle}>{name}</Text>
                    <Text style={s.wrapCardText}>
                      Card {index + 1} mixes long and short copy to prove that the layout engine respects basis before expansion.
                    </Text>
                    {showBadges ? (
                      <View style={s.tokenRow}>
                        {TOKENS.slice(0, 4 + (index % 3)).map((token) => (
                          <View key={`${name}-${token}`} style={s.token}>
                            <Text style={s.tokenText}>{token}</Text>
                          </View>
                        ))}
                      </View>
                    ) : null}
                  </View>
                ))}
              </View>
            </View>

            {showInspector ? (
              <View style={s.inspector}>
                <Text style={s.controlTitle}>Inspector</Text>
                <View style={s.stack}>
                  <Text style={s.tinyLabel}>density</Text>
                  <Text style={s.sectionText}>{density}</Text>
                </View>
                <View style={s.stack}>
                  <Text style={s.tinyLabel}>row mode</Text>
                  <Text style={s.sectionText}>{rowMode}</Text>
                </View>
                <View style={s.stack}>
                  <Text style={s.tinyLabel}>wrap</Text>
                  <Text style={s.sectionText}>{wrapMode}</Text>
                </View>
                <View style={s.stack}>
                  <Text style={s.tinyLabel}>basis / gap</Text>
                  <Text style={s.sectionText}>{basis}px / {gap}px</Text>
                </View>
                <View style={{ ...s.tokenRow, gap }}>
                  {TOKENS.map((token) => (
                    <View key={token} style={{ ...s.token, backgroundColor: "#142737", borderColor: "#24485f" }}>
                      <Text style={s.tokenText}>{token}</Text>
                    </View>
                  ))}
                </View>
              </View>
            ) : null}
          </View>
          </View>
        </View>
      </ScrollView>
    </SafeAreaView>
  );
}

mount(React.createElement(App));
