#include <Geode/Geode.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/TextInput.hpp>
#include <algorithm>
#include <optional>
#include <string>
#include <utility>
#include <vector>

using namespace geode::prelude;

// Если кнопка в правом столбце чуть больше или меньше соседних,
// поменяй это число (1.0f = точно как у соседей).
static constexpr float kButtonScaleTweak = 1.0f;

// ============================================================
//  Preset data
// ============================================================

struct FilterPreset {
    std::string name;
    int searchType = 0;
    std::string query;
    std::string difficulty;
    std::string length;
    bool star = false;
    bool uncompleted = false;
    bool featured = false;
    int songID = 0;
    bool original = false;
    bool twoPlayer = false;
    bool customSong = false;
    bool songFilter = false;
    bool noStar = false;
    bool coins = false;
    bool epic = false;
    bool legendary = false;
    bool mythic = false;
    bool completed = false;
    int demonFilter = 0;
    int folder = 0;
    int searchMode = 0;
};

template <>
struct matjson::Serialize<FilterPreset> {
    static geode::Result<FilterPreset> fromJson(matjson::Value const& value) {
        GEODE_UNWRAP_INTO(auto name, value["name"].asString());
        GEODE_UNWRAP_INTO(auto searchType, value["searchType"].asInt());
        GEODE_UNWRAP_INTO(auto query, value["query"].asString());
        GEODE_UNWRAP_INTO(auto difficulty, value["difficulty"].asString());
        GEODE_UNWRAP_INTO(auto length, value["length"].asString());
        GEODE_UNWRAP_INTO(auto star, value["star"].asBool());
        GEODE_UNWRAP_INTO(auto uncompleted, value["uncompleted"].asBool());
        GEODE_UNWRAP_INTO(auto featured, value["featured"].asBool());
        GEODE_UNWRAP_INTO(auto songID, value["songID"].asInt());
        GEODE_UNWRAP_INTO(auto original, value["original"].asBool());
        GEODE_UNWRAP_INTO(auto twoPlayer, value["twoPlayer"].asBool());
        GEODE_UNWRAP_INTO(auto customSong, value["customSong"].asBool());
        GEODE_UNWRAP_INTO(auto songFilter, value["songFilter"].asBool());
        GEODE_UNWRAP_INTO(auto noStar, value["noStar"].asBool());
        GEODE_UNWRAP_INTO(auto coins, value["coins"].asBool());
        GEODE_UNWRAP_INTO(auto epic, value["epic"].asBool());
        GEODE_UNWRAP_INTO(auto legendary, value["legendary"].asBool());
        GEODE_UNWRAP_INTO(auto mythic, value["mythic"].asBool());
        GEODE_UNWRAP_INTO(auto completed, value["completed"].asBool());
        GEODE_UNWRAP_INTO(auto demonFilter, value["demonFilter"].asInt());
        GEODE_UNWRAP_INTO(auto folder, value["folder"].asInt());
        GEODE_UNWRAP_INTO(auto searchMode, value["searchMode"].asInt());

        FilterPreset preset;

        preset.name = std::move(name);
        preset.searchType = static_cast<int>(searchType);
        preset.query = std::move(query);
        preset.difficulty = std::move(difficulty);
        preset.length = std::move(length);
        preset.star = star;
        preset.uncompleted = uncompleted;
        preset.featured = featured;
        preset.songID = static_cast<int>(songID);
        preset.original = original;
        preset.twoPlayer = twoPlayer;
        preset.customSong = customSong;
        preset.songFilter = songFilter;
        preset.noStar = noStar;
        preset.coins = coins;
        preset.epic = epic;
        preset.legendary = legendary;
        preset.mythic = mythic;
        preset.completed = completed;
        preset.demonFilter = static_cast<int>(demonFilter);
        preset.folder = static_cast<int>(folder);
        preset.searchMode = static_cast<int>(searchMode);

        return geode::Ok(std::move(preset));
    }

    static matjson::Value toJson(FilterPreset const& value) {
        auto json = matjson::Value::object();

        json["name"] = value.name;
        json["searchType"] = value.searchType;
        json["query"] = value.query;
        json["difficulty"] = value.difficulty;
        json["length"] = value.length;
        json["star"] = value.star;
        json["uncompleted"] = value.uncompleted;
        json["featured"] = value.featured;
        json["songID"] = value.songID;
        json["original"] = value.original;
        json["twoPlayer"] = value.twoPlayer;
        json["customSong"] = value.customSong;
        json["songFilter"] = value.songFilter;
        json["noStar"] = value.noStar;
        json["coins"] = value.coins;
        json["epic"] = value.epic;
        json["legendary"] = value.legendary;
        json["mythic"] = value.mythic;
        json["completed"] = value.completed;
        json["demonFilter"] = value.demonFilter;
        json["folder"] = value.folder;
        json["searchMode"] = value.searchMode;

        return json;
    }
};

static std::vector<FilterPreset> loadPresets() {
    return Mod::get()->getSavedValue<std::vector<FilterPreset>>(
        "filter-presets",
        std::vector<FilterPreset>{}
    );
}

static void savePresets(std::vector<FilterPreset> const& presets) {
    Mod::get()->setSavedValue("filter-presets", presets);
}

// ============================================================
//  Helpers
// ============================================================

// Removes spaces at both ends of a string
static std::string trimmed(std::string const& text) {
    auto begin = text.find_first_not_of(" \t\r\n");

    if (begin == std::string::npos) {
        return "";
    }

    auto end = text.find_last_not_of(" \t\r\n");

    return text.substr(begin, end - begin + 1);
}

// Sets the size of a button and remembers it, so it doesn't jump when pressed
static void setButtonScale(CCMenuItemSpriteExtra* button, float scale) {
    button->setScale(scale);
    button->m_baseScale = scale;
}

// How wide a button really looks on screen
static float visualWidth(CCNode* button) {
    float width = button->getContentSize().width;

    if (auto inner = button->getChildByIndex(0)) {
        if (inner->getContentSize().width > 0.f) {
            width = inner->getContentSize().width * inner->getScaleX();
        }
    }

    return width * button->getScaleX();
}

// Button with text
static CCMenuItemSpriteExtra* makeTextButton(
    std::string const& text,
    int width,
    char const* texture,
    CCObject* target,
    SEL_MenuHandler callback,
    float height = 28.f,
    float scale = 0.6f
) {
    auto sprite = ButtonSprite::create(
        text.c_str(),
        width,
        true,
        "bigFont.fnt",
        texture,
        height,
        scale
    );

    if (!sprite) {
        return nullptr;
    }

    return CCMenuItemSpriteExtra::create(sprite, target, callback);
}

// Button with an icon
static CCMenuItemSpriteExtra* makeIconButton(
    char const* frame,
    CCObject* target,
    SEL_MenuHandler callback,
    float scale
) {
    auto sprite = CCSprite::createWithSpriteFrameName(frame);

    if (!sprite) {
        return nullptr;
    }

    auto button = CCMenuItemSpriteExtra::create(sprite, target, callback);

    if (button) {
        setButtonScale(button, scale);
    }

    return button;
}

// Text field with a proper background
static geode::TextInput* makeInput(
    float width,
    char const* placeholder,
    size_t maxChars
) {
    auto input = geode::TextInput::create(width, placeholder, "bigFont.fnt");

    if (input) {
        input->setCommonFilter(geode::CommonFilter::Any);
        input->setMaxCharCount(maxChars);
    }

    return input;
}

// Remembers the filters currently selected on the search screen
static std::optional<FilterPreset> capturePreset(
    LevelSearchLayer* layer,
    std::string const& name,
    std::string const& levelName,
    int searchType
) {
    if (!layer) {
        return std::nullopt;
    }

    auto search = layer->getSearchObject(
        static_cast<SearchType>(searchType),
        levelName
    );

    if (!search) {
        return std::nullopt;
    }

    FilterPreset preset;

    preset.name = name;
    preset.searchType = searchType;
    preset.query = levelName;
    preset.difficulty = search->m_difficulty;
    preset.length = search->m_length;
    preset.star = search->m_starFilter;
    preset.uncompleted = search->m_uncompletedFilter;
    preset.featured = search->m_featuredFilter;
    preset.songID = search->m_songID;
    preset.original = search->m_originalFilter;
    preset.twoPlayer = search->m_twoPlayerFilter;
    preset.customSong = search->m_customSongFilter;
    preset.songFilter = search->m_songFilter;
    preset.noStar = search->m_noStarFilter;
    preset.coins = search->m_coinsFilter;
    preset.epic = search->m_epicFilter;
    preset.legendary = search->m_legendaryFilter;
    preset.mythic = search->m_mythicFilter;
    preset.completed = search->m_completedFilter;
    preset.demonFilter = static_cast<int>(search->m_demonFilter);
    preset.folder = search->m_folder;
    preset.searchMode = search->m_searchMode;

    return preset;
}

// Three list types: Search (0), Recent (4), Liked (2)
static int const kModeTypes[3] = {0, 4, 2};
static char const* const kModeNames[3] = {"Search", "Recent", "Liked"};

// ============================================================
//  Presets window
// ============================================================

class PresetsPopup : public geode::Popup {
protected:
    LevelSearchLayer* m_owner = nullptr;

    geode::TextInput* m_nameInput = nullptr;
    geode::TextInput* m_queryInput = nullptr;
    geode::ScrollLayer* m_scroll = nullptr;

    CCMenu* m_menu = nullptr;
    CCMenu* m_listMenu = nullptr;
    CCLabelBMFont* m_status = nullptr;
    ButtonSprite* m_modeSprites[3] = {nullptr, nullptr, nullptr};

    int m_searchType = 0;

    bool init(LevelSearchLayer* owner) {
        m_owner = owner;

        if (!Popup::init(420.f, 290.f)) {
            return false;
        }

        this->setTitle("Filter Presets");

        auto size = m_mainLayer->getContentSize();
        float centerX = size.width / 2.f;

        // ---------- Preset list ----------
        float listW = 380.f;
        float listH = 110.f;
        float listBottom = 132.f;

        auto listBg = CCScale9Sprite::create(
            "square02b_001.png",
            {0.f, 0.f, 80.f, 80.f}
        );

        if (listBg) {
            listBg->setColor(ccColor3B{0, 0, 0});
            listBg->setOpacity(90);
            listBg->setContentSize({listW, listH});
            listBg->setPosition({centerX, listBottom + listH / 2.f});
            m_mainLayer->addChild(listBg, 1);
        }

        m_scroll = geode::ScrollLayer::create(
            CCSize(listW - 8.f, listH - 8.f)
        );

        if (!m_scroll) {
            return false;
        }

        m_scroll->setAnchorPoint({0.5f, 0.5f});
        m_scroll->setPosition({centerX, listBottom + listH / 2.f});
        m_scroll->enableScrollClipping(true);
        m_mainLayer->addChild(m_scroll, 2);

        // ---------- Text fields ----------
        float nameY = 108.f;
        float queryY = 76.f;

        m_nameInput = makeInput(250.f, "Preset name", 18);
        m_queryInput = makeInput(250.f, "Search text (optional)", 40);

        if (!m_nameInput || !m_queryInput) {
            return false;
        }

        m_nameInput->setPosition({20.f + 125.f, nameY});
        m_mainLayer->addChild(m_nameInput, 5);

        m_queryInput->setPosition({20.f + 125.f, queryY});
        m_mainLayer->addChild(m_queryInput, 5);

        // ---------- Buttons ----------
        m_menu = CCMenu::create();
        m_menu->setPosition({0.f, 0.f});
        m_mainLayer->addChild(m_menu, 10);

        auto saveButton = makeTextButton(
            "Save",
            100,
            "GJ_button_01.png",
            this,
            menu_selector(PresetsPopup::onSave),
            30.f,
            0.7f
        );

        if (saveButton) {
            saveButton->setPosition({340.f, nameY});
            m_menu->addChild(saveButton);
        }

        // Label and three list type buttons
        float modeY = 46.f;

        auto modeLabel = CCLabelBMFont::create("Mode:", "bigFont.fnt");
        modeLabel->setScale(0.5f);
        modeLabel->setAnchorPoint({0.f, 0.5f});
        modeLabel->setPosition({22.f, modeY});
        m_mainLayer->addChild(modeLabel, 3);

        for (int i = 0; i < 3; i++) {
            auto sprite = ButtonSprite::create(
                kModeNames[i],
                84,
                true,
                "bigFont.fnt",
                "GJ_button_02.png",
                26.f,
                0.55f
            );

            if (!sprite) {
                continue;
            }

            auto item = CCMenuItemSpriteExtra::create(
                sprite,
                this,
                menu_selector(PresetsPopup::onSelectMode)
            );

            item->setTag(i);
            item->setPosition({165.f + i * 90.f, modeY});
            m_menu->addChild(item);

            m_modeSprites[i] = sprite;
        }

        // ---------- Message line ----------
        m_status = CCLabelBMFont::create("", "bigFont.fnt");
        m_status->setPosition({centerX, 18.f});
        m_mainLayer->addChild(m_status, 3);

        setStatus(
            "Set your filters on the search screen, then save a preset",
            ccColor3B{255, 255, 255}
        );

        updateModeButtons();
        refreshList();

        return true;
    }

    // Shows a message at the bottom (and shrinks it if it is too long)
    void setStatus(std::string const& text, ccColor3B color) {
        m_status->setString(text.c_str());
        m_status->setColor(color);

        float width = m_status->getContentSize().width;
        float scale = 0.4f;

        if (width > 0.f && width * scale > 390.f) {
            scale = 390.f / width;
        }

        m_status->setScale(scale);
    }

    // Highlights the selected list type in green
    void updateModeButtons() {
        for (int i = 0; i < 3; i++) {
            if (!m_modeSprites[i]) {
                continue;
            }

            m_modeSprites[i]->updateBGImage(
                kModeTypes[i] == m_searchType
                    ? "GJ_button_01.png"
                    : "GJ_button_02.png"
            );
        }
    }

    // Redraws the preset list
    void refreshList() {
        if (m_listMenu) {
            m_listMenu->removeFromParentAndCleanup(true);
            m_listMenu = nullptr;
        }

        auto presets = loadPresets();

        float viewW = m_scroll->getContentSize().width;
        float viewH = m_scroll->getContentSize().height;
        float rowH = 32.f;

        float contentH = std::max(
            viewH,
            static_cast<float>(presets.size()) * rowH + 6.f
        );

        m_scroll->m_contentLayer->setContentSize({viewW, contentH});

        m_listMenu = CCMenu::create();
        m_listMenu->setPosition({0.f, 0.f});
        m_listMenu->setContentSize({viewW, contentH});
        m_scroll->m_contentLayer->addChild(m_listMenu);

        if (presets.empty()) {
            auto emptyLabel = CCLabelBMFont::create(
                "No presets yet. Create one below.",
                "bigFont.fnt"
            );

            emptyLabel->setScale(0.45f);
            emptyLabel->setPosition({viewW / 2.f, contentH / 2.f});
            m_listMenu->addChild(emptyLabel);
        }

        for (int i = 0; i < static_cast<int>(presets.size()); i++) {
            float y = contentH - 3.f - rowH / 2.f - i * rowH;

            auto applyButton = makeTextButton(
                presets[i].name,
                262,
                "GJ_button_01.png",
                this,
                menu_selector(PresetsPopup::onApply)
            );

            if (applyButton) {
                applyButton->setTag(i);
                applyButton->setPosition({137.f, y});
                m_listMenu->addChild(applyButton);
            }

            auto overwriteButton = makeIconButton(
                "GJ_editBtn_001.png",
                this,
                menu_selector(PresetsPopup::onOverwrite),
                0.6f
            );

            if (overwriteButton) {
                overwriteButton->setTag(i);
                overwriteButton->setPosition({292.f, y});
                m_listMenu->addChild(overwriteButton);
            }

            auto deleteButton = makeIconButton(
                "GJ_deleteBtn_001.png",
                this,
                menu_selector(PresetsPopup::onDelete),
                0.6f
            );

            if (deleteButton) {
                deleteButton->setTag(i);
                deleteButton->setPosition({328.f, y});
                m_listMenu->addChild(deleteButton);
            }
        }

        m_scroll->scrollToTop();
    }

    // Replaces the filters of a preset with the current ones
    void overwritePreset(int index) {
        auto presets = loadPresets();

        if (index < 0 || index >= static_cast<int>(presets.size())) {
            return;
        }

        auto updated = capturePreset(
            m_owner,
            presets[index].name,
            presets[index].query,
            presets[index].searchType
        );

        if (!updated) {
            setStatus(
                "Could not read the current filters",
                ccColor3B{255, 100, 100}
            );
            return;
        }

        presets[index] = *updated;
        savePresets(presets);

        setStatus(
            "Updated: " + presets[index].name,
            ccColor3B{130, 255, 130}
        );
    }

    // Removes a preset
    void deletePreset(int index) {
        auto presets = loadPresets();

        if (index < 0 || index >= static_cast<int>(presets.size())) {
            return;
        }

        std::string name = presets[index].name;

        presets.erase(presets.begin() + index);
        savePresets(presets);

        setStatus("Deleted: " + name, ccColor3B{130, 255, 130});
        refreshList();
    }

    // ---------- Button handlers ----------

    void onSelectMode(CCObject* sender) {
        int index = static_cast<CCNode*>(sender)->getTag();

        if (index < 0 || index > 2) {
            return;
        }

        m_searchType = kModeTypes[index];
        updateModeButtons();
    }

    void onSave(CCObject*) {
        std::string name = trimmed(m_nameInput->getString());
        std::string query = trimmed(m_queryInput->getString());

        if (name.empty()) {
            setStatus("Enter a preset name first", ccColor3B{255, 100, 100});
            return;
        }

        auto presets = loadPresets();

        for (auto const& preset : presets) {
            if (preset.name == name) {
                setStatus(
                    "A preset with this name already exists",
                    ccColor3B{255, 100, 100}
                );
                return;
            }
        }

        auto preset = capturePreset(m_owner, name, query, m_searchType);

        if (!preset) {
            setStatus(
                "Could not read the current filters",
                ccColor3B{255, 100, 100}
            );
            return;
        }

        presets.push_back(*preset);
        savePresets(presets);

        m_nameInput->setString("");

        setStatus("Saved: " + name, ccColor3B{130, 255, 130});
        refreshList();
    }

    void onApply(CCObject* sender) {
        int index = static_cast<CCNode*>(sender)->getTag();
        auto presets = loadPresets();

        if (index < 0 || index >= static_cast<int>(presets.size())) {
            return;
        }

        // A copy: after closing the window only this copy is used
        FilterPreset preset = presets[index];

        this->onClose(nullptr);

        auto search = GJSearchObject::create(
            static_cast<SearchType>(preset.searchType),
            preset.query,
            preset.difficulty,
            preset.length,
            0,
            preset.star,
            preset.uncompleted,
            preset.featured,
            preset.songID,
            preset.original,
            preset.twoPlayer,
            preset.customSong,
            preset.songFilter,
            preset.noStar,
            preset.coins,
            preset.epic,
            preset.legendary,
            preset.mythic,
            preset.completed,
            preset.demonFilter,
            preset.folder,
            preset.searchMode
        );

        if (!search) {
            return;
        }

        auto scene = LevelBrowserLayer::scene(search);

        if (!scene) {
            return;
        }

        CCDirector::sharedDirector()->pushScene(
            CCTransitionFade::create(0.5f, scene)
        );
    }

    void onOverwrite(CCObject* sender) {
        int index = static_cast<CCNode*>(sender)->getTag();
        auto presets = loadPresets();

        if (index < 0 || index >= static_cast<int>(presets.size())) {
            return;
        }

        std::string name = presets[index].name;

        geode::createQuickPopup(
            "Update Preset",
            "Replace the filters saved in \"" + name +
                "\" with the filters currently selected on the search screen?",
            "Cancel",
            "Update",
            [this, index](FLAlertLayer*, bool confirmed) {
                if (confirmed) {
                    this->overwritePreset(index);
                }
            }
        );
    }

    void onDelete(CCObject* sender) {
        int index = static_cast<CCNode*>(sender)->getTag();
        auto presets = loadPresets();

        if (index < 0 || index >= static_cast<int>(presets.size())) {
            return;
        }

        std::string name = presets[index].name;

        geode::createQuickPopup(
            "Delete Preset",
            "Are you sure you want to delete \"" + name + "\"?",
            "Cancel",
            "Delete",
            [this, index](FLAlertLayer*, bool confirmed) {
                if (confirmed) {
                    this->deletePreset(index);
                }
            }
        );
    }

public:
    static PresetsPopup* create(LevelSearchLayer* owner) {
        auto ret = new PresetsPopup();

        if (ret && ret->init(owner)) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }
};

// ============================================================
//  Button in the right column of the level search screen
// ============================================================

class $modify(PresetLevelSearchLayer, LevelSearchLayer) {
    bool init(int type) {
        if (!LevelSearchLayer::init(type)) {
            return false;
        }

        auto sprite = CCSprite::createWithSpriteFrameName("GJ_editBtn_001.png");

        if (!sprite) {
            return true;
        }

        float nativeWidth = sprite->getContentSize().width;

        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(PresetLevelSearchLayer::onOpenPresets)
        );

        if (!button) {
            return true;
        }

        button->setID("presets-button"_spr);

        // The right column (clear filters, advanced filters, lists...)
        CCNode* column = this->getChildByID("other-filter-menu");

        if (!column) {
            column = this->getChildByType<CCMenu>(0);
        }

        // Match the size of the neighbouring buttons exactly
        float scale = 0.8f;

        if (column) {
            if (auto reference = column->getChildByIndex(0)) {
                float referenceWidth = visualWidth(reference);

                if (referenceWidth > 5.f && nativeWidth > 0.f) {
                    scale = referenceWidth / nativeWidth;
                }
            }
        }

        setButtonScale(button, scale * kButtonScaleTweak);

        if (column && column->getLayout()) {
            column->addChild(button);
            column->updateLayout();
        } else {
            // Fallback: our own small menu near the right edge
            auto winSize = CCDirector::sharedDirector()->getWinSize();

            auto ownMenu = CCMenu::create();
            ownMenu->setPosition({winSize.width - 24.f, winSize.height * 0.30f});
            ownMenu->setID("presets-menu"_spr);

            button->setPosition({0.f, 0.f});
            ownMenu->addChild(button);

            this->addChild(ownMenu, 10);
        }

        return true;
    }

    void onOpenPresets(CCObject*) {
        auto popup = PresetsPopup::create(this);

        if (popup) {
            popup->show();
        }
    }
};
