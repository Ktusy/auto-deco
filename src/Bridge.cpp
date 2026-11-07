#include "Bridge.hpp"

#include <Geode/Geode.hpp>

#include <cmath>
#include <unordered_set>
#include <vector>

using namespace geode::prelude;

namespace autodeco {
namespace {

// Every object we create is tagged with the id of the run that created it.
// The tag lives on the in-memory node only, so Undo / Remove work for the
// current editor session.
constexpr char const* kTag = "autodeco-run-id";

int g_runCounter = 0;
std::vector<int> g_runs;  // run ids that can still be undone, oldest first

// Solid objects that are NOT a full 30x30 block (slabs and small blocks).
// They occupy their cell but do not get edge decoration.
bool isPartialId(int id) {
    static std::unordered_set<int> const ids = {
        40, 62, 63, 64, 65, 66, 68, 147, 170, 171, 172, 173, 174, 176, 192, 195, 196, 197,
        204, 206, 215, 219, 220, 369, 370, 653, 654, 655, 656, 657, 661, 662, 663, 664,
        723, 724, 735, 736, 891, 916, 917, 1079, 1080, 1081, 1096, 1097, 1098, 1903, 1904,
        1905, 1910, 1911, 1927};
    return ids.contains(id);
}

bool nearly(float a, float b, float eps = 0.05f) {
    return std::fabs(a - b) < eps;
}

bool isRightAngle(float rotation) {
    float r = std::fmod(std::fabs(rotation), 90.f);
    return r < 0.5f || r > 89.5f;
}

std::vector<LayoutObject> collectLayout(LevelEditorLayer* lel, bool selectedOnly) {
    std::vector<LayoutObject> out;
    auto all = lel->getAllObjects();
    if (!all) return out;

    std::unordered_set<GameObject*> selected;
    if (selectedOnly) {
        if (auto ui = lel->m_editorUI) {
            if (ui->m_selectedObject) selected.insert(ui->m_selectedObject);
            if (ui->m_selectedObjects) {
                for (unsigned i = 0; i < ui->m_selectedObjects->count(); ++i) {
                    if (auto o = static_cast<GameObject*>(ui->m_selectedObjects->objectAtIndex(i)))
                        selected.insert(o);
                }
            }
        }
    }

    out.reserve(all->count());
    for (unsigned i = 0; i < all->count(); ++i) {
        auto obj = static_cast<GameObject*>(all->objectAtIndex(i));
        if (!obj) continue;
        if (obj->getUserObject(kTag)) continue;        // our own output from this session
        int id = obj->m_objectID;
        if (isDecoId(id)) continue;                     // our own output from an earlier session

        LayoutObject lo;
        auto pos = obj->getPosition();
        lo.x = pos.x;
        lo.y = pos.y;
        lo.selected = !selectedOnly || selected.contains(obj);

        auto type = obj->m_objectType;
        if (type == GameObjectType::Decoration) {
            lo.kind = Kind::Decoration;
        } else if (type == GameObjectType::Solid) {
            bool plain = !isPartialId(id) && nearly(obj->getScaleX(), 1.f) &&
                         nearly(obj->getScaleY(), 1.f) && isRightAngle(obj->getRotation());
            lo.kind = plain ? Kind::Solid : Kind::Partial;
        } else if (type == GameObjectType::Slope) {
            lo.kind = Kind::Partial;
        } else {
            lo.kind = Kind::Blocked;
        }
        out.push_back(lo);
    }
    return out;
}

// Removes every object tagged with `runId` (0 = any run). Returns how many.
int removeTagged(LevelEditorLayer* lel, int runId) {
    auto all = lel->getAllObjects();
    if (!all) return 0;

    std::vector<GameObject*> victims;
    for (unsigned i = 0; i < all->count(); ++i) {
        auto obj = static_cast<GameObject*>(all->objectAtIndex(i));
        if (!obj) continue;
        auto tag = static_cast<CCInteger*>(obj->getUserObject(kTag));
        if (!tag) continue;
        if (runId == 0 || tag->getValue() == runId) victims.push_back(obj);
    }
    if (victims.empty()) return 0;

    if (auto ui = lel->m_editorUI) ui->deselectAll();
    for (auto obj : victims) lel->removeObject(obj, true);
    lel->dirtifyTriggers();
    return static_cast<int>(victims.size());
}

}  // namespace

bool editorIsOpen() {
    auto lel = LevelEditorLayer::get();
    return lel && lel->m_editorUI;
}

bool hasUndoableRun() {
    return !g_runs.empty();
}

RunResult decorateEditor(Options const& opt) {
    RunResult res;
    auto lel = LevelEditorLayer::get();
    if (!lel || !lel->m_editorUI) {
        res.message = "Open the level editor first.";
        return res;
    }

    // Re-decorating replaces the previous result instead of piling on top of it.
    if (opt.replaceLast && !g_runs.empty()) {
        res.removed = removeTagged(lel, g_runs.back());
        g_runs.pop_back();
    }

    auto layout = collectLayout(lel, opt.selectedOnly);
    auto gen = generate(layout, opt);
    if (!gen.error.empty()) {
        res.message = gen.error;
        return res;
    }

    int runId = ++g_runCounter;
    int created = 0;
    std::vector<GameObject*> hazards;

    for (auto const& chunk : gen.chunks) {
        auto allBefore = lel->getAllObjects();
        unsigned before = allBefore ? allBefore->count() : 0;

        // noUndo = true (we have our own undo), noLimit = true (ignore the object limit)
        auto made = lel->createObjectsFromString(chunk, true, true);

        std::vector<GameObject*> fresh;
        if (made && made->count() > 0) {
            for (unsigned i = 0; i < made->count(); ++i)
                if (auto o = static_cast<GameObject*>(made->objectAtIndex(i))) fresh.push_back(o);
        } else if (auto allAfter = lel->getAllObjects()) {
            // Fallback: new objects are appended to the object array.
            for (unsigned i = before; i < allAfter->count(); ++i)
                if (auto o = static_cast<GameObject*>(allAfter->objectAtIndex(i))) fresh.push_back(o);
        }

        for (auto obj : fresh) {
            obj->setUserObject(kTag, CCInteger::create(runId));
            // Safety net: decoration must never kill the player.
            if (obj->m_objectType == GameObjectType::Hazard) hazards.push_back(obj);
            ++created;
        }
    }

    for (auto obj : hazards) {
        lel->removeObject(obj, true);
        --created;
    }
    lel->dirtifyTriggers();

    if (created <= 0) {
        res.message = "Nothing could be created. Is this GD version supported?";
        return res;
    }

    g_runs.push_back(runId);
    res.ok = true;
    res.created = created;

    res.message = fmt::format("Added {} objects around {} blocks", created, gen.sourceCount);
    if (res.removed > 0) res.message += fmt::format(" (replaced {})", res.removed);
    if (gen.capped) res.message += " - object cap reached";
    return res;
}

RunResult removeDeco(bool lastRunOnly) {
    RunResult res;
    auto lel = LevelEditorLayer::get();
    if (!lel || !lel->m_editorUI) {
        res.message = "Open the level editor first.";
        return res;
    }

    if (lastRunOnly) {
        if (g_runs.empty()) {
            res.message = "Nothing to undo.";
            return res;
        }
        res.removed = removeTagged(lel, g_runs.back());
        g_runs.pop_back();
    } else {
        res.removed = removeTagged(lel, 0);
        g_runs.clear();
    }

    res.ok = res.removed > 0;
    res.message = res.ok ? fmt::format("Removed {} objects", res.removed)
                         : std::string("Nothing to remove (deco from earlier sessions can't be tracked).");
    return res;
}

}  // namespace autodeco
