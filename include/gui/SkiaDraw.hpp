#pragma once
// Public re-export of all Skia drawing types needed for custom draw() overrides.
// Include this (or any AUK* umbrella) instead of Skia headers directly.

// ── Geometry ──────────────────────────────────────────────────────────────────
#include <include/core/SkRect.h>
#include <include/core/SkRRect.h>
#include <include/core/SkPoint.h>
#include <include/core/SkSize.h>

// ── Canvas + Paint ────────────────────────────────────────────────────────────
#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkColor.h>
#include <include/core/SkColorSpace.h>

// ── Paths ─────────────────────────────────────────────────────────────────────
#include <include/core/SkPath.h>
#include <include/core/SkPathBuilder.h>

// ── Text ──────────────────────────────────────────────────────────────────────
#include <include/core/SkFont.h>
#include <include/core/SkFontMetrics.h>
#include <include/core/SkFontStyle.h>
#include <include/core/SkFontMgr.h>
#include <include/core/SkTypeface.h>

// ── Shaders / Effects ─────────────────────────────────────────────────────────
#include <include/core/SkShader.h>
#include <include/core/SkMaskFilter.h>
#include <include/core/SkBlurTypes.h>
#include <include/effects/SkGradient.h>
#include <include/effects/SkRuntimeEffect.h>

// ── Utilities ─────────────────────────────────────────────────────────────────
#include <include/utils/SkParsePath.h>

// ── Surface ───────────────────────────────────────────────────────────────────
#include <include/core/SkSurface.h>
#include <include/core/SkSurfaceProps.h>
