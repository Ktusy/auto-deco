#include "DecoPopup.hpp"

#include "Bridge.hpp"
#include "Presets.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <string>

using namespace geode::prelude;

namespace {

bool g_open = false;

struct ToggleDef {
    char const* label;
    char const* key;  // saved-value key
    bool autodeco::Options::* member;
    bool defaultValue;
};

// 3 columns x 4 rows
ToggleDef const kToggles[] = {
    {"Outlines",      "opt-outlines",     &autodeco::Options::outlines,     true},
    {"Glow",          "opt-glow",         &autodeco::Options::glow,         true},
    {"Top Decor",     "opt-top",          &autodeco::Options::topDecor,     true},
    {"Ground Decor",  "opt-ground",       &autodeco::Options::groundDecor,  true},

    {"Hanging Decor", "opt-hang",         &autodeco::Options::hangDecor,    true},
    {"Side Decor",    "opt-side",         &autodeco::Options::sideDecor,    false},
    {"Props",         "opt-props",        &autodeco::Options::props,        true},
    {"Background",    "opt-background",   &autodeco::Options::background,   true},

    {"Theme Colors",  "opt-colors",       &autodeco::Options::colors,       true},
    {"Selected Only", "opt-selected",     &autodeco::Options::selectedOnly, false},
    {"Replace Last",  "opt-replace",      &autodeco::Options::replaceLast,  true},
};
constexpr size_t kToggleCount = sizeof(kToggles) / sizeof(kToggles[0]);

uint32_t randomSeed() {
    std::random_device rd;
    return rd() % 1000000000u;
}

CCLabelBMFont* makeLabel(char const* text, char const* font, float scale, bool leftAlign = false) {
    auto label = CCLabelBMFont::create(text, font);
    label->setScale(scale);
    if (leftAlign) label->setAnchorPoint({0.f, 0.5f});
    return label;
}

}  // namespace

DecoPopup* DecoPopup::create() {
    auto ret = new DecoPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool DecoPopup::isOpen() {
    return g_open;
}

DecoPopup::~DecoPopup() {
    g_open = false;
}

bool DecoPopup::init() {
    if (!Popup::init(440.f, 280.f)) return false;
    g_open = true;

    this->loadState();
    this->buildUI();
    this->refreshPreset();
    this->refreshValues();
    return true;
}

// ---------------------------------------------------------------------------
//  State
// ---------------------------------------------------------------------------
void DecoPopup::loadState() {
    auto mod = Mod::get();
    int count = static_cast<int>(autodeco::presets().size());
    m_opt.presetIndex = std::clamp(static_cast<int>(mod->getSavedValue<int64_t>("preset", 0)), 0, count - 1);
    m_opt.density = std::clamp(static_cast<float>(mod->getSavedValue<double>("density", 0.6)), 0.05f, 1.f);
    for (auto const& t : kToggles) m_opt.*(t.member) = mod->getSavedValue<bool>(t.key, t.defaultValue);
    m_opt.seed = randomSeed();
}

void DecoPopup::saveState() {
    auto mod = Mod::get();
    mod->setSavedValue<int64_t>("preset", m_opt.presetIndex);
    mod->setSavedValue<double>("density", m_opt.density);
    for (auto const& t : kToggles) mod->setSavedValue<bool>(t.key, m_opt.*(t.member));
}

// ---------------------------------------------------------------------------
//  UI
// ---------------------------------------------------------------------------
void DecoPopup::buildUI() {
    // Title
    auto title = makeLabel("Auto Deco", "goldFont.fnt", 0.8f);
    m_mainLayer->addChildAtPosition(title, Anchor::Top, {0.f, -20.f});

    // Preset row:  <  Name  >
    auto makeArrow = [this](bool right, SEL_MenuHandler cb) {
        auto spr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        spr->setFlipX(right);
        spr->setScale(0.8f);
        return CCMenuItemSpriteExtra::create(spr, this, cb);
    };
    m_buttonMenu->addChildAtPosition(makeArrow(false, menu_selector(DecoPopup::onPrevPreset)),
                                     Anchor::Center, {-150.f, 98.f});
    m_buttonMenu->addChildAtPosition(makeArrow(true, menu_selector(DecoPopup::onNextPreset)),
                                     Anchor::Center, {150.f, 98.f});

    m_presetLabel = makeLabel("", "goldFont.fnt", 0.85f);
    m_mainLayer->addChildAtPosition(m_presetLabel, Anchor::Center, {0.f, 98.f});

    m_desc1 = makeLabel("", "chatFont.fnt", 0.6f);
    m_desc2 = makeLabel("", "chatFont.fnt", 0.6f);
    m_mainLayer->addChildAtPosition(m_desc1, Anchor::Center, {0.f, 74.f});
    m_mainLayer->addChildAtPosition(m_desc2, Anchor::Center, {0.f, 62.f});

    // Palette preview swatches
    for (size_t i = 0; i < m_swatches.size(); ++i) {
        float x = -50.f + static_cast<float>(i) * 20.f;

        auto frame = CCLayerColor::create(ccc4(255, 255, 255, 90), 22.f, 22.f);
        frame->ignoreAnchorPointForPosition(false);
        frame->setAnchorPoint({0.5f, 0.5f});
        m_mainLayer->addChildAtPosition(frame, Anchor::Center, {x, 40.f});

        auto sw = CCLayerColor::create(ccc4(0, 0, 0, 255), 18.f, 18.f);
        sw->ignoreAnchorPointForPosition(false);
        sw->setAnchorPoint({0.5f, 0.5f});
        m_mainLayer->addChildAtPosition(sw, Anchor::Center, {x, 40.f});
        m_swatches[i] = sw;
    }

    // Toggles: 3 columns x 4 rows
    for (size_t i = 0; i < kToggleCount; ++i) {
        float col = static_cast<float>(i / 4);
        float row = static_cast<float>(i % 4);
        float x = -200.f + col * 140.f;
        float y = 14.f - row * 22.f;

        auto toggler = CCMenuItemToggler::createWithStandardSprites(
            this, menu_selector(DecoPopup::onToggle), 0.55f);
        toggler->setTag(static_cast<int>(i));
        toggler->toggle(m_opt.*(kToggles[i].member));
        m_buttonMenu->addChildAtPosition(toggler, Anchor::Center, {x, y});

        auto label = makeLabel(kToggles[i].label, "bigFont.fnt", 0.34f, true);
        m_mainLayer->addChildAtPosition(label, Anchor::Center, {x + 14.f, y});
    }

    // Density stepper
    auto densityTitle = makeLabel("Density", "bigFont.fnt", 0.4f, true);
    m_mainLayer->addChildAtPosition(densityTitle, Anchor::Center, {-200.f, -76.f});

    auto makeStep = [this](bool right, int dir) {
        auto spr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        spr->setFlipX(right);
        spr->setScale(0.55f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(DecoPopup::onDensity));
        btn->setTag(dir);
        return btn;
    };
    m_buttonMenu->addChildAtPosition(makeStep(false, -1), Anchor::Center, {-105.f, -76.f});
    m_buttonMenu->addChildAtPosition(makeStep(true, +1), Anchor::Center, {-35.f, -76.f});
    m_densityLabel = makeLabel("", "goldFont.fnt", 0.55f);
    m_mainLayer->addChildAtPosition(m_densityLabel, Anchor::Center, {-70.f, -76.f});

    // Seed + roll
    auto seedTitle = makeLabel("Seed", "bigFont.fnt", 0.4f, true);
    m_mainLayer->addChildAtPosition(seedTitle, Anchor::Center, {20.f, -76.f});
    m_seedLabel = makeLabel("", "goldFont.fnt", 0.5f);
    m_mainLayer->addChildAtPosition(m_seedLabel, Anchor::Center, {102.f, -76.f});

    auto rollSpr = ButtonSprite::create("Roll", "goldFont.fnt", "GJ_button_02.png", 0.6f);
    auto rollBtn = CCMenuItemSpriteExtra::create(rollSpr, this, menu_selector(DecoPopup::onRoll));
    m_buttonMenu->addChildAtPosition(rollBtn, Anchor::Center, {180.f, -76.f});

    // Action buttons
    auto decorateBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Decorate", "goldFont.fnt", "GJ_button_01.png", 0.85f),
        this, menu_selector(DecoPopup::onDecorate));
    m_buttonMenu->addChildAtPosition(decorateBtn, Anchor::Center, {-110.f, -112.f});

    auto undoBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Undo Last", "goldFont.fnt", "GJ_button_02.png", 0.7f),
        this, menu_selector(DecoPopup::onUndo));
    m_buttonMenu->addChildAtPosition(undoBtn, Anchor::Center, {30.f, -112.f});

    auto removeBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Remove All", "goldFont.fnt", "GJ_button_06.png", 0.7f),
        this, menu_selector(DecoPopup::onRemoveAll));
    m_buttonMenu->addChildAtPosition(removeBtn, Anchor::Center, {145.f, -112.f});
}

void DecoPopup::refreshPreset() {
    auto const& all = autodeco::presets();
    auto const& preset = all[static_cast<size_t>(m_opt.presetIndex)];

    m_presetLabel->setString(preset.name.c_str());
    m_presetLabel->setScale(1.f);
    float w = m_presetLabel->getContentSize().width;
    m_presetLabel->setScale(w > 0.f ? std::min(0.85f, 210.f / w) : 0.85f);

    std::string d1 = preset.description, d2;
    if (auto nl = d1.find('\n'); nl != std::string::npos) {
        d2 = d1.substr(nl + 1);
        d1 = d1.substr(0, nl);
    }
    m_desc1->setString(d1.c_str());
    m_desc2->setString(d2.c_str());

    autodeco::Palette pal = preset.randomPalette ? autodeco::makeChaos(m_opt.seed).palette : preset.palette;
    autodeco::Rgb colors[6] = {pal.bg, pal.layout, pal.primary, pal.secondary, pal.accent, pal.light};
    for (size_t i = 0; i < m_swatches.size(); ++i) {
        m_swatches[i]->setColor(ccc3(colors[i].r, colors[i].g, colors[i].b));
    }
}

void DecoPopup::refreshValues() {
    m_densityLabel->setString(fmt::format("{}%", static_cast<int>(std::lround(m_opt.density * 100.f))).c_str());
    m_seedLabel->setString(std::to_string(m_opt.seed).c_str());
}

// ---------------------------------------------------------------------------
//  Callbacks
// ---------------------------------------------------------------------------
void DecoPopup::onPrevPreset(CCObject*) {
    int n = static_cast<int>(autodeco::presets().size());
    m_opt.presetIndex = (m_opt.presetIndex + n - 1) % n;
    this->refreshPreset();
}

void DecoPopup::onNextPreset(CCObject*) {
    int n = static_cast<int>(autodeco::presets().size());
    m_opt.presetIndex = (m_opt.presetIndex + 1) % n;
    this->refreshPreset();
}

void DecoPopup::onToggle(CCObject* sender) {
    auto toggler = static_cast<CCMenuItemToggler*>(sender);
    size_t i = static_cast<size_t>(toggler->getTag());
    if (i >= kToggleCount) return;
    // The callback runs before the toggler flips, so the new value is the opposite.
    m_opt.*(kToggles[i].member) = !toggler->isToggled();
}

void DecoPopup::onDensity(CCObject* sender) {
    int dir = static_cast<CCNode*>(sender)->getTag();
    float v = std::round((m_opt.density + 0.05f * static_cast<float>(dir)) * 100.f) / 100.f;
    m_opt.density = std::clamp(v, 0.05f, 1.f);
    this->refreshValues();
}

void DecoPopup::onRoll(CCObject*) {
    m_opt.seed = randomSeed();
    this->refreshValues();
    this->refreshPreset();  // the Chaos preview palette depends on the seed
}

void DecoPopup::onDecorate(CCObject*) {
    this->saveState();

    auto opt = m_opt;
    auto mod = Mod::get();
    opt.baseChannel = std::clamp(static_cast<int>(mod->getSettingValue<int64_t>("base-channel")), 1, 990);
    opt.recolorLayout = mod->getSettingValue<bool>("recolor-layout");
    opt.editorLayer = std::max(0, static_cast<int>(mod->getSettingValue<int64_t>("deco-editor-layer")));
    opt.maxObjects = std::max(500, static_cast<int>(mod->getSettingValue<int64_t>("max-objects")));

    auto res = autodeco::decorateEditor(opt);
    if (!res.ok) {
        FLAlertLayer::create("Auto Deco", res.message, "OK")->show();
        return;
    }
    Notification::create(res.message, NotificationIcon::Success)->show();
}

void DecoPopup::onUndo(CCObject*) {
    auto res = autodeco::removeDeco(true);
    Notification::create(res.message, res.ok ? NotificationIcon::Success : NotificationIcon::Warning)->show();
}

void DecoPopup::onRemoveAll(CCObject*) {
    auto res = autodeco::removeDeco(false);
    Notification::create(res.message, res.ok ? NotificationIcon::Success : NotificationIcon::Warning)->show();
}
