#pragma once

// Pure C++ (no Geode types): turns a description of the layout into GD object
// strings. Bridge.cpp feeds it real editor objects and hands the result to
// LevelEditorLayer::createObjectsFromString. Because it has no game
// dependencies it can be (and is) tested natively - see tests/.

#include "Presets.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace autodeco {

enum class Kind : uint8_t {
    Decoration,  // ignored entirely
    Solid,       // full 30x30 block: occupies its cell and gets edge decoration
    Partial,     // slab / slope / scaled / rotated: occupies its cell, no edge decoration
    Blocked      // hazard, orb, pad, portal, trigger...: cell is kept free of decoration
};

struct LayoutObject {
    float x = 0.f, y = 0.f;     // GD position (30 units per block)
    Kind  kind = Kind::Blocked;
    bool  selected = true;      // false = not an edge source (but still counts as occupied)
};

struct Options {
    int      presetIndex = 0;
    float    density = 0.6f;    // 0..1
    uint32_t seed = 1;

    bool outlines    = true;
    bool glow        = true;
    bool topDecor    = true;
    bool groundDecor = true;
    bool hangDecor   = true;
    bool sideDecor   = false;
    bool props       = true;
    bool background  = true;
    bool colors      = true;    // add colour triggers that apply the palette

    // UI-level flags (used by the editor bridge, ignored by the generator)
    bool selectedOnly = false;
    bool replaceLast  = true;

    int  baseChannel   = 981;   // deco colour channels: base .. base+5
    bool recolorLayout = true;  // also colour the built-in "Obj" channel (default blocks)
    int  editorLayer   = 0;     // 0 = leave on the default editor layer
    int  maxObjects    = 8000;
};

struct Output {
    std::vector<std::string> chunks;  // ';'-terminated object strings, split into manageable pieces
    int  objectCount = 0;             // decoration objects (not counting colour triggers)
    int  triggerCount = 0;
    int  sourceCount = 0;             // number of layout blocks that were decorated
    bool capped = false;              // hit Options::maxObjects
    std::string error;                // non-empty -> nothing was generated
};

Output generate(std::vector<LayoutObject> const& layout, Options const& opt);

}  // namespace autodeco
