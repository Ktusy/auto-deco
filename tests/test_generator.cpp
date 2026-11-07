// Native sanity test for the generator. Does NOT need Geode or Geometry Dash.
//
//   g++ -std=c++20 -O1 -g -fsanitize=address,undefined
//       -I src tests/test_generator.cpp src/Generator.cpp src/Presets.cpp -o test_generator
//   ./test_generator            # prints stats for every preset
//   ./test_generator --dump     # also writes sample_<preset>.txt level strings

#include "Generator.hpp"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <regex>
#include <string>

using namespace autodeco;

static std::vector<LayoutObject> makeLayout() {
    std::vector<LayoutObject> v;
    auto block = [&](int cx, int cy) { v.push_back({cx * 30.f + 15.f, cy * 30.f + 15.f, Kind::Solid, true}); };
    auto hazard = [&](int cx, int cy) { v.push_back({cx * 30.f + 15.f, cy * 30.f + 15.f, Kind::Blocked, true}); };
    auto slab = [&](int cx, int cy) { v.push_back({cx * 30.f + 15.f, cy * 30.f + 7.f, Kind::Partial, true}); };

    for (int x = 0; x < 200; ++x) block(x, 0);                 // floor row
    for (int x = 20; x < 30; ++x) { block(x, 3); block(x, 4); } // floating platform
    for (int x = 60; x < 75; ++x) block(x, 6);                  // higher platform
    for (int y = 1; y < 8; ++y) block(100, y);                  // wall
    for (int x = 120; x < 140; ++x) { block(x, 7); block(x, 8); } // ceiling slab
    hazard(40, 1); hazard(41, 1); slab(50, 1);
    return v;
}

static bool wellFormed(std::string const& chunk) {
    // every object: k,v pairs of integers/decimals, terminated by ';'
    static std::regex const obj(R"(^(-?\d+(\.\d+)?,-?\d+(\.\d+)?)(,-?\d+(\.\d+)?,-?\d+(\.\d+)?)*;)");
    size_t pos = 0;
    while (pos < chunk.size()) {
        size_t end = chunk.find(';', pos);
        if (end == std::string::npos) return false;
        std::string one = chunk.substr(pos, end - pos + 1);
        if (!std::regex_match(one, obj)) {
            std::printf("  BAD OBJECT: %s\n", one.c_str());
            return false;
        }
        if (one.rfind("1,", 0) != 0) return false;  // key 1 (id) must come first
        pos = end + 1;
    }
    return true;
}

int main(int argc, char** argv) {
    bool dump = argc > 1 && std::strcmp(argv[1], "--dump") == 0;
    auto layout = makeLayout();
    int failures = 0;

    auto const& ps = presets();
    std::printf("%zu presets, layout of %zu objects\n\n", ps.size(), layout.size());

    for (size_t i = 0; i < ps.size(); ++i) {
        Options o;
        o.presetIndex = static_cast<int>(i);
        o.seed = 12345;
        o.sideDecor = true;
        auto a = generate(layout, o);
        auto b = generate(layout, o);   // determinism check

        bool ok = a.error.empty() && a.chunks == b.chunks;
        for (auto const& c : a.chunks) ok = ok && wellFormed(c);
        if (a.objectCount <= 0) ok = false;
        // triggers: 12 with recolor on
        if (a.triggerCount != 12) { std::printf("  unexpected trigger count %d\n", a.triggerCount); ok = false; }
        if (a.chunks.empty() || a.chunks.back().find("nan") != std::string::npos ||
            a.chunks.back().find("inf") != std::string::npos) ok = false;

        std::printf("%-16s objects=%-5d triggers=%-3d chunks=%zu capped=%d  %s\n", ps[i].name.c_str(),
                    a.objectCount, a.triggerCount, a.chunks.size(), a.capped ? 1 : 0, ok ? "OK" : "FAIL");
        if (!ok) ++failures;

        if (dump) {
            std::ofstream f("sample_" + std::to_string(i) + ".txt");
            for (auto const& c : a.chunks) f << c;
        }
    }

    // different seeds must differ
    Options o1, o2;
    o1.seed = 1; o2.seed = 2;
    if (generate(layout, o1).chunks == generate(layout, o2).chunks) { std::printf("seed has no effect!\n"); ++failures; }

    // empty layout -> error, not a crash
    if (generate({}, Options{}).error.empty()) { std::printf("empty layout should error\n"); ++failures; }

    // selected-only with nothing selected -> error
    {
        auto copy = layout;
        for (auto& l : copy) l.selected = false;
        if (generate(copy, Options{}).error.empty()) { std::printf("nothing selected should error\n"); ++failures; }
    }

    // object cap is honoured
    {
        Options o;
        o.maxObjects = 150;
        o.presetIndex = 0;
        auto r = generate(layout, o);
        if (!r.capped || r.objectCount > 150 + 8) { std::printf("cap not honoured: %d\n", r.objectCount); ++failures; }
    }

    // editor layer + colour option variants
    {
        Options o;
        o.editorLayer = 7;
        o.colors = false;
        auto r = generate(layout, o);
        bool hasLayer = !r.chunks.empty() && r.chunks[0].find(",20,7;") != std::string::npos;
        if (!hasLayer || r.triggerCount != 0) { std::printf("editor layer/colors option failed\n"); ++failures; }
    }

    std::printf("\n%s\n", failures ? "SOME TESTS FAILED" : "ALL TESTS PASSED");
    return failures ? 1 : 0;
}
