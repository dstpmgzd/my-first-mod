
#include <Geode/Geode.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>
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

static CCMenuItemSpriteExtra* createLabeledButton(
    std::string const& text,
    float width,
    char const* background,
    CCObject* target,
    SEL_MenuHandler callback
) {
    auto sprite = ButtonSprite::create(
        "",
        width,
        0,
        0.45f,
        false,
        "bigFont.fnt",
        background
    );

    if (!sprite) {
        return nullptr;
    }

    auto label = CCLabelBMFont::create(
        text.c_str(),
        "bigFont.fnt"
    );

    if (!label) {
        return nullptr;
    }

    float scale = 0.42f;
    float maxWidth = width - 12.f;

    if (label->getContentSize().width * scale > maxWidth) {
        scale = maxWidth / label->getContentSize().width;
    }

    label->setScale(scale);
    label->setPosition({
        sprite->getContentSize().width / 2.f,
        sprite->getContentSize().height / 2.f
    });

    sprite->addChild(label, 10);

    return CCMenuItemSpriteExtra::create(
        sprite,
        target,
        callback
    );
}

static CCMenuItemSpriteExtra* createIconTextButton(
    std::string const& text,
    char const* iconFrame,
    float width,
    CCObject* target,
    SEL_MenuHandler callback
) {
    auto sprite = ButtonSprite::create(
        "",
        width,
        0,
        0.45f,
        false,
        "bigFont.fnt",
        "GJ_button_01.png"
    );

    if (!sprite) {
        return nullptr;
    }

    auto icon = CCSprite::createWithSpriteFrameName(iconFrame);

    if (icon) {
        float maxDimension = std::max(
            icon->getContentSize().width,
            icon->getContentSize().height
        );

        icon->setScale(21.f / maxDimension);
        icon->setPosition({
            17.f,
            sprite->getContentSize().height / 2.f
        });

        sprite->addChild(icon, 10);
    }

    auto label = CCLabelBMFont::create(
        text.c_str(),
        "bigFont.fnt"
    );

    if (label) {
        float availableWidth = width - 36.f;
        float scale = 0.38f;

        if (label->getContentSize().width * scale > availableWidth) {
            scale = availableWidth / label->getContentSize().width;
        }

        label->setScale(scale);
        label->setPosition({
            37.f + availableWidth / 2.f,
            sprite->getContentSize().height / 2.f
        });

        sprite->addChild(label, 10);
    }

    return CCMenuItemSpriteExtra::create(
        sprite,
        target,
        callback
    );
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

        float centerX = winSize.width - 155.f;
        float managerY = winSize.height * 0.87f;

        auto plusSprite = CCSprite::createWithSpriteFrameName(
            "GJ_plusBtn_001.png"
        );

        auto managerButton = CCMenuItemSpriteExtra::create(
            plusSprite,
            this,
            menu_selector(PresetLevelSearchLayer::onOpenPresetManager)
        );

        managerButton->setScale(0.75f);
        managerButton->setPosition({centerX, managerY});
        menu->addChild(managerButton);

        auto managerLabel = CCLabelBMFont::create(
            "Presets",
            "bigFont.fnt"
        );

        managerLabel->setScale(0.34f);
        managerLabel->setPosition({
            centerX,
            managerY - 23.f
        });

        menu->addChild(managerLabel);

        auto presets = loadPresets();

        float startY = winSize.height * 0.78f;

        for (int i = 0; i < static_cast<int>(presets.size()); i++) {
            auto button = createLabeledButton(
                presets[i].name,
                140.f,
                "GJ_button_01.png",
                this,
                menu_selector(PresetLevelSearchLayer::onApplyPreset)
            );

            if (!button) {
                continue;
            }

            button->setTag(i);
            button->setScale(0.76f);
            button->setPosition({
                centerX,
                startY - i * 29.f
            });

            menu->addChild(button);
        }
    }

    void onOpenPresetManager(CCObject*) {
        auto popup = PresetManagerPopup::create(this);

        if (popup) {
            popup->show();
        }
    }

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

    CCMenu* m_listMenu = nullptr;
    CCMenu* m_modeMenu = nullptr;
    CCMenu* m_actionMenu = nullptr;

    int m_selectedIndex = -1;
    int m_selectedSearchType = 0;

    bool init(PresetLevelSearchLayer* owner) {
        m_owner = owner;

        auto presets = loadPresets();

        int rows = static_cast<int>(
            (presets.size() + 1) / 2
        );

        float popupHeight = std::min(
            720.f,
            std::max(520.f, 430.f + rows * 30.f)
        );

        if (!Popup::init(500.f, popupHeight)) {
            return false;
        }

        this->setTitle("Preset Manager");

        auto size = m_mainLayer->getContentSize();

        auto listTitle = CCLabelBMFont::create(
            "Saved Presets",
            "bigFont.fnt"
        );

        listTitle->setScale(0.48f);
        listTitle->setPosition({
            size.width / 2.f,
            size.height - 72.f
        });

        m_mainLayer->addChild(listTitle);

        auto nameLabel = CCLabelBMFont::create(
            "Preset Name:",
            "bigFont.fnt"
        );

        nameLabel->setScale(0.42f);
        nameLabel->setAnchorPoint({0.f, 0.5f});
        nameLabel->setPosition({32.f, 135.f});

        m_mainLayer->addChild(nameLabel);

        auto nameBackground = CCLayerColor::create(
            ccColor4B(12, 28, 66, 220),
            290.f,
            36.f
        );

        nameBackground->setPosition({155.f, 117.f});
        m_mainLayer->addChild(nameBackground);

        m_nameInput = CCTextInputNode::create(
            270.f,
            30.f,
            "Enter preset name",
            "bigFont.fnt"
        );

        m_nameInput->setPosition({300.f, 135.f});
        m_nameInput->setID("preset-name-input"_spr);

        m_mainLayer->addChild(m_nameInput, 5);

        auto levelNameLabel = CCLabelBMFont::create(
            "Level Name:",
            "bigFont.fnt"
        );

        levelNameLabel->setScale(0.42f);
        levelNameLabel->setAnchorPoint({0.f, 0.5f});
        levelNameLabel->setPosition({32.f, 295.f});

        m_mainLayer->addChild(levelNameLabel);

        auto levelNameBackground = CCLayerColor::create(
            ccColor4B(12, 28, 66, 220),
            290.f,
            36.f
        );

        levelNameBackground->setPosition({155.f, 277.f});
        m_mainLayer->addChild(levelNameBackground);

        m_levelNameInput = CCTextInputNode::create(
            270.f,
            30.f,
            "e.g. wave challenge (empty = any)",
            "bigFont.fnt"
        );

        m_levelNameInput->setPosition({300.f, 295.f});
        m_levelNameInput->setID("level-name-input"_spr);

        m_mainLayer->addChild(m_levelNameInput, 5);

        auto modeLabel = CCLabelBMFont::create(
            "Search Type:",
            "bigFont.fnt"
        );

        modeLabel->setScale(0.46f);
        modeLabel->setPosition({
            size.width / 2.f,
            225.f
        });

        m_mainLayer->addChild(modeLabel);

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

        m_mainLayer->addChild(m_modeMenu);

        auto searchButton = createLabeledButton(
            "Search",
            120.f,
            m_selectedSearchType == 0
                ? "GJ_button_01.png"
                : "GJ_button_02.png",
            this,
            menu_selector(PresetManagerPopup::onSelectSearch)
        );

        auto recentButton = createLabeledButton(
            "Recent",
            120.f,
            m_selectedSearchType == 4
                ? "GJ_button_01.png"
                : "GJ_button_02.png",
            this,
            menu_selector(PresetManagerPopup::onSelectRecent)
        );

        auto likedButton = createLabeledButton(
            "Most Liked",
            120.f,
            m_selectedSearchType == 2
                ? "GJ_button_01.png"
                : "GJ_button_02.png",
            this,
            menu_selector(PresetManagerPopup::onSelectMostLiked)
        );

        if (searchButton) {
            searchButton->setPosition({85.f, 195.f});
            m_modeMenu->addChild(searchButton);
        }

        if (recentButton) {
            recentButton->setPosition({250.f, 195.f});
            m_modeMenu->addChild(recentButton);
        }

        if (likedButton) {
            likedButton->setPosition({415.f, 195.f});
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

        auto size = m_mainLayer->getContentSize();

        m_listMenu = CCMenu::create();
        m_listMenu->setPosition({0.f, 0.f});

        m_mainLayer->addChild(m_listMenu);

        auto presets = loadPresets();

        if (m_selectedIndex >= static_cast<int>(presets.size())) {
            m_selectedIndex = -1;
        }

        if (presets.empty()) {
            auto emptyLabel = CCLabelBMFont::create(
                "No presets saved yet",
                "bigFont.fnt"
            );

            emptyLabel->setScale(0.48f);
            emptyLabel->setPosition({
                size.width / 2.f,
                size.height - 118.f
            });

            m_listMenu->addChild(emptyLabel);

            auto hintLabel = CCLabelBMFont::create(
                "Create your first preset using the Save button.",
                "bigFont.fnt"
            );

            hintLabel->setScale(0.33f);
            hintLabel->setPosition({
                size.width / 2.f,
                size.height - 145.f
            });

            m_listMenu->addChild(hintLabel);
        }

        float startY = size.height - 118.f;

        for (int i = 0; i < static_cast<int>(presets.size()); i++) {
            int column = i % 2;
            int row = i / 2;

            auto button = createLabeledButton(
                presets[i].name,
                185.f,
                m_selectedIndex == i
                    ? "GJ_button_01.png"
                    : "GJ_button_02.png",
                this,
                menu_selector(PresetManagerPopup::onSelectPreset)
            );

            if (!button) {
                continue;
            }

            button->setTag(i);

            button->setPosition({
                column == 0 ? 130.f : 370.f,
                startY - row * 29.f
            });

            m_listMenu->addChild(button);
        }

        m_actionMenu = CCMenu::create();
        m_actionMenu->setPosition({0.f, 0.f});

        m_mainLayer->addChild(m_actionMenu);

        auto saveButton = createIconTextButton(
            "Save",
            "GJ_plusBtn_001.png",
            115.f,
            this,
            menu_selector(PresetManagerPopup::onCreate)
        );

        auto renameButton = createIconTextButton(
            "Rename",
            "GJ_editBtn_001.png",
            115.f,
            this,
            menu_selector(PresetManagerPopup::onRename)
        );

        auto deleteButton = createIconTextButton(
            "Delete",
            "GJ_deleteBtn_001.png",
            110.f,
            this,
            menu_selector(PresetManagerPopup::onDelete)
        );

        if (saveButton) {
            saveButton->setPosition({85.f, 43.f});
            m_actionMenu->addChild(saveButton);
        }

        if (renameButton) {
            renameButton->setPosition({250.f, 43.f});
            renameButton->setEnabled(m_selectedIndex >= 0);
            m_actionMenu->addChild(renameButton);
        }

        if (deleteButton) {
            deleteButton->setPosition({415.f, 43.f});
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

    void onRename(CCObject*) {
        auto presets = loadPresets();

        if (m_selectedIndex < 0 ||
            m_selectedIndex >= static_cast<int>(presets.size())) {
            FLAlertLayer::create(
                "No Preset Selected",
                "Select a saved preset from the list first.",
                "OK"
            )->show();

            return;
        }

        std::string name = m_nameInput->getString();

        if (name.find_first_not_of(" \t\r\n") == std::string::npos) {
            FLAlertLayer::create(
                "Missing Name",
                "Enter a new name first.",
                "OK"
            )->show();

            return;
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

        savePresets(presets);

        m_owner->refreshPresetButtons();
        refreshList();
    }

    void onDelete(CCObject*) {
        if (m_selectedIndex < 0) {
            FLAlertLayer::create(
                "No Preset Selected",
                "Select a saved preset from the list first.",
                "OK"
            )->show();

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
