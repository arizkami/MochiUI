/**
 * AureliaKit React UI — DAW Mixer Demo
 *
 * This is the JS entry point bundled by `bun run bundle:demo` into
 * example/ReactUI/bundle.js, then embedded as a C++ byte array by
 * gen_resources.py and loaded at runtime via res://bundle.js.
 */
import React, { useState } from "react";
import {
  View,
  Text,
  Button,
  TouchableOpacity,
  ScrollView,
  TextInput,
  Switch,
  Slider,
  FlatList,
  SafeAreaView,
  StyleSheet,
  mount,
} from "./index";

// ── Types ─────────────────────────────────────────────────────────────────────

interface Channel {
  id: string;
  name: string;
  bus: string;
  volume: number; // 0–1
  pan: number;    // -1..1
  muted: boolean;
  solo: boolean;
}

// ── Initial data ──────────────────────────────────────────────────────────────

const INITIAL_CHANNELS: Channel[] = [
  { id: "1", name: "Kick",       bus: "Drums", volume: 0.85, pan:  0.00, muted: false, solo: false },
  { id: "2", name: "Snare",      bus: "Drums", volume: 0.70, pan:  0.00, muted: false, solo: false },
  { id: "3", name: "Hi-Hat",     bus: "Drums", volume: 0.55, pan:  0.15, muted: false, solo: false },
  { id: "4", name: "Bass",       bus: "Bass",  volume: 0.90, pan:  0.00, muted: false, solo: false },
  { id: "5", name: "Lead Synth", bus: "Keys",  volume: 0.75, pan: -0.20, muted: false, solo: false },
  { id: "6", name: "Pad",        bus: "Keys",  volume: 0.50, pan:  0.10, muted: false, solo: false },
  { id: "7", name: "Vocals",     bus: "Vox",   volume: 0.80, pan:  0.00, muted: false, solo: false },
  { id: "8", name: "FX Return",  bus: "Aux",   volume: 0.40, pan:  0.00, muted: false, solo: false },
];

// ── Styles ────────────────────────────────────────────────────────────────────

const s = StyleSheet.create({
  root: {
    flex: 1,
    flexDirection: "column" as const,
    backgroundColor: "#111114",
    padding: 12,
    gap: 10,
  },
  header: {
    flexDirection: "row" as const,
    alignItems: "center" as const,
    justifyContent: "space-between" as const,
    marginBottom: 4,
  },
  title: {
    fontSize: 20,
    fontWeight: "bold" as const,
    color: "#e8e8f0",
  },
  projectInput: {
    flex: 1,
    marginHorizontal: 12,
    padding: 6,
    backgroundColor: "#1e1e28",
    borderRadius: 4,
    fontSize: 14,
    color: "#c0c0d0",
  },
  masterCard: {
    backgroundColor: "#1a1a24",
    borderRadius: 8,
    padding: 14,
    gap: 10,
  },
  sectionLabel: {
    fontSize: 11,
    fontWeight: "bold" as const,
    color: "#6060a0",
  },
  row: {
    flexDirection: "row" as const,
    alignItems: "center" as const,
    gap: 8,
  },
  flexLabel: {
    fontSize: 13,
    color: "#c0c0d0",
    flex: 1,
  },
  actionRow: {
    flexDirection: "row" as const,
    gap: 6,
    marginTop: 4,
  },
  strip: {
    backgroundColor: "#1a1a24",
    borderRadius: 6,
    padding: 10,
    marginBottom: 8,
    gap: 6,
  },
  stripHeader: {
    flexDirection: "row" as const,
    alignItems: "center" as const,
    justifyContent: "space-between" as const,
  },
  channelName: {
    fontSize: 13,
    fontWeight: "bold" as const,
    color: "#dcdce8",
  },
  channelBus: {
    fontSize: 11,
    color: "#6060a0",
    marginTop: 1,
  },
  pillRow: {
    flexDirection: "row" as const,
    gap: 4,
  },
  pill: {
    borderRadius: 4,
    paddingHorizontal: 6,
    paddingVertical: 2,
  },
  pillText: {
    fontSize: 11,
    fontWeight: "bold" as const,
  },
  paramRow: {
    flexDirection: "row" as const,
    alignItems: "center" as const,
    gap: 6,
  },
  paramLabel: {
    fontSize: 11,
    color: "#808090",
    minWidth: 54,
  },
  sectionTitle: {
    fontSize: 12,
    fontWeight: "bold" as const,
    color: "#6060a0",
    marginBottom: 4,
  },
});

// ── Channel strip ─────────────────────────────────────────────────────────────

interface ChannelStripProps {
  ch: Channel;
  onVolumeChange: (id: string, v: number) => void;
  onPanChange:    (id: string, v: number) => void;
  onMuteToggle:   (id: string) => void;
  onSoloToggle:   (id: string) => void;
}

function ChannelStrip({ ch, onVolumeChange, onPanChange, onMuteToggle, onSoloToggle }: ChannelStripProps) {
  const volPct = Math.round(ch.volume * 100);
  const panStr = ch.pan === 0
    ? "C"
    : ch.pan < 0
    ? `L${Math.round(-ch.pan * 100)}`
    : `R${Math.round(ch.pan * 100)}`;

  return (
    <View style={s.strip}>
      <View style={s.stripHeader}>
        <View>
          <Text style={s.channelName}>{ch.name}</Text>
          <Text style={s.channelBus}>{ch.bus}</Text>
        </View>
        <View style={s.pillRow}>
          <TouchableOpacity onPress={() => onMuteToggle(ch.id)}>
            <View style={{ ...s.pill, backgroundColor: ch.muted ? "#e05050" : "#2a2a38" }}>
              <Text style={{ ...s.pillText, color: ch.muted ? "#ffffff" : "#808090" }}>M</Text>
            </View>
          </TouchableOpacity>
          <TouchableOpacity onPress={() => onSoloToggle(ch.id)}>
            <View style={{ ...s.pill, backgroundColor: ch.solo ? "#d0a020" : "#2a2a38" }}>
              <Text style={{ ...s.pillText, color: ch.solo ? "#ffffff" : "#808090" }}>S</Text>
            </View>
          </TouchableOpacity>
        </View>
      </View>

      <View style={s.paramRow}>
        <Text style={s.paramLabel}>VOL  {volPct}%</Text>
        <Slider
          value={ch.volume}
          minimumValue={0}
          maximumValue={1}
          onValueChange={(v) => onVolumeChange(ch.id, v)}
          style={{ flex: 1 }}
        />
      </View>

      <View style={s.paramRow}>
        <Text style={s.paramLabel}>PAN  {panStr}</Text>
        <Slider
          value={(ch.pan + 1) / 2}
          minimumValue={0}
          maximumValue={1}
          onValueChange={(v) => onPanChange(ch.id, v * 2 - 1)}
          style={{ flex: 1 }}
        />
      </View>
    </View>
  );
}

// ── Master section ────────────────────────────────────────────────────────────

interface MasterProps {
  volume:          number;
  enabled:         boolean;
  onVolumeChange:  (v: number) => void;
  onEnabledChange: (v: boolean) => void;
}

function MasterSection({ volume, enabled, onVolumeChange, onEnabledChange }: MasterProps) {
  return (
    <View style={s.masterCard}>
      <Text style={s.sectionLabel}>MASTER OUTPUT</Text>

      <View style={s.row}>
        <Text style={s.flexLabel}>Master — {Math.round(volume * 100)}%</Text>
        <Switch value={enabled} onValueChange={onEnabledChange} />
      </View>

      <Slider value={volume} minimumValue={0} maximumValue={1} onValueChange={onVolumeChange} />

      <View style={s.actionRow}>
        <Button title="▶  Play"    onPress={() => console.log("Play")} />
        <Button title="■  Stop"    onPress={() => console.log("Stop")} />
        <Button title="⏺  Record"  onPress={() => console.log("Record")} />
        <Button title="⬆  Export"  onPress={() => console.log("Export")} />
      </View>
    </View>
  );
}

// ── App ───────────────────────────────────────────────────────────────────────

function App() {
  const [project,       setProject]       = useState("Untitled Session");
  const [masterVolume,  setMasterVolume]  = useState(0.80);
  const [masterEnabled, setMasterEnabled] = useState(true);
  const [channels,      setChannels]      = useState<Channel[]>(INITIAL_CHANNELS);

  function updateChannel<K extends keyof Channel>(id: string, key: K, value: Channel[K]) {
    setChannels((prev) => prev.map((ch) => ch.id === id ? { ...ch, [key]: value } : ch));
  }

  return (
    <SafeAreaView style={s.root}>
      {/* ── Header ── */}
      <View style={s.header}>
        <Text style={s.title}>AureliaKit</Text>
        <TextInput
          value={project}
          placeholder="Project name…"
          onChangeText={setProject}
          style={s.projectInput}
        />
        <Text style={{ fontSize: 12, color: "#6060a0" }}>v1.0</Text>
      </View>

      {/* ── Master ── */}
      <MasterSection
        volume={masterVolume}
        enabled={masterEnabled}
        onVolumeChange={setMasterVolume}
        onEnabledChange={setMasterEnabled}
      />

      {/* ── Channel list ── */}
      <Text style={s.sectionTitle}>CHANNELS ({channels.length})</Text>
      <FlatList
        data={channels}
        keyExtractor={(ch) => ch.id}
        style={{ flex: 1 }}
        renderItem={({ item }) => (
          <ChannelStrip
            ch={item}
            onVolumeChange={(id, v) => updateChannel(id, "volume", v)}
            onPanChange={   (id, v) => updateChannel(id, "pan", v)}
            onMuteToggle={  (id)    => updateChannel(id, "muted", !item.muted)}
            onSoloToggle={  (id)    => updateChannel(id, "solo",  !item.solo)}
          />
        )}
      />
    </SafeAreaView>
  );
}

// ── Entry ─────────────────────────────────────────────────────────────────────
mount(React.createElement(App));
