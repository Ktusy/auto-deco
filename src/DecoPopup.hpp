#pragma once

#include <Geode/Geode.hpp>

#include "Generator.hpp"

#include <array>

class DecoPopup : public geode::Popup {
public:
    static DecoPopup* create();
    static bool isOpen();

    ~DecoPopup() override;

protected:
    bool init();

    void loadState();
    void saveState();
    void buildUI();
    void refreshPreset();
    void refreshValues();

    void onPrevPreset(cocos2d::CCObject*);
    void onNextPreset(cocos2d::CCObject*);
    void onToggle(cocos2d::CCObject*);
    void onDensity(cocos2d::CCObject*);
    void onRoll(cocos2d::CCObject*);
    void onDecorate(cocos2d::CCObject*);
    void onUndo(cocos2d::CCObject*);
    void onRemoveAll(cocos2d::CCObject*);

    autodeco::Options m_opt;

    cocos2d::CCLabelBMFont* m_presetLabel = nullptr;
    cocos2d::CCLabelBMFont* m_desc1 = nullptr;
    cocos2d::CCLabelBMFont* m_desc2 = nullptr;
    cocos2d::CCLabelBMFont* m_densityLabel = nullptr;
    cocos2d::CCLabelBMFont* m_seedLabel = nullptr;
    std::array<cocos2d::CCLayerColor*, 6> m_swatches{};
};
