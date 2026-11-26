
#include <Geode/Geode.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <algorithm>
#include <optional>
#include <string>
#include <utility>
#include <vector>

using namespace geode::prelude;

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

static float getSpriteScale(CCSprite* sprite, float targetSize) {
    if (!sprite) {
        return 1.f;
    }

    auto size = sprite->getContentSize();
    float dimension = std::max(size.width, size.height);

    if (dimension <= 0.f) {
        return 1.f;
    }

    return targetSize / dimension;
}

static CCMenuItemSpriteExtra* createLabeledButton(
    std::string const& text,
    float width,
    char const* background,
    CCObject* target,
    SEL_MenuHandler callback,
    float textScale = 0.42f
) {
    auto sprite = ButtonSprite::create(
        text.c_str(),
        width,
        0,
        textScale,
        false,
        "bigFont.fnt",
        background
    );

    if (!sprite) {
        return nullptr;
    }

    return CCMenuItemSpriteExtra::create(
        sprite,
        target,
        callback
    );
}

static CCMenuItemSpriteExtra* createIconButton(
    char const* frame,
    CCObject* target,
    SEL_MenuHandler callback,
    float size
) {
    auto sprite = CCSprite::createWithSpriteFrameName(frame);

    if (!sprite) {
        return nullptr;
    }

    sprite->setScale(getSpriteScale(sprite, size));

    return CCMenuItemSpriteExtra::create(
        sprite,
        target,
        callback
    );
}

static CCMenuItemSpriteExtra* createPresetButton(
    int index,
    CCObject* target,
    SEL_MenuHandler callback
) {
    auto sprite = CCSprite::createWithSpriteFrameName(
        "GJ_copyBtn_001.png"
    );

    if (!sprite) {
        return nullptr;
    }

    sprite->setScale(getSpriteScale(sprite, 52.f));

    auto item = CCMenuItemSpriteExtra::create(
        sprite,
        target,
        callback
    );

    if (!item) {
        return nullptr;
    }

    auto number = CCLabelBMFont::create(
        std::to_string(index + 1).c_str(),
        "bigFont.fnt"
    );

    if (number) {
        number->setScale(0.48f);
        number->setPosition({
            sprite->getContentSize().width / 2.f,
            sprite->getContentSize().height / 2.f
        });

        sprite->addChild(number, 10);
    }

    item->setTag(index);

    return item;
}

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

class PresetManagerPopup;

class $modify(PresetLevelSearchLayer, LevelSearchLayer) {
public:
    struct Fields {
        CCMenu* presetMenu = nullptr;
    };

    bool init(int type) {
        if (!LevelSearchLayer::init(type)) {
            return false;
        }

        refreshPresetButtons();

        return true;
    }

    void refreshPresetButtons() {
        if (m_fields->presetMenu) {
            this->removeChild(m_fields->presetMenu, true);
            m_fields->presetMenu = nullptr;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto menu = CCMenu::create();
        menu->setPosition({0.f, 0.f});
        menu->setID("filter-presets-menu"_spr);

        this->addChild(menu, 1000);
        m_fields->presetMenu = menu;

        float startX = winSize.width - 52.f;
        float managerY = winSize.height * 0.34f;

        auto managerButton = createIconButton(
            "GJ_plusBtn_001.png",
            this,
            menu_selector(PresetLevelSearchLayer::onOpenPresetManager),
            52.f
        );

        if (managerButton) {
            managerButton->setPosition({
                startX,
                managerY
            });

            menu->addChild(managerButton);
        }

        auto presets = loadPresets();

        int columns = winSize.width < 1100.f ? 2 : 3;

        float startY = managerY - 58.f;
        float horizontalSpacing = 62.f;
        float verticalSpacing = 72.f;

        for (int i = 0; i < static_cast<int>(presets.size()); i++) {
            int column = i % columns;
            int row = i / columns;

            float x = startX - column * horizontalSpacing;
            float y = startY - row * verticalSpacing;

            auto button = createPresetButton(
                i,
                this,
                menu_selector(PresetLevelSearchLayer::onApplyPreset)
            );

            if (!button) {
                continue;
            }

            button->setPosition({x, y});
            menu->addChild(button);

            std::string labelText = presets[i].name;

            if (labelText.size() > 9) {
                labelText = labelText.substr(0, 8) + ".";
            }

            auto label = CCLabelBMFont::create(
                labelText.c_str(),
                "chatFont.fnt"
            );

            if (label) {
                float scale = 0.42f;

                if (label->getContentSize().width * scale > 66.f) {
                    scale = 66.f / label->getContentSize().width;
                }

                label->setScale(scale);
                label->setPosition({
                    x,
                    y - 33.f
                });

                menu->addChild(label);
            }
        }
    }

    void onOpenPresetManager(CCObject*);

    void onApplyPreset(CCObject* sender) {
        auto item = static_cast<CCNode*>(sender);
        int index = item->getTag();

        auto presets = loadPresets();

        if (index < 0 || index >= static_cast<int>(presets.size())) {
            return;
        }

        auto const& preset = presets[index];

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
            FLAlertLayer::create(
                "Error",
                "Could not apply this preset.",
                "OK"
            )->show();

            return;
        }

        auto scene = LevelBrowserLayer::scene(search);

        if (!scene) {
            FLAlertLayer::create(
                "Error",
                "Could not open the search results.",
                "OK"
            )->show();

            return;
        }

        auto transition = CCTransitionFade::create(0.5f, scene);

        if (transition) {
            CCDirector::sharedDirector()->pushScene(transition);
        }
    }
};

class PresetManagerPopup : public geode::Popup {
protected:
    PresetLevelSearchLayer* m_owner = nullptr;

    CCTextInputNode* m_nameInput = nullptr;
    CCTextInputNode* m_levelNameInput = nullptr;

    geode::ScrollLayer* m_scrollLayer = nullptr;

    CCMenu* m_listMenu = nullptr;
    CCMenu* m_modeMenu = nullptr;
    CCMenu* m_actionMenu = nullptr;

    int m_selectedIndex = -1;
    int m_selectedSearchType = 0;

    float m_popupWidth = 0.f;
    float m_popupHeight = 0.f;

    bool init(PresetLevelSearchLayer* owner) {
        m_owner = owner;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        m_popupWidth = std::min(
            500.f,
            winSize.width * 0.82f
        );

        m_popupHeight = std::min(
            340.f,
            winSize.height * 0.78f
        );

        if (!Popup::init(m_popupWidth, m_popupHeight)) {
            return false;
        }

        this->setTitle("Preset Manager");

        auto size = m_mainLayer->getContentSize();

        auto listTitle = CCLabelBMFont::create(
            "Saved Presets",
            "bigFont.fnt"
        );

        listTitle->setScale(0.43f);
        listTitle->setPosition({
            size.width / 2.f,
            size.height - 48.f
        });

        m_mainLayer->addChild(listTitle);

        float scrollWidth = size.width - 36.f;
        float scrollHeight = 65.f;
        float scrollY = size.height - 105.f;

        m_scrollLayer = geode::ScrollLayer::create(
            CCSize(scrollWidth, scrollHeight)
        );

        if (!m_scrollLayer) {
            return false;
        }

        m_scrollLayer->setAnchorPoint({0.5f, 0.5f});
        m_scrollLayer->setPosition({
            size.width / 2.f,
            scrollY
        });

        m_scrollLayer->enableScrollClipping(true);
        m_mainLayer->addChild(m_scrollLayer, 2);

        float labelX = 27.f;
        float fieldX = 139.f;
        float fieldWidth = size.width - fieldX - 24.f;

        float levelNameY = 112.f;
        float nameY = 75.f;

        auto levelNameLabel = CCLabelBMFont::create(
            "Level Name:",
            "bigFont.fnt"
        );

        levelNameLabel->setScale(0.35f);
        levelNameLabel->setAnchorPoint({0.f, 0.5f});
        levelNameLabel->setPosition({
            labelX,
            levelNameY
        });

        m_mainLayer->addChild(levelNameLabel, 3);

        m_levelNameInput = CCTextInputNode::create(
            fieldWidth,
            30.f,
            "",
            "bigFont.fnt"
        );

        m_levelNameInput->setPosition({
            fieldX + fieldWidth / 2.f,
            levelNameY
        });

        m_levelNameInput->setID("level-name-input"_spr);

        m_mainLayer->addChild(m_levelNameInput, 5);

        auto nameLabel = CCLabelBMFont::create(
            "Preset Name:",
            "bigFont.fnt"
        );

        nameLabel->setScale(0.35f);
        nameLabel->setAnchorPoint({0.f, 0.5f});
        nameLabel->setPosition({
            labelX,
            nameY
        });

        m_mainLayer->addChild(nameLabel, 3);

        m_nameInput = CCTextInputNode::create(
            fieldWidth,
            30.f,
            "",
            "bigFont.fnt"
        );

        m_nameInput->setPosition({
            fieldX + fieldWidth / 2.f,
            nameY
        });

        m_nameInput->setID("preset-name-input"_spr);

        m_mainLayer->addChild(m_nameInput, 5);

        auto modeLabel = CCLabelBMFont::create(
            "Search Type",
            "bigFont.fnt"
        );

        modeLabel->setScale(0.42f);
        modeLabel->setPosition({
            size.width / 2.f,
            186.f
        });

        m_mainLayer->addChild(modeLabel, 3);

        int currentType = static_cast<int>(owner->m_type);

        if (currentType == 0 || currentType == 2 || currentType == 4) {
            m_selectedSearchType = currentType;
        }

        rebuildSearchModeButtons();
        refreshList();

        return true;
    }

    void rebuildSearchModeButtons() {
        if (m_modeMenu) {
            m_modeMenu->removeFromParentAndCleanup(true);
            m_modeMenu = nullptr;
        }

        m_modeMenu = CCMenu::create();
        m_modeMenu->setPosition({0.f, 0.f});

        m_mainLayer->addChild(m_modeMenu, 4);

        auto searchButton = createLabeledButton(
            "Search",
            112.f,
            m_selectedSearchType == 0
                ? "GJ_button_01.png"
                : "GJ_button_02.png",
            this,
            menu_selector(PresetManagerPopup::onSelectSearch),
            0.40f
        );

        auto recentButton = createLabeledButton(
            "Recent",
            112.f,
            m_selectedSearchType == 4
                ? "GJ_button_01.png"
                : "GJ_button_02.png",
            this,
            menu_selector(PresetManagerPopup::onSelectRecent),
            0.40f
        );

        auto likedButton = createLabeledButton(
            "Most Liked",
            112.f,
            m_selectedSearchType == 2
                ? "GJ_button_01.png"
                : "GJ_button_02.png",
            this,
            menu_selector(PresetManagerPopup::onSelectMostLiked),
            0.40f
        );

        float y = 160.f;

        if (searchButton) {
            searchButton->setPosition({
                m_popupWidth * 0.19f,
                y
            });

            m_modeMenu->addChild(searchButton);
        }

        if (recentButton) {
            recentButton->setPosition({
                m_popupWidth * 0.50f,
                y
            });

            m_modeMenu->addChild(recentButton);
        }

        if (likedButton) {
            likedButton->setPosition({
                m_popupWidth * 0.81f,
                y
            });

            m_modeMenu->addChild(likedButton);
        }
    }

    void refreshList() {
        if (m_listMenu) {
            m_listMenu->removeFromParentAndCleanup(true);
            m_listMenu = nullptr;
        }

        if (m_actionMenu) {
            m_actionMenu->removeFromParentAndCleanup(true);
            m_actionMenu = nullptr;
        }

        auto presets = loadPresets();

        if (m_selectedIndex >= static_cast<int>(presets.size())) {
            m_selectedIndex = -1;
        }

        float scrollWidth = m_scrollLayer->getContentSize().width;
        float scrollHeight = m_scrollLayer->getContentSize().height;

        int rows = static_cast<int>((presets.size() + 1) / 2);

        float contentHeight = std::max(
            scrollHeight,
            rows * 34.f + 8.f
        );

        m_scrollLayer->m_contentLayer->setContentSize({
            scrollWidth,
            contentHeight
        });

        m_listMenu = CCMenu::create();
        m_listMenu->setPosition({0.f, 0.f});
        m_listMenu->setContentSize({
            scrollWidth,
            contentHeight
        });

        m_scrollLayer->m_contentLayer->addChild(m_listMenu);

        if (presets.empty()) {
            auto emptyLabel = CCLabelBMFont::create(
                "No presets saved yet",
                "bigFont.fnt"
            );

            emptyLabel->setScale(0.42f);
            emptyLabel->setPosition({
                scrollWidth / 2.f,
                contentHeight / 2.f
            });

            m_listMenu->addChild(emptyLabel);
        }

        float buttonWidth = scrollWidth * 0.44f;

        for (int i = 0; i < static_cast<int>(presets.size()); i++) {
            int column = i % 2;
            int row = i / 2;

            auto button = createLabeledButton(
                presets[i].name,
                buttonWidth,
                m_selectedIndex == i
                    ? "GJ_button_01.png"
                    : "GJ_button_02.png",
                this,
                menu_selector(PresetManagerPopup::onSelectPreset),
                0.38f
            );

            if (!button) {
                continue;
            }

            button->setTag(i);

            button->setPosition({
                column == 0
                    ? scrollWidth * 0.25f
                    : scrollWidth * 0.75f,
                contentHeight - 18.f - row * 34.f
            });

            m_listMenu->addChild(button);
        }

        m_scrollLayer->scrollToTop();

        m_actionMenu = CCMenu::create();
        m_actionMenu->setPosition({0.f, 0.f});

        m_mainLayer->addChild(m_actionMenu, 5);

        auto saveButton = createLabeledButton(
            "Save",
            105.f,
            "GJ_button_01.png",
            this,
            menu_selector(PresetManagerPopup::onCreate),
            0.43f
        );

        auto updateButton = createLabeledButton(
            "Update",
            115.f,
            "GJ_button_01.png",
            this,
            menu_selector(PresetManagerPopup::onUpdate),
            0.43f
        );

        auto deleteButton = createIconButton(
            "GJ_deleteBtn_001.png",
            this,
            menu_selector(PresetManagerPopup::onDelete),
            42.f
        );

        float actionY = 27.f;

        if (saveButton) {
            saveButton->setPosition({
                m_popupWidth * 0.19f,
                actionY
            });

            m_actionMenu->addChild(saveButton);
        }

        if (updateButton) {
            updateButton->setPosition({
                m_popupWidth * 0.50f,
                actionY
            });

            updateButton->setEnabled(m_selectedIndex >= 0);
            m_actionMenu->addChild(updateButton);
        }

        if (deleteButton) {
            deleteButton->setPosition({
                m_popupWidth * 0.81f,
                actionY
            });

            deleteButton->setEnabled(m_selectedIndex >= 0);
            m_actionMenu->addChild(deleteButton);
        }
    }

    void onSelectSearch(CCObject*) {
        m_selectedSearchType = 0;
        rebuildSearchModeButtons();
    }

    void onSelectRecent(CCObject*) {
        m_selectedSearchType = 4;
        rebuildSearchModeButtons();
    }

    void onSelectMostLiked(CCObject*) {
        m_selectedSearchType = 2;
        rebuildSearchModeButtons();
    }

    void onSelectPreset(CCObject* sender) {
        auto item = static_cast<CCNode*>(sender);
        int index = item->getTag();

        auto presets = loadPresets();

        if (index < 0 || index >= static_cast<int>(presets.size())) {
            return;
        }

        m_selectedIndex = index;

        m_nameInput->setString(presets[index].name);
        m_levelNameInput->setString(presets[index].query);

        m_selectedSearchType = presets[index].searchType;

        rebuildSearchModeButtons();
        refreshList();
    }

    void onCreate(CCObject*) {
        std::string name = m_nameInput->getString();
        std::string levelName = m_levelNameInput->getString();

        if (name.find_first_not_of(" \t\r\n") == std::string::npos) {
            FLAlertLayer::create(
                "Missing Name",
                "Enter a name for your preset first.",
                "OK"
            )->show();

            return;
        }

        if (levelName.find_first_not_of(" \t\r\n") == std::string::npos) {
            levelName.clear();
        }

        auto presets = loadPresets();

        for (auto const& preset : presets) {
            if (preset.name == name) {
                FLAlertLayer::create(
                    "Name Already Used",
                    "A preset with this name already exists.",
                    "OK"
                )->show();

                return;
            }
        }

        auto preset = capturePreset(
            m_owner,
            name,
            levelName,
            m_selectedSearchType
        );

        if (!preset) {
            FLAlertLayer::create(
                "Error",
                "Could not save the current filters.",
                "OK"
            )->show();

            return;
        }

        presets.push_back(*preset);
        savePresets(presets);

        m_selectedIndex = static_cast<int>(presets.size()) - 1;

        m_owner->refreshPresetButtons();
        refreshList();
    }

    void onUpdate(CCObject*) {
        auto presets = loadPresets();

        if (m_selectedIndex < 0 ||
            m_selectedIndex >= static_cast<int>(presets.size())) {
            return;
        }

        std::string name = m_nameInput->getString();
        std::string levelName = m_levelNameInput->getString();

        if (name.find_first_not_of(" \t\r\n") == std::string::npos) {
            FLAlertLayer::create(
                "Missing Name",
                "Enter a preset name first.",
                "OK"
            )->show();

            return;
        }

        if (levelName.find_first_not_of(" \t\r\n") == std::string::npos) {
            levelName.clear();
        }

        for (int i = 0; i < static_cast<int>(presets.size()); i++) {
            if (i != m_selectedIndex && presets[i].name == name) {
                FLAlertLayer::create(
                    "Name Already Used",
                    "A preset with this name already exists.",
                    "OK"
                )->show();

                return;
            }
        }

        presets[m_selectedIndex].name = name;
        presets[m_selectedIndex].query = levelName;
        presets[m_selectedIndex].searchType = m_selectedSearchType;

        savePresets(presets);

        m_owner->refreshPresetButtons();
        refreshList();

        FLAlertLayer::create(
            "Preset Updated",
            "Your preset changes have been saved.",
            "OK"
        )->show();
    }

    void onDelete(CCObject*) {
        if (m_selectedIndex < 0) {
            return;
        }

        int index = m_selectedIndex;

        geode::createQuickPopup(
            "Delete Preset",
            "Are you sure you want to delete this preset?",
            "Cancel",
            "Delete",
            [this, index](auto, bool confirmed) {
                if (!confirmed) {
                    return;
                }

                auto presets = loadPresets();

                if (index < 0 ||
                    index >= static_cast<int>(presets.size())) {
                    return;
                }

                presets.erase(presets.begin() + index);
                savePresets(presets);

                m_selectedIndex = -1;

                m_nameInput->setString("");
                m_levelNameInput->setString("");

                m_owner->refreshPresetButtons();
                refreshList();
            }
        );
    }

public:
    static PresetManagerPopup* create(PresetLevelSearchLayer* owner) {
        auto ret = new PresetManagerPopup();

        if (ret && ret->init(owner)) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }
};

void PresetLevelSearchLayer::onOpenPresetManager(CCObject*) {
    auto popup = PresetManagerPopup::create(this);

    if (popup) {
        popup->show();
    }
}
