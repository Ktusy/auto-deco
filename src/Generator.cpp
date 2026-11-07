#include "Generator.hpp"

#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace autodeco {
namespace {

constexpr float CELL = 30.f;
constexpr int kChunkSize = 1200;

struct Cell {
    int x, y;
};

inline int64_t cellKey(int x, int y) {
    return static_cast<int64_t>(x) * 4294967296LL + static_cast<int64_t>(y);
}

// Small deterministic RNG (same sequence on every platform for a given seed).
struct Rng {
    uint64_t s;
    explicit Rng(uint32_t seed) : s(0x9E3779B97F4A7C15ull ^ (static_cast<uint64_t>(seed) * 0xBF58476D1CE4E5B9ull)) {
        next();
        next();
    }
    uint32_t next() {  // xorshift64*
        s ^= s >> 12;
        s ^= s << 25;
        s ^= s >> 27;
        return static_cast<uint32_t>((s * 0x2545F4914F6CDD1Dull) >> 32);
    }
    float f() { return static_cast<float>(next() >> 8) * (1.0f / 16777216.0f); }
    float range(float a, float b) { return a + (b - a) * f(); }
    bool chance(float p) { return f() < p; }
};

Piece const* pickPiece(std::vector<Piece> const& v, Rng& rng) {
    if (v.empty()) return nullptr;
    float total = 0.f;
    for (auto const& p : v) total += std::max(0.f, p.weight);
    if (total <= 0.f) return &v.front();
    float t = rng.f() * total;
    for (auto const& p : v) {
        t -= std::max(0.f, p.weight);
        if (t <= 0.f) return &p;
    }
    return &v.back();
}

// --- locale-independent number formatting -----------------------------------
void putInt(std::string& s, long long v) {
    s += std::to_string(v);
}

void putFixed(std::string& s, float v, int decimals) {
    long long scale = 1;
    for (int i = 0; i < decimals; ++i) scale *= 10;
    long long iv = std::llround(static_cast<double>(v) * static_cast<double>(scale));
    if (iv < 0) {
        s += '-';
        iv = -iv;
    }
    s += std::to_string(iv / scale);
    if (decimals > 0) {
        s += '.';
        std::string frac = std::to_string(iv % scale);
        s.append(static_cast<size_t>(decimals) - frac.size(), '0');
        s += frac;
    }
}

// --- builds the object strings ----------------------------------------------
class Emitter {
public:
    Emitter(int baseChannel, int editorLayer, int cap)
        : m_base(baseChannel), m_layer(editorLayer), m_cap(cap) {}

    bool full() const { return m_objects >= m_cap; }
    int objects() const { return m_objects; }
    int triggers() const { return m_triggers; }

    int channel(Slot s) const {
        switch (s) {
            case Slot::Primary:   return m_base + 0;
            case Slot::Secondary: return m_base + 1;
            case Slot::Accent:    return m_base + 2;
            case Slot::Dark:      return m_base + 3;
            case Slot::Light:     return m_base + 4;
            case Slot::Glow:      return m_base + 5;
            case Slot::Layout:    return 1004;  // built-in "Obj" channel
            case Slot::None:      break;
        }
        return 0;
    }

    void object(int id, float x, float y, float rot, float scale, bool flipX, bool flipY,
                Slot main, Slot detail, int z, int zo) {
        auto& s = m_cur;
        s += "1,";
        putInt(s, id);
        s += ",2,";
        putFixed(s, x, 2);
        s += ",3,";
        putFixed(s, y, 2);
        if (flipX) s += ",4,1";
        if (flipY) s += ",5,1";
        if (std::fabs(rot) > 0.01f) {
            s += ",6,";
            putFixed(s, rot, 1);
        }
        int mc = channel(main);
        int dc = (detail == Slot::None) ? mc : channel(detail);
        if (mc) {
            s += ",21,";
            putInt(s, mc);
        }
        if (dc) {
            s += ",22,";
            putInt(s, dc);
        }
        s += ",24,";
        putInt(s, z);
        s += ",25,";
        putInt(s, zo);
        if (std::fabs(scale - 1.f) > 0.001f) {
            s += ",32,";
            putFixed(s, scale, 3);
        }
        if (m_layer > 0) {
            s += ",20,";
            putInt(s, m_layer);
        }
        s += ';';
        ++m_objects;
        bump();
    }

    // Colour trigger (object 899): sets `ch` to `c` instantly at level start.
    void colorTrigger(int ch, Rgb c, bool blending, float x, float y) {
        auto& s = m_cur;
        s += "1,899,2,";
        putFixed(s, x, 2);
        s += ",3,";
        putFixed(s, y, 2);
        s += ",36,1,7,";
        putInt(s, c.r);
        s += ",8,";
        putInt(s, c.g);
        s += ",9,";
        putInt(s, c.b);
        s += ",10,0,35,1,17,";
        s += blending ? '1' : '0';
        s += ",23,";
        putInt(s, ch);
        s += ';';
        ++m_triggers;
        bump();
    }

    std::vector<std::string> finish() {
        if (m_curCount > 0) m_chunks.push_back(std::move(m_cur));
        m_cur.clear();
        m_curCount = 0;
        return std::move(m_chunks);
    }

private:
    void bump() {
        if (++m_curCount >= kChunkSize) {
            m_chunks.push_back(std::move(m_cur));
            m_cur.clear();
            m_curCount = 0;
        }
    }

    int m_base, m_layer, m_cap;
    int m_objects = 0, m_triggers = 0, m_curCount = 0;
    std::string m_cur;
    std::vector<std::string> m_chunks;
};

enum class Face { Top, Bottom, Left, Right };

}  // namespace

Output generate(std::vector<LayoutObject> const& layout, Options const& opt) {
    Output out;

    auto const& list = presets();
    if (list.empty()) {
        out.error = "No presets are available.";
        return out;
    }
    int idx = std::clamp(opt.presetIndex, 0, static_cast<int>(list.size()) - 1);
    Preset preset = list[static_cast<size_t>(idx)];
    if (preset.randomPalette) preset = makeChaos(opt.seed);

    float d = std::clamp(opt.density, 0.f, 1.f);
    auto scaled = [&](float base) { return std::clamp(base * (0.25f + 1.5f * d), 0.f, 1.f); };

    // ---- 1. Occupancy grid ---------------------------------------------------
    std::unordered_set<int64_t> solidish;  // solid blocks, slabs, slopes
    std::unordered_set<int64_t> blocked;   // anything that is not pure decoration
    std::unordered_set<int64_t> srcSet;
    std::vector<Cell> src;                 // blocks that receive decoration

    for (auto const& o : layout) {
        if (o.kind == Kind::Decoration) continue;
        int cx = static_cast<int>(std::floor(o.x / CELL));
        int cy = static_cast<int>(std::floor(o.y / CELL));
        int64_t k = cellKey(cx, cy);
        blocked.insert(k);
        if (o.kind == Kind::Solid || o.kind == Kind::Partial) solidish.insert(k);
        if (o.kind == Kind::Solid && o.selected && srcSet.insert(k).second) src.push_back({cx, cy});
    }

    if (src.empty()) {
        out.error = "No solid blocks found to decorate.\n(With 'Selected Only' on, select some blocks first.)";
        return out;
    }

    // Sorted so that the same seed always gives the same result.
    std::sort(src.begin(), src.end(), [](Cell a, Cell b) { return a.x != b.x ? a.x < b.x : a.y < b.y; });
    out.sourceCount = static_cast<int>(src.size());

    int minCx = src[0].x, maxCx = src[0].x, minCy = src[0].y, maxCy = src[0].y;
    for (auto c : src) {
        minCx = std::min(minCx, c.x);
        maxCx = std::max(maxCx, c.x);
        minCy = std::min(minCy, c.y);
        maxCy = std::max(maxCy, c.y);
    }

    Rng rng(opt.seed);
    int cap = std::max(100, opt.maxObjects);
    Emitter em(opt.baseChannel, opt.editorLayer, cap);

    // Places `p` against the face `f` of block cell (cx, cy).
    auto place = [&](Piece const& p, int cx, int cy, Face f) {
        float s = rng.range(p.scaleMin, p.scaleMax);
        float half = p.length * s * 0.5f + p.offset;
        float jitter = rng.range(-6.f, 6.f);
        float x = 0, y = 0, rot = 0;
        switch (f) {
            case Face::Top:
                x = cx * CELL + CELL / 2 + jitter;
                y = (cy + 1) * CELL + half;
                rot = 0.f;
                break;
            case Face::Bottom:
                x = cx * CELL + CELL / 2 + jitter;
                y = cy * CELL - half;
                rot = 180.f;
                break;
            case Face::Right:
                x = (cx + 1) * CELL + half;
                y = cy * CELL + CELL / 2 + jitter;
                rot = 90.f;
                break;
            case Face::Left:
                x = cx * CELL - half;
                y = cy * CELL + CELL / 2 + jitter;
                rot = 270.f;
                break;
        }
        if (!p.orient) rot = 0.f;
        if (p.randomRot) rot = rng.range(0.f, 360.f);
        bool fx = p.flipX && rng.chance(0.5f);
        em.object(p.id, x, y, rot, s, fx, false, p.main, p.detail, p.zLayer, p.zOrder);
    };

    auto isFree = [&](int x, int y) { return !blocked.contains(cellKey(x, y)); };
    auto isOpen = [&](int x, int y) { return !solidish.contains(cellKey(x, y)); };

    // ---- 2. Outlines + glow on every exposed face ----------------------------
    if ((opt.outlines || opt.glow)) {
        auto face = [&](int cx, int cy, float rot) {
            float x = cx * CELL + CELL / 2, y = cy * CELL + CELL / 2;
            if (opt.glow) em.object(preset.glowId, x, y, rot, 1.f, false, false, preset.glowSlot, Slot::None, 3, 1);
            if (opt.outlines) em.object(preset.outlineId, x, y, rot, 1.f, false, false, preset.outlineSlot, Slot::None, 3, 3);
        };
        for (auto c : src) {
            if (em.full()) break;
            if (isOpen(c.x, c.y + 1)) face(c.x, c.y, 0.f);
            if (isOpen(c.x + 1, c.y)) face(c.x, c.y, 90.f);
            if (c.y > 0 && isOpen(c.x, c.y - 1)) face(c.x, c.y, 180.f);  // nothing visible below the ground
            if (isOpen(c.x - 1, c.y)) face(c.x, c.y, 270.f);
        }
    }

    // ---- 3. Decoration growing out of top surfaces ---------------------------
    if (opt.topDecor && !preset.top.empty()) {
        float p = scaled(preset.topChance);
        for (auto c : src) {
            if (em.full()) break;
            if (!isFree(c.x, c.y + 1)) continue;
            if (!rng.chance(p)) continue;
            if (auto piece = pickPiece(preset.top, rng)) place(*piece, c.x, c.y, Face::Top);
        }
    }

    // ---- 4. Decoration along the ground line ---------------------------------
    if (opt.groundDecor && !preset.top.empty()) {
        float p = scaled(preset.topChance * 0.7f);
        for (int cx = minCx; cx <= maxCx; ++cx) {
            if (em.full()) break;
            if (!isFree(cx, 0)) continue;
            if (!rng.chance(p)) continue;
            if (auto piece = pickPiece(preset.top, rng)) place(*piece, cx, -1, Face::Top);
        }
    }

    // ---- 5. Hanging decoration under ceilings / platforms ---------------------
    if (opt.hangDecor && !preset.hang.empty()) {
        float p = scaled(preset.hangChance);
        for (auto c : src) {
            if (em.full()) break;
            if (c.y - 1 < 0 || !isFree(c.x, c.y - 1)) continue;
            if (!rng.chance(p)) continue;
            if (auto piece = pickPiece(preset.hang, rng)) place(*piece, c.x, c.y, Face::Bottom);
        }
    }

    // ---- 6. Wall decoration ----------------------------------------------------
    if (opt.sideDecor && !preset.top.empty()) {
        float p = scaled(preset.sideChance);
        std::vector<Piece const*> eligible;
        for (auto const& piece : preset.top)
            if (piece.sideOk) eligible.push_back(&piece);
        if (!eligible.empty()) {
            for (auto c : src) {
                if (em.full()) break;
                if (isFree(c.x + 1, c.y) && rng.chance(p))
                    place(*eligible[rng.next() % eligible.size()], c.x, c.y, Face::Right);
                if (isFree(c.x - 1, c.y) && rng.chance(p))
                    place(*eligible[rng.next() % eligible.size()], c.x, c.y, Face::Left);
            }
        }
    }

    // ---- 7. Props (gears, skulls, orbs) resting on surfaces ------------------
    if (opt.props && !preset.props.empty()) {
        float p = scaled(preset.propChance);
        for (auto c : src) {
            if (em.full()) break;
            if (!isFree(c.x, c.y + 1) || !rng.chance(p)) continue;
            if (auto piece = pickPiece(preset.props, rng)) place(*piece, c.x, c.y, Face::Top);
        }
    }

    // ---- 8. Background shapes scattered around the layout --------------------
    if (opt.background && !preset.bg.empty() && preset.bgChance > 0.f) {
        float spacing = 260.f - (260.f - 100.f) * d;
        float x0 = (minCx - 3) * CELL, x1 = (maxCx + 3) * CELL;
        float y0 = std::max(0.f, (minCy - 6) * CELL), y1 = (maxCy + 10) * CELL;
        for (float gx = x0; gx <= x1 && !em.full(); gx += spacing) {
            for (float gy = y0; gy <= y1 && !em.full(); gy += spacing) {
                if (!rng.chance(preset.bgChance)) continue;
                float x = gx + rng.range(-spacing * 0.45f, spacing * 0.45f);
                float y = gy + rng.range(-spacing * 0.45f, spacing * 0.45f);
                if (y < 0.f) continue;
                int cx = static_cast<int>(std::floor(x / CELL));
                int cy = static_cast<int>(std::floor(y / CELL));
                if (!isFree(cx, cy) || !isFree(cx, cy + 1) || !isFree(cx, cy - 1)) continue;
                Piece const* piece = pickPiece(preset.bg, rng);
                if (!piece) continue;
                float s = rng.range(piece->scaleMin, piece->scaleMax);
                float rot = piece->randomRot ? rng.range(0.f, 360.f) : 0.f;
                bool fx = piece->flipX && rng.chance(0.5f);
                em.object(piece->id, x, y, rot, s, fx, false, piece->main, piece->detail,
                          piece->zLayer, piece->zOrder);
            }
        }
    }

    out.capped = em.full();

    // ---- 9. Colour triggers that apply the palette ---------------------------
    if (opt.colors) {
        float tx = std::min(-15.f, minCx * CELL - 30.f);
        float ty = (maxCy + 6) * CELL;
        int row = 0;
        auto trig = [&](int ch, Rgb c, bool blend = false) {
            em.colorTrigger(ch, c, blend, tx, ty + row * CELL);
            ++row;
        };
        auto const& pal = preset.palette;
        trig(1000, pal.bg);
        trig(1001, pal.ground);
        trig(1009, pal.ground2);
        trig(1002, pal.line);
        trig(1003, pal.line);
        if (opt.recolorLayout) trig(1004, pal.layout);
        trig(em.channel(Slot::Primary), pal.primary);
        trig(em.channel(Slot::Secondary), pal.secondary);
        trig(em.channel(Slot::Accent), pal.accent);
        trig(em.channel(Slot::Dark), pal.dark);
        trig(em.channel(Slot::Light), pal.light);
        trig(em.channel(Slot::Glow), pal.glow, true);
    }

    out.objectCount = em.objects();
    out.triggerCount = em.triggers();
    out.chunks = em.finish();
    return out;
}

}  // namespace autodeco
