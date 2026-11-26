
#include <Geode/Geode.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>
#include <algorithm>
#include <optional>
#include <string>
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

static std::optional<FilterPreset> capturePreset(
    LevelSearchLayer* layer,
    std::string const& name
) {
    if (!layer) {
        return std::nullopt;
    }

    std::string query = layer->m_searchInput
        ? layer->m_searchInput->getString()
        : std::string();

    auto search = layer->getSearchObject(
        static_cast<SearchType>(layer->m_type),
        query
    );

    if (!search) {
        return std::nullopt;
    }

    FilterPreset preset;

    preset.name = name;
    preset.searchType = static_cast<int>(search->m_searchType);
    preset.query = search->m_searchQuery;
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

        auto manageSprite = ButtonSprite::create(
            "Пресеты",
            180,
            0,
            0.42f,
            false,
            "bigFont.fnt",
            "GJ_button_01.png"
        );

        auto manageButton = CCMenuItemSpriteExtra::create(
            manageSprite,
            this,
            menu_selector(PresetLevelSearchLayer::onOpenPresetManager)
        );

        manageButton->setPosition({
            winSize.width - 52.f,
            winSize.height * 0.34f
        });

        menu->addChild(manageButton);

        auto presets = loadPresets();

        int columns = winSize.width < 1100.f ? 2 : 3;
        float startY = winSize.height * 0.34f - 42.f;
        float startX = winSize.width - 52.f;

        for (int i = 0; i < static_cast<int>(presets.size()); i++) {
            auto const& preset = presets[i];

            auto sprite = ButtonSprite::create(
                preset.name.c_str(),
                180,
                0,
                0.42f,
                false,
                "bigFont.fnt",
                "GJ_button_01.png"
            );

            auto button = CCMenuItemSpriteExtra::create(
                sprite,
                this,
                menu_selector(PresetLevelSearchLayer::onApplyPreset)
            );

            int column = i % columns;
            int row = i / columns;

            button->setTag(i);
            button->setPosition({
                startX - column * 82.f,
                startY - row * 29.f
            });

            menu->addChild(button);
        }
    }

    void onOpenPresetManager(CCObject* sender);

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
                "Ошибка",
                "Не удалось применить этот пресет.",
                "ОК"
            )->show();

            return;
        }

        auto scene = LevelBrowserLayer::scene(search);

        if (!scene) {
            FLAlertLayer::create(
                "Ошибка",
                "Не удалось открыть результаты поиска.",
                "ОК"
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
    CCMenu* m_listMenu = nullptr;
    CCLabelBMFont* m_emptyLabel = nullptr;
    int m_selectedIndex = -1;

    bool init(PresetLevelSearchLayer* owner) {
        m_owner = owner;

        auto presets = loadPresets();
        int rows = static_cast<int>((presets.size() + 1) / 2);

        float popupHeight = std::max(
            300.f,
            190.f + rows * 28.f
        );

        popupHeight = std::min(popupHeight, 720.f);

        if (!Popup::init(420.f, popupHeight)) {
            return false;
        }

        this->setTitle("Управление пресетами");

        auto size = m_mainLayer->getContentSize();

        auto description = CCLabelBMFont::create(
            "Выбери пресет для изменения",
            "bigFont.fnt"
        );

        description->setScale(0.45f);
        description->setPosition({
            size.width / 2.f,
            size.height - 43.f
        });

        m_mainLayer->addChild(description);

        m_nameInput = CCTextInputNode::create(
            250.f,
            30.f,
            "Название пресета",
            "bigFont.fnt"
        );

        m_nameInput->setPosition({
            size.width / 2.f,
            75.f
        });

        m_mainLayer->addChild(m_nameInput);

        refreshList();

        return true;
    }

    void refreshList() {
        if (m_listMenu) {
            m_listMenu->removeFromParentAndCleanup(true);
            m_listMenu = nullptr;
        }

        if (m_emptyLabel) {
            m_emptyLabel->removeFromParentAndCleanup(true);
            m_emptyLabel = nullptr;
        }

        auto size = m_mainLayer->getContentSize();

        m_listMenu = CCMenu::create();
        m_listMenu->setPosition({0.f, 0.f});
        m_mainLayer->addChild(m_listMenu);

        auto presets = loadPresets();

        if (presets.empty()) {
            m_emptyLabel = CCLabelBMFont::create(
                "У тебя пока нет пресетов",
                "bigFont.fnt"
            );

            m_emptyLabel->setScale(0.5f);
            m_emptyLabel->setPosition({
                size.width / 2.f,
                size.height - 100.f
            });

            m_mainLayer->addChild(m_emptyLabel);
        }

        float top = size.height - 75.f;

        for (int i = 0; i < static_cast<int>(presets.size()); i++) {
            std::string text = std::to_string(i + 1) + ". " + presets[i].name;

            auto sprite = ButtonSprite::create(
                text.c_str(),
                180,
                0,
                0.42f,
                false,
                "bigFont.fnt",
                "GJ_button_01.png"
            );

            auto item = CCMenuItemSpriteExtra::create(
                sprite,
                this,
                menu_selector(PresetManagerPopup::onSelectPreset)
            );

            int column = i % 2;
            int row = i / 2;

            item->setTag(i);
            item->setPosition({
                column == 0 ? size.width * 0.28f : size.width * 0.72f,
                top - row * 26.f
            });

            m_listMenu->addChild(item);
        }

        auto createSprite = ButtonSprite::create(
            "Создать",
            110,
            0,
            0.45f,
            false,
            "bigFont.fnt",
            "GJ_button_01.png"
        );

        auto createButton = CCMenuItemSpriteExtra::create(
            createSprite,
            this,
            menu_selector(PresetManagerPopup::onCreate)
        );

        createButton->setPosition({70.f, 30.f});
        m_listMenu->addChild(createButton);

        auto renameSprite = ButtonSprite::create(
            "Переименовать",
            135,
            0,
            0.45f,
            false,
            "bigFont.fnt",
            "GJ_button_01.png"
        );

        auto renameButton = CCMenuItemSpriteExtra::create(
            renameSprite,
            this,
            menu_selector(PresetManagerPopup::onRename)
        );

        renameButton->setPosition({210.f, 30.f});
        m_listMenu->addChild(renameButton);

        auto deleteSprite = ButtonSprite::create(
            "Удалить",
            110,
            0,
            0.45f,
            false,
            "bigFont.fnt",
            "GJ_button_01.png"
        );

        auto deleteButton = CCMenuItemSpriteExtra::create(
            deleteSprite,
            this,
            menu_selector(PresetManagerPopup::onDelete)
        );

        deleteButton->setPosition({350.f, 30.f});
        m_listMenu->addChild(deleteButton);
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
    }

    void onCreate(CCObject*) {
        std::string name = m_nameInput->getString();

        if (name.find_first_not_of(" \t\r\n") == std::string::npos) {
            FLAlertLayer::create(
                "Ошибка",
                "Сначала введи название пресета.",
                "ОК"
            )->show();

            return;
        }

        auto presets = loadPresets();

        for (auto const& preset : presets) {
            if (preset.name == name) {
                FLAlertLayer::create(
                    "Ошибка",
                    "Пресет с таким названием уже существует.",
                    "ОК"
                )->show();

                return;
            }
        }

        auto preset = capturePreset(m_owner, name);

        if (!preset) {
            FLAlertLayer::create(
                "Ошибка",
                "Не удалось сохранить текущие фильтры.",
                "ОК"
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
        std::string name = m_nameInput->getString();
        auto presets = loadPresets();

        if (m_selectedIndex < 0 ||
            m_selectedIndex >= static_cast<int>(presets.size())) {
            FLAlertLayer::create(
                "Ошибка",
                "Сначала выбери пресет из списка.",
                "ОК"
            )->show();

            return;
        }

        if (name.find_first_not_of(" \t\r\n") == std::string::npos) {
            return;
        }

        for (int i = 0; i < static_cast<int>(presets.size()); i++) {
            if (i != m_selectedIndex && presets[i].name == name) {
                FLAlertLayer::create(
                    "Ошибка",
                    "Пресет с таким названием уже существует.",
                    "ОК"
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
                "Ошибка",
                "Сначала выбери пресет из списка.",
                "ОК"
            )->show();

            return;
        }

        int index = m_selectedIndex;

        geode::createQuickPopup(
            "Удаление пресета",
            "Ты действительно хочешь удалить выбранный пресет?",
            "Отмена",
            "Удалить",
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
