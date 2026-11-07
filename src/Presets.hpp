#pragma once

// Pure C++ (no Geode types) so the generator can be unit-tested natively.

#include <cstdint>
#include <string>
#include <vector>

namespace autodeco {

// Which colour channel a piece uses. The deco channels are allocated from
// `Options::baseChannel` upward (Primary = base, Secondary = base+1, ...).
enum class Slot : int {
    None      = -1,
    Primary   = 0,
    Secondary = 1,
    Accent    = 2,
    Dark      = 3,
    Light     = 4,
    Glow      = 5,   // additive-blending channel
    Layout    = 6    // built-in "Obj" channel (1004) used by default blocks
};

constexpr int kDecoChannelCount = 6;

struct Rgb {
    uint8_t r = 0, g = 0, b = 0;
};

struct Palette {
    // built-in channels
    Rgb bg, ground, ground2, line, layout;
    // custom deco channels
    Rgb primary, secondary, accent, dark, light, glow;
};

// One decoration object that a preset may place.
struct Piece {
    int   id        = 1;
    float weight    = 1.f;   // relative pick chance inside its list
    float scaleMin  = 1.f;
    float scaleMax  = 1.f;
    float length    = 30.f;  // size along the surface normal in GD units (30 = 1 block)
    float offset    = 0.f;   // extra push away from the surface
    Slot  main      = Slot::Primary;
    Slot  detail    = Slot::None;  // None -> same channel as `main`
    int   zLayer    = 3;     // GD z-layer value: -3=B4 -1=B3 1=B2 3=B1 5=T1
    int   zOrder    = 0;
    bool  orient    = true;  // rotate to follow the surface it sits on
    bool  randomRot = false; // random rotation (gears, shapes)
    bool  flipX     = true;  // randomly mirror for variety
    bool  sideOk    = false; // may also be placed on vertical walls
};

struct Preset {
    std::string name;
    std::string description;   // up to two lines, separated by '\n'
    Palette     palette;

    int   outlineId   = 468;   // "Outline Top"
    Slot  outlineSlot = Slot::Accent;
    int   glowId      = 1292;  // "Straight Small Glow"
    Slot  glowSlot    = Slot::Glow;

    // Base probabilities (scaled by the density slider)
    float topChance  = 0.4f;
    float hangChance = 0.25f;
    float sideChance = 0.15f;
    float propChance = 0.05f;
    float bgChance   = 0.6f;

    std::vector<Piece> top;    // grows up out of upward-facing surfaces
    std::vector<Piece> hang;   // hangs from downward-facing surfaces
    std::vector<Piece> props;  // bigger accents (gears, skulls...)
    std::vector<Piece> bg;     // scattered behind the layout

    bool randomPalette = false;  // "Chaos" preset
};

// All built-in presets. Slaughterhouse and Acheron come first.
std::vector<Preset> const& presets();

// Builds a random preset (palette + piece mix) from a seed.
Preset makeChaos(uint32_t seed);

// True if `id` is used by any built-in preset (so it is never mistaken for layout).
bool isDecoId(int id);

}  // namespace autodeco
