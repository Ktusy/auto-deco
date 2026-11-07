#include "Presets.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <unordered_set>

// =============================================================================
//  PRESET TABLE
//
//  Every object ID below comes from the community-maintained 2.2 object ID list
//  (names in the comments). All of them are non-solid, non-hazard decorations.
//
//  The "length" of a piece is its approximate height along the surface it sits
//  on, in GD units (30 = one block). It is only used to place the piece flush
//  against the surface - if something floats a bit or sinks a bit in your
//  game, tweak `length` or `offset` here and rebuild. Nothing else needs to
//  change.
//
//  z-layers:  -3 = B4, -1 = B3, 1 = B2, 3 = B1, 5 = T1
// =============================================================================

namespace autodeco {
namespace {

constexpr Rgb rgb(int r, int g, int b) {
    return Rgb{static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b)};
}

Piece P(int id, float weight, float s0, float s1, float length, Slot main,
        Slot detail = Slot::None, int z = 3, int zo = 0) {
    Piece p;
    p.id = id;
    p.weight = weight;
    p.scaleMin = s0;
    p.scaleMax = s1;
    p.length = length;
    p.main = main;
    p.detail = detail;
    p.zLayer = z;
    p.zOrder = zo;
    return p;
}

Piece side(Piece p)  { p.sideOk = true;     return p; }  // can grow out of walls too
Piece fixed(Piece p) { p.orient = false;    return p; }  // never rotates (chains)
Piece spin(Piece p)  { p.randomRot = true;  return p; }  // random rotation (gears)
Piece noFlip(Piece p){ p.flipX = false;     return p; }

// ----------------------------------------------------------------------------
Preset slaughterhouse() {
    Preset p;
    p.name = "Slaughterhouse";
    p.description = "Blood-red & black. Thorns, chains, skulls and\nbig grinding gears. Aggressive and heavy.";
    p.palette = Palette{
        .bg = rgb(16, 4, 7),      .ground = rgb(28, 8, 12), .ground2 = rgb(46, 10, 16),
        .line = rgb(255, 38, 38), .layout = rgb(34, 12, 16),
        .primary = rgb(176, 16, 28), .secondary = rgb(92, 6, 14), .accent = rgb(255, 60, 60),
        .dark = rgb(8, 2, 4), .light = rgb(228, 214, 196), .glow = rgb(255, 48, 48)};
    p.outlineId = 1202;  // Thick Outline Top
    p.outlineSlot = Slot::Accent;
    p.glowId = 503;      // Straight Medium Glow
    p.topChance = 0.50f; p.hangChance = 0.40f; p.sideChance = 0.20f;
    p.propChance = 0.08f; p.bgChance = 0.60f;

    p.top = {
        side(P(128, 2.0f, .9f, 1.2f, 36, Slot::Primary, Slot::Secondary)),   // Very Large Decorative Thorn
        side(P(123, 2.5f, .9f, 1.2f, 30, Slot::Primary, Slot::Secondary)),   // Large Decorative Thorn
        side(P(124, 2.0f, .9f, 1.1f, 26, Slot::Secondary)),                  // Medium-Large Decorative Thorn
        P(1889, 3.0f, .85f, 1.0f, 30, Slot::Primary),                        // Fake Spike
        P(1890, 2.0f, .9f, 1.0f, 15, Slot::Primary),                         // Half Fake Spike
        P(18, 1.5f, 1.0f, 1.0f, 30, Slot::Dark, Slot::Primary),              // Large Decorative Spikes
        P(19, 1.0f, 1.0f, 1.0f, 24, Slot::Dark, Slot::Primary),              // Medium Decorative Spikes
    };
    p.hang = {
        fixed(P(41, 3.0f, 1.f, 1.f, 75, Slot::Dark, Slot::Secondary, 1, -2)),   // Tall Chain
        fixed(P(110, 2.0f, 1.f, 1.f, 45, Slot::Dark, Slot::Secondary, 1, -2)),  // Small Chain
        fixed(P(106, 1.0f, 1.f, 1.f, 60, Slot::Dark, Slot::Secondary, 1, -2)),  // Large Wide Chain
        P(1889, 2.0f, .8f, 1.0f, 30, Slot::Primary),                            // Fake Spike (points down)
        P(123, 2.0f, .9f, 1.2f, 30, Slot::Primary, Slot::Secondary),            // thorn (points down)
    };
    p.props = {
        spin(P(85, 2.0f, 1.4f, 2.4f, 30, Slot::Secondary, Slot::Dark, -1, -3)),  // Large Decorative Gear
        spin(P(86, 1.5f, 1.4f, 2.4f, 30, Slot::Secondary, Slot::Dark, -1, -3)),  // Medium Decorative Gear
        P(1596, 1.5f, .9f, 1.2f, 30, Slot::Light, Slot::None, 3, 4),             // Skull Decoration
        P(1597, 1.5f, .9f, 1.2f, 30, Slot::Light, Slot::None, 3, 4),             // Skull Decoration
    };
    p.bg = {
        spin(P(85, 2.0f, 2.5f, 5.5f, 30, Slot::Secondary, Slot::Dark, -3, -8)),
        spin(P(86, 1.5f, 2.5f, 4.5f, 30, Slot::Secondary, Slot::Dark, -3, -8)),
        spin(P(18, 1.5f, 3.0f, 6.0f, 30, Slot::Dark, Slot::Secondary, -3, -9)),
        P(51, 1.0f, 1.5f, 4.0f, 30, Slot::Primary, Slot::Primary, -3, -7),       // Pulsing Circle
        P(53, 1.0f, 1.5f, 3.5f, 30, Slot::Secondary, Slot::Secondary, -3, -7),   // Pulsing Diamond
        fixed(P(41, 1.2f, 1.0f, 1.6f, 75, Slot::Dark, Slot::Secondary, -3, -9)), // Tall Chain
    };
    return p;
}

// ----------------------------------------------------------------------------
Preset acheron() {
    Preset p;
    p.name = "Acheron";
    p.description = "Charcoal grey with red accents. Thorns, rods,\npulsing shapes and slow gears (1.9-style).";
    p.palette = Palette{
        .bg = rgb(13, 13, 16),     .ground = rgb(24, 24, 29), .ground2 = rgb(34, 34, 41),
        .line = rgb(214, 36, 48),  .layout = rgb(40, 40, 50),
        .primary = rgb(84, 84, 98), .secondary = rgb(46, 46, 56), .accent = rgb(230, 48, 60),
        .dark = rgb(9, 9, 11), .light = rgb(180, 180, 196), .glow = rgb(255, 64, 74)};
    p.outlineId = 468;   // Outline Top
    p.outlineSlot = Slot::Accent;
    p.glowId = 1292;     // Straight Small Glow
    p.topChance = 0.42f; p.hangChance = 0.30f; p.sideChance = 0.15f;
    p.propChance = 0.06f; p.bgChance = 0.55f;

    p.top = {
        side(P(123, 2.5f, .9f, 1.2f, 30, Slot::Secondary, Slot::Primary)),
        side(P(125, 2.0f, .9f, 1.1f, 26, Slot::Secondary, Slot::Primary)),   // Medium-Small Decorative Thorn
        side(P(126, 2.0f, .9f, 1.1f, 22, Slot::Primary)),                    // Small Decorative Thorn
        P(1891, 2.0f, .9f, 1.0f, 22, Slot::Dark, Slot::Secondary),           // Small Fake Spike
        P(20, 1.5f, 1.0f, 1.0f, 20, Slot::Dark, Slot::Primary),              // Small Decorative Spikes
        P(406, 1.5f, .9f, 1.1f, 20, Slot::Primary),                          // Patch of Grass
        P(151, 1.0f, 1.0f, 1.0f, 90, Slot::Secondary, Slot::Primary),        // Tall Spike Rod
    };
    p.hang = {
        fixed(P(41, 3.0f, 1.f, 1.f, 75, Slot::Secondary, Slot::Dark, 1, -2)),
        fixed(P(110, 2.0f, 1.f, 1.f, 45, Slot::Secondary, Slot::Dark, 1, -2)),
        P(16, 1.5f, 1.f, 1.f, 60, Slot::Secondary, Slot::Primary),           // Medium Rod
        P(19, 1.5f, 1.f, 1.f, 24, Slot::Dark, Slot::Secondary),
    };
    p.props = {
        spin(P(85, 2.0f, 1.2f, 2.2f, 30, Slot::Primary, Slot::Secondary, -1, -3)),
        spin(P(87, 1.5f, 1.2f, 2.0f, 30, Slot::Primary, Slot::Secondary, -1, -3)),   // Small Decorative Gear
        P(51, 1.0f, 1.0f, 1.6f, 30, Slot::Accent, Slot::Accent, 3, 4),
        P(1473, 1.0f, .9f, 1.1f, 30, Slot::Secondary, Slot::Dark),                    // Large Rock Pile
    };
    p.bg = {
        P(50, 1.5f, 1.5f, 4.5f, 30, Slot::Secondary, Slot::Secondary, -3, -8),   // Pulsing Filled Circle
        P(51, 1.5f, 1.5f, 4.5f, 30, Slot::Secondary, Slot::Secondary, -3, -8),   // Pulsing Circle
        P(148, 1.0f, 1.5f, 3.5f, 30, Slot::Secondary, Slot::Secondary, -3, -8),  // Pulsing Square
        P(149, 1.0f, 1.5f, 3.5f, 30, Slot::Secondary, Slot::Secondary, -3, -8),  // Pulsing Triangle
        P(130, 1.0f, 1.5f, 2.5f, 30, Slot::Dark, Slot::Dark, -3, -9),            // Large Cloud
        P(131, 1.0f, 1.5f, 2.5f, 30, Slot::Dark, Slot::Dark, -3, -9),            // Small Cloud
        fixed(P(15, 1.2f, 1.0f, 1.6f, 90, Slot::Secondary, Slot::Primary, -3, -9)),  // Tall Rod
        spin(P(85, 1.5f, 3.0f, 6.0f, 30, Slot::Secondary, Slot::Dark, -3, -9)),
    };
    return p;
}

// ----------------------------------------------------------------------------
Preset bloodbath() {
    Preset p;
    p.name = "Bloodbath";
    p.description = "Classic red / black / grey. Thorns, gears, chains\nand rods in the old-school collab style.";
    p.palette = Palette{
        .bg = rgb(10, 10, 10),     .ground = rgb(22, 22, 22), .ground2 = rgb(34, 16, 16),
        .line = rgb(200, 20, 20),  .layout = rgb(52, 52, 52),
        .primary = rgb(170, 20, 20), .secondary = rgb(70, 70, 70), .accent = rgb(230, 30, 30),
        .dark = rgb(6, 6, 6), .light = rgb(200, 200, 200), .glow = rgb(255, 40, 40)};
    p.outlineId = 468; p.glowId = 1292;
    p.topChance = 0.45f; p.hangChance = 0.30f; p.sideChance = 0.18f;
    p.propChance = 0.07f; p.bgChance = 0.55f;

    p.top = {
        side(P(123, 2.0f, .9f, 1.2f, 30, Slot::Primary)),
        side(P(124, 2.0f, .9f, 1.1f, 26, Slot::Primary)),
        side(P(125, 2.0f, .9f, 1.1f, 26, Slot::Primary)),
        side(P(127, 1.5f, .9f, 1.1f, 16, Slot::Primary)),                     // Very Small Decorative Thorn
        P(1889, 2.0f, .9f, 1.0f, 30, Slot::Primary),
        P(21, 1.0f, 1.0f, 1.0f, 12, Slot::Dark, Slot::Secondary),             // Very Small Decorative Spikes
    };
    p.hang = {
        fixed(P(41, 3.0f, 1.f, 1.f, 75, Slot::Secondary, Slot::Dark, 1, -2)),
        fixed(P(110, 2.0f, 1.f, 1.f, 45, Slot::Secondary, Slot::Dark, 1, -2)),
        P(17, 1.5f, 1.f, 1.f, 30, Slot::Secondary, Slot::Primary),            // Short Rod
        P(123, 1.5f, .9f, 1.2f, 30, Slot::Primary),
    };
    p.props = {
        spin(P(85, 2.0f, 1.3f, 2.2f, 30, Slot::Secondary, Slot::Dark, -1, -3)),
        spin(P(86, 1.5f, 1.3f, 2.2f, 30, Slot::Secondary, Slot::Dark, -1, -3)),
        spin(P(87, 1.5f, 1.3f, 2.0f, 30, Slot::Secondary, Slot::Dark, -1, -3)),
        P(1596, 0.7f, .9f, 1.1f, 30, Slot::Light, Slot::None, 3, 4),
    };
    p.bg = {
        spin(P(85, 2.0f, 2.5f, 5.5f, 30, Slot::Secondary, Slot::Dark, -3, -8)),
        spin(P(86, 1.5f, 2.5f, 4.5f, 30, Slot::Secondary, Slot::Dark, -3, -8)),
        P(130, 1.0f, 1.5f, 2.5f, 30, Slot::Dark, Slot::Dark, -3, -9),
        P(50, 1.0f, 1.5f, 4.0f, 30, Slot::Primary, Slot::Primary, -3, -7),
        P(52, 0.7f, 1.5f, 3.0f, 30, Slot::Primary, Slot::Primary, -3, -7),   // Pulsing Heart
    };
    return p;
}

// ----------------------------------------------------------------------------
Preset nineCircles() {
    Preset p;
    p.name = "Nine Circles";
    p.description = "Molten orange glow. Big light orbs, pulses,\nneon outlines and warm thorns.";
    p.palette = Palette{
        .bg = rgb(12, 3, 2),        .ground = rgb(28, 8, 4),  .ground2 = rgb(44, 12, 4),
        .line = rgb(255, 140, 40),  .layout = rgb(30, 10, 6),
        .primary = rgb(255, 90, 20), .secondary = rgb(150, 30, 8), .accent = rgb(255, 170, 60),
        .dark = rgb(10, 3, 2), .light = rgb(255, 226, 170), .glow = rgb(255, 110, 30)};
    p.outlineId = 1191;  // Neon Outline Top
    p.outlineSlot = Slot::Accent;
    p.glowId = 1011;     // Straight Large Glow
    p.topChance = 0.40f; p.hangChance = 0.25f; p.sideChance = 0.15f;
    p.propChance = 0.12f; p.bgChance = 0.55f;

    p.top = {
        side(P(123, 2.0f, .9f, 1.2f, 30, Slot::Primary, Slot::Secondary)),
        side(P(125, 2.0f, .9f, 1.1f, 26, Slot::Primary, Slot::Secondary)),
        P(1889, 2.0f, .85f, 1.0f, 30, Slot::Primary),
        P(18, 1.0f, 1.0f, 1.0f, 30, Slot::Secondary, Slot::Primary),
        P(1887, 1.5f, .8f, 1.2f, 30, Slot::Glow, Slot::Glow, 3, 2),             // Small Glow Orb
    };
    p.hang = {
        P(15, 1.5f, 1.f, 1.f, 90, Slot::Secondary, Slot::Primary),
        P(16, 1.5f, 1.f, 1.f, 60, Slot::Secondary, Slot::Primary),
        P(17, 1.5f, 1.f, 1.f, 30, Slot::Secondary, Slot::Primary),
        P(1889, 1.5f, .8f, 1.0f, 30, Slot::Primary),
    };
    p.props = {
        P(1888, 3.0f, 1.0f, 1.8f, 30, Slot::Glow, Slot::Glow, 3, 3),            // Large Glow Orb
        P(1886, 3.0f, 1.0f, 1.8f, 30, Slot::Glow, Slot::Glow, 3, 3),            // Medium Glow Orb
        P(51, 1.5f, 1.0f, 1.8f, 30, Slot::Accent, Slot::Accent, 3, 4),
        P(497, 1.0f, .8f, 1.2f, 30, Slot::Primary, Slot::Primary, 3, 4),        // Large Pulsing Circle
    };
    p.bg = {
        P(1888, 3.0f, 2.0f, 6.0f, 30, Slot::Glow, Slot::Glow, -3, -8),
        P(1886, 2.0f, 2.0f, 5.0f, 30, Slot::Glow, Slot::Glow, -3, -8),
        P(497, 1.5f, 1.5f, 4.0f, 30, Slot::Secondary, Slot::Secondary, -3, -7),
        P(51, 1.5f, 1.5f, 4.0f, 30, Slot::Secondary, Slot::Secondary, -3, -7),
        spin(P(1058, 1.0f, 1.5f, 3.5f, 30, Slot::Secondary, Slot::Secondary, -3, -9)),  // Large Swirl
        spin(P(123, 1.0f, 2.5f, 5.0f, 30, Slot::Secondary, Slot::Dark, -3, -9)),
    };
    return p;
}

// ----------------------------------------------------------------------------
Preset sonicWave() {
    Preset p;
    p.name = "Sonic Wave";
    p.description = "Electric cyan neon. Pulsing diamonds, clouds,\nglow edges and crisp decorative spikes.";
    p.palette = Palette{
        .bg = rgb(4, 10, 24),      .ground = rgb(8, 20, 44),  .ground2 = rgb(12, 30, 60),
        .line = rgb(40, 220, 255), .layout = rgb(14, 30, 60),
        .primary = rgb(20, 150, 255), .secondary = rgb(10, 70, 160), .accent = rgb(60, 230, 255),
        .dark = rgb(3, 8, 20), .light = rgb(200, 245, 255), .glow = rgb(40, 200, 255)};
    p.outlineId = 468; p.glowId = 503;
    p.topChance = 0.35f; p.hangChance = 0.20f; p.sideChance = 0.10f;
    p.propChance = 0.08f; p.bgChance = 0.60f;

    p.top = {
        P(1889, 2.0f, .85f, 1.0f, 30, Slot::Accent, Slot::Primary),
        P(1891, 2.0f, .9f, 1.0f, 22, Slot::Primary),
        P(1892, 1.5f, .9f, 1.0f, 12, Slot::Accent),                               // Tiny Fake Spike
        P(19, 1.5f, 1.0f, 1.0f, 24, Slot::Secondary, Slot::Primary),
        P(134, 1.0f, 1.0f, 1.0f, 40, Slot::Primary, Slot::Accent),               // Small Diamond Rod
        P(190, 0.8f, 1.0f, 1.0f, 90, Slot::Primary, Slot::Accent),               // Tall Diamond Rod
    };
    p.hang = {
        P(134, 2.0f, 1.f, 1.f, 40, Slot::Primary, Slot::Accent),
        P(1889, 1.5f, .8f, 1.0f, 30, Slot::Accent, Slot::Primary),
        P(20, 1.5f, 1.f, 1.f, 20, Slot::Secondary, Slot::Primary),
    };
    p.props = {
        P(53, 2.0f, 1.0f, 1.6f, 30, Slot::Accent, Slot::Accent, 3, 4),           // Pulsing Diamond
        P(54, 1.5f, 1.0f, 1.6f, 30, Slot::Accent, Slot::Accent, 3, 4),           // Pulsing Star
        P(1887, 1.5f, 1.0f, 1.5f, 30, Slot::Glow, Slot::Glow, 3, 3),
    };
    p.bg = {
        P(53, 1.5f, 1.5f, 4.0f, 30, Slot::Secondary, Slot::Secondary, -3, -7),
        P(54, 1.0f, 1.5f, 3.5f, 30, Slot::Secondary, Slot::Secondary, -3, -7),
        P(50, 1.5f, 1.5f, 4.0f, 30, Slot::Secondary, Slot::Secondary, -3, -7),
        P(497, 1.0f, 1.5f, 3.5f, 30, Slot::Secondary, Slot::Secondary, -3, -7),
        P(223, 1.5f, 1.5f, 3.0f, 30, Slot::Dark, Slot::Dark, -3, -9),            // Medium Round Cloud
        P(224, 1.5f, 1.5f, 3.0f, 30, Slot::Dark, Slot::Dark, -3, -9),            // Small Round Cloud
        P(149, 1.0f, 1.5f, 3.5f, 30, Slot::Secondary, Slot::Secondary, -3, -8),
    };
    return p;
}

// ----------------------------------------------------------------------------
Preset toxicFactory() {
    Preset p;
    p.name = "Toxic Factory";
    p.description = "Industrial green. Gears, spike rods, chains\nand jagged decorative spikes.";
    p.palette = Palette{
        .bg = rgb(5, 12, 6),        .ground = rgb(10, 24, 12), .ground2 = rgb(16, 36, 18),
        .line = rgb(120, 255, 60),  .layout = rgb(20, 40, 22),
        .primary = rgb(70, 200, 40), .secondary = rgb(30, 100, 24), .accent = rgb(170, 255, 70),
        .dark = rgb(4, 8, 4), .light = rgb(214, 255, 190), .glow = rgb(110, 255, 50)};
    p.outlineId = 1202; p.glowId = 1292;
    p.topChance = 0.45f; p.hangChance = 0.35f; p.sideChance = 0.15f;
    p.propChance = 0.10f; p.bgChance = 0.55f;

    p.top = {
        P(151, 1.5f, 1.0f, 1.0f, 90, Slot::Secondary, Slot::Primary),             // Tall Spike Rod
        P(152, 1.5f, 1.0f, 1.0f, 60, Slot::Secondary, Slot::Primary),             // Medium Spike Rod
        P(153, 1.5f, 1.0f, 1.0f, 30, Slot::Secondary, Slot::Primary),             // Small Spike Rod
        P(18, 2.0f, 1.0f, 1.0f, 30, Slot::Dark, Slot::Primary),
        P(19, 1.5f, 1.0f, 1.0f, 24, Slot::Dark, Slot::Primary),
        P(1889, 1.5f, .9f, 1.0f, 30, Slot::Primary),
        side(P(128, 1.0f, .9f, 1.1f, 36, Slot::Primary, Slot::Secondary)),
    };
    p.hang = {
        fixed(P(41, 2.0f, 1.f, 1.f, 75, Slot::Dark, Slot::Secondary, 1, -2)),
        fixed(P(106, 1.5f, 1.f, 1.f, 60, Slot::Dark, Slot::Secondary, 1, -2)),
        fixed(P(107, 1.5f, 1.f, 1.f, 40, Slot::Dark, Slot::Secondary, 1, -2)),    // Small Wide Chain
        fixed(P(110, 1.5f, 1.f, 1.f, 45, Slot::Dark, Slot::Secondary, 1, -2)),
        P(152, 1.0f, 1.f, 1.f, 60, Slot::Secondary, Slot::Primary),
    };
    p.props = {
        spin(P(678, 2.0f, 1.2f, 2.2f, 30, Slot::Primary, Slot::Secondary, -1, -3)),  // Large Colored Gear
        spin(P(679, 1.5f, 1.2f, 2.0f, 30, Slot::Primary, Slot::Secondary, -1, -3)),  // Medium Colored Gear
        spin(P(85, 1.5f, 1.2f, 2.2f, 30, Slot::Secondary, Slot::Dark, -1, -3)),
        P(1887, 1.0f, 1.0f, 1.4f, 30, Slot::Glow, Slot::Glow, 3, 3),
    };
    p.bg = {
        spin(P(678, 2.0f, 2.5f, 5.5f, 30, Slot::Secondary, Slot::Dark, -3, -8)),
        spin(P(85, 2.0f, 2.5f, 5.5f, 30, Slot::Secondary, Slot::Dark, -3, -8)),
        spin(P(394, 1.0f, 2.0f, 4.0f, 30, Slot::Secondary, Slot::Secondary, -3, -8)),  // Large Rotating Hexagon
        P(405, 1.0f, 1.5f, 3.5f, 30, Slot::Secondary, Slot::Secondary, -3, -7),        // Pulsing Hexagon
        P(236, 1.0f, 1.5f, 3.5f, 30, Slot::Secondary, Slot::Secondary, -3, -7),        // Large Pulsing Hollow Circle
        fixed(P(16, 1.0f, 1.0f, 1.5f, 60, Slot::Dark, Slot::Secondary, -3, -9)),
    };
    return p;
}

// ----------------------------------------------------------------------------
Preset frostbite() {
    Preset p;
    p.name = "Frostbite";
    p.description = "Icy blue. Icicle spikes, drifting clouds,\npulsing stars and a cold glow.";
    p.palette = Palette{
        .bg = rgb(6, 14, 28),         .ground = rgb(14, 30, 52), .ground2 = rgb(20, 44, 74),
        .line = rgb(170, 230, 255),   .layout = rgb(30, 60, 96),
        .primary = rgb(120, 190, 240), .secondary = rgb(60, 110, 170), .accent = rgb(210, 245, 255),
        .dark = rgb(6, 14, 28), .light = rgb(240, 252, 255), .glow = rgb(150, 220, 255)};
    p.outlineId = 468; p.glowId = 503;
    p.topChance = 0.30f; p.hangChance = 0.45f; p.sideChance = 0.10f;
    p.propChance = 0.07f; p.bgChance = 0.60f;

    p.top = {
        P(1889, 2.0f, .85f, 1.0f, 30, Slot::Light, Slot::Primary),
        P(1890, 1.5f, .9f, 1.0f, 15, Slot::Light, Slot::Primary),
        P(1891, 1.5f, .9f, 1.0f, 22, Slot::Light, Slot::Primary),
        P(20, 1.5f, 1.0f, 1.0f, 20, Slot::Primary, Slot::Light),
        P(1473, 1.0f, .9f, 1.1f, 30, Slot::Secondary, Slot::Primary),
        P(1496, 1.0f, .9f, 1.1f, 20, Slot::Secondary, Slot::Primary),             // Small Rock Pile
    };
    p.hang = {   // icicles
        P(1889, 3.0f, .8f, 1.1f, 30, Slot::Light, Slot::Primary),
        P(1891, 2.0f, .9f, 1.0f, 22, Slot::Light, Slot::Primary),
        P(18, 1.5f, 1.0f, 1.0f, 30, Slot::Primary, Slot::Light),
        P(19, 1.5f, 1.0f, 1.0f, 24, Slot::Primary, Slot::Light),
        P(134, 1.0f, 1.0f, 1.0f, 40, Slot::Primary, Slot::Light),
    };
    p.props = {
        P(54, 2.0f, 1.0f, 1.6f, 30, Slot::Light, Slot::Light, 3, 4),              // Pulsing Star
        P(53, 1.5f, 1.0f, 1.6f, 30, Slot::Light, Slot::Light, 3, 4),
        P(1887, 1.5f, 1.0f, 1.5f, 30, Slot::Glow, Slot::Glow, 3, 3),
    };
    p.bg = {
        P(130, 2.0f, 1.5f, 3.0f, 30, Slot::Secondary, Slot::Secondary, -3, -9),   // Large Cloud
        P(129, 1.5f, 1.5f, 3.0f, 30, Slot::Secondary, Slot::Secondary, -3, -9),   // Medium Cloud
        P(222, 1.5f, 1.5f, 3.0f, 30, Slot::Secondary, Slot::Secondary, -3, -9),   // Large Round Cloud
        P(48, 1.0f, 1.5f, 3.0f, 30, Slot::Secondary, Slot::Secondary, -3, -9),    // Large Fading Cloud
        P(53, 1.0f, 1.5f, 3.5f, 30, Slot::Secondary, Slot::Secondary, -3, -7),
        P(54, 1.0f, 1.5f, 3.5f, 30, Slot::Secondary, Slot::Secondary, -3, -7),
    };
    return p;
}

// ----------------------------------------------------------------------------
Preset neonGrid() {
    Preset p;
    p.name = "Neon Grid";
    p.description = "Magenta + cyan synthwave. Neon outlines,\npulsing squares and floating shapes.";
    p.palette = Palette{
        .bg = rgb(8, 4, 20),         .ground = rgb(18, 8, 40),  .ground2 = rgb(28, 10, 60),
        .line = rgb(255, 60, 220),   .layout = rgb(22, 10, 44),
        .primary = rgb(255, 50, 210), .secondary = rgb(60, 230, 255), .accent = rgb(255, 120, 240),
        .dark = rgb(6, 3, 14), .light = rgb(230, 250, 255), .glow = rgb(255, 60, 220)};
    p.outlineId = 1191;  // Neon Outline Top
    p.outlineSlot = Slot::Accent;
    p.glowId = 1011;
    p.topChance = 0.22f; p.hangChance = 0.15f; p.sideChance = 0.08f;
    p.propChance = 0.10f; p.bgChance = 0.55f;

    p.top = {
        P(1892, 2.0f, .9f, 1.0f, 12, Slot::Secondary),
        P(1891, 1.5f, .9f, 1.0f, 22, Slot::Primary),
        noFlip(P(1764, 2.0f, .8f, 1.2f, 12, Slot::Secondary, Slot::Secondary, 3, 2)),    // Tiny Circle
        noFlip(P(1767, 1.5f, .8f, 1.2f, 12, Slot::Primary, Slot::Primary, 3, 2)),        // Tiny Rectangle
    };
    p.hang = {
        P(17, 1.5f, 1.f, 1.f, 30, Slot::Secondary, Slot::Primary),
        noFlip(P(1764, 2.0f, .8f, 1.2f, 12, Slot::Primary, Slot::Primary, 3, 2)),
        P(1892, 1.0f, .9f, 1.0f, 12, Slot::Primary),
    };
    p.props = {
        P(148, 2.0f, .8f, 1.4f, 30, Slot::Secondary, Slot::Secondary, 3, 4),       // Pulsing Square
        P(150, 1.5f, .8f, 1.4f, 30, Slot::Primary, Slot::Primary, 3, 4),           // Pulsing Cross
        P(149, 1.5f, .8f, 1.4f, 30, Slot::Accent, Slot::Accent, 3, 4),             // Pulsing Triangle
    };
    p.bg = {
        P(495, 1.5f, 1.5f, 4.0f, 30, Slot::Secondary, Slot::Secondary, -3, -8),    // Large Pulsing Square
        P(496, 1.5f, 1.5f, 4.0f, 30, Slot::Primary, Slot::Primary, -3, -8),        // Large Pulsing Hollow Square
        P(3639, 1.0f, 1.0f, 3.0f, 30, Slot::Secondary, Slot::Secondary, -3, -8),   // Large Hollow Square
        P(3636, 1.0f, 1.0f, 3.0f, 30, Slot::Primary, Slot::Primary, -3, -8),       // Large Hollow Circle
        P(3628, 1.0f, 1.0f, 3.0f, 30, Slot::Secondary, Slot::Secondary, -3, -8),   // Triangle
        P(997, 1.0f, 1.0f, 3.0f, 30, Slot::Primary, Slot::Primary, -3, -8),        // Large Split Circle
        noFlip(P(1764, 2.0f, 1.0f, 2.0f, 12, Slot::Accent, Slot::Accent, -3, -7)),
    };
    return p;
}

// ----------------------------------------------------------------------------
Preset minimalClean() {
    Preset p;
    p.name = "Minimal Clean";
    p.description = "Just crisp outlines and a soft edge glow.\nA tidy base for hand decorating.";
    p.palette = Palette{
        .bg = rgb(14, 14, 18),       .ground = rgb(26, 26, 32),  .ground2 = rgb(36, 36, 44),
        .line = rgb(240, 240, 250),  .layout = rgb(46, 46, 58),
        .primary = rgb(150, 160, 190), .secondary = rgb(80, 88, 110), .accent = rgb(255, 255, 255),
        .dark = rgb(8, 8, 10), .light = rgb(240, 240, 250), .glow = rgb(200, 210, 255)};
    p.outlineId = 468; p.glowId = 1292;
    p.topChance = 0.f; p.hangChance = 0.f; p.sideChance = 0.f;
    p.propChance = 0.f; p.bgChance = 0.f;
    return p;
}

// ----------------------------------------------------------------------------
Preset chaosPlaceholder() {
    Preset p;
    p.name = "Chaos (Random)";
    p.description = "Random palette and a random mix every time.\nRoll the seed for endless variations.";
    p.randomPalette = true;
    p.palette = minimalClean().palette;  // only used before the first roll
    return p;
}

std::vector<Preset> buildAll() {
    std::vector<Preset> v;
    v.push_back(slaughterhouse());
    v.push_back(acheron());
    v.push_back(bloodbath());
    v.push_back(nineCircles());
    v.push_back(sonicWave());
    v.push_back(toxicFactory());
    v.push_back(frostbite());
    v.push_back(neonGrid());
    v.push_back(minimalClean());
    v.push_back(chaosPlaceholder());
    return v;
}

// --- tiny HSV helper for the Chaos preset -----------------------------------
Rgb hsv(float h, float s, float v) {
    h = h - std::floor(h);
    float c = v * s;
    float hp = h * 6.f;
    float x = c * (1.f - std::fabs(std::fmod(hp, 2.f) - 1.f));
    float r = 0, g = 0, b = 0;
    if (hp < 1) { r = c; g = x; }
    else if (hp < 2) { r = x; g = c; }
    else if (hp < 3) { g = c; b = x; }
    else if (hp < 4) { g = x; b = c; }
    else if (hp < 5) { r = x; b = c; }
    else { r = c; b = x; }
    float m = v - c;
    auto to8 = [](float f) { return static_cast<int>(std::lround(std::clamp(f, 0.f, 1.f) * 255.f)); };
    return rgb(to8(r + m), to8(g + m), to8(b + m));
}

}  // namespace

std::vector<Preset> const& presets() {
    static std::vector<Preset> const all = buildAll();
    return all;
}

bool isDecoId(int id) {
    static std::unordered_set<int> const ids = [] {
        std::unordered_set<int> s;
        for (auto const& p : presets()) {
            s.insert(p.outlineId);
            s.insert(p.glowId);
            for (auto const* list : {&p.top, &p.hang, &p.props, &p.bg})
                for (auto const& piece : *list) s.insert(piece.id);
        }
        return s;
    }();
    return ids.contains(id);
}

Preset makeChaos(uint32_t seed) {
    std::mt19937 g(seed ^ 0xC4A05u);
    auto rf = [&](float a, float b) {
        return a + (b - a) * static_cast<float>((g() >> 8) * (1.0 / 16777216.0));
    };

    Preset p = chaosPlaceholder();
    float h = rf(0.f, 1.f);
    float h2 = h + rf(0.35f, 0.65f);  // contrasting accent hue
    p.palette = Palette{
        .bg = hsv(h, 0.65f, 0.08f),         .ground = hsv(h, 0.55f, 0.13f),
        .ground2 = hsv(h, 0.50f, 0.19f),    .line = hsv(h2, 0.75f, 1.0f),
        .layout = hsv(h, 0.45f, 0.22f),
        .primary = hsv(h, 0.80f, 0.85f),    .secondary = hsv(h + 0.06f, 0.70f, 0.45f),
        .accent = hsv(h2, 0.85f, 1.0f),     .dark = hsv(h, 0.50f, 0.06f),
        .light = hsv(h, 0.15f, 0.96f),      .glow = hsv(h2, 0.85f, 1.0f)};

    // Pool pieces from the other presets and take a random handful of each kind.
    auto const& all = presets();
    std::vector<Piece> top, hang, props, bg;
    for (size_t i = 0; i + 1 < all.size(); ++i) {  // skip the chaos placeholder itself
        top.insert(top.end(), all[i].top.begin(), all[i].top.end());
        hang.insert(hang.end(), all[i].hang.begin(), all[i].hang.end());
        props.insert(props.end(), all[i].props.begin(), all[i].props.end());
        bg.insert(bg.end(), all[i].bg.begin(), all[i].bg.end());
    }
    auto take = [&](std::vector<Piece>& src, std::vector<Piece>& dst, size_t n) {
        std::shuffle(src.begin(), src.end(), g);
        for (size_t i = 0; i < src.size() && i < n; ++i) dst.push_back(src[i]);
    };
    take(top, p.top, 6);
    take(hang, p.hang, 5);
    take(props, p.props, 5);
    take(bg, p.bg, 7);

    // Chaos pieces were authored against other palettes; that's fine because
    // slots are palette-relative.
    auto const& src = all[static_cast<size_t>(g() % (all.size() - 1))];
    p.outlineId = src.outlineId;
    p.glowId = src.glowId;
    p.topChance = rf(0.25f, 0.5f);
    p.hangChance = rf(0.15f, 0.4f);
    p.sideChance = rf(0.05f, 0.2f);
    p.propChance = rf(0.05f, 0.12f);
    p.bgChance = rf(0.45f, 0.65f);
    return p;
}

}  // namespace autodeco
