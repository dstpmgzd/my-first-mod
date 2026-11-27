#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/ui/Notification.hpp>
#include <functional>
#include <string>
#include <utility>

using namespace geode::prelude;

namespace {

// ============================================================
//  The list page the player opened the current level from
// ============================================================

GJSearchObject* s_search = nullptr;
CCArray* s_levels = nullptr;

// Keeps exactly one object alive in a slot (releases the old one)
template <class T>
void keepOnly(T*& slot, T* value) {
    if (value) {
        value->retain();
    }

    if (slot) {
        slot->release();
    }

    slot = value;
}

// Position of a level (by its ID) inside a list, or -1
int findLevelIndex(CCArray* levels, int levelID) {
    if (!levels) {
        return -1;
    }

    for (unsigned int i = 0; i < levels->count(); i++) {
        auto level = typeinfo_cast<GJGameLevel*>(levels->objectAtIndex(i));

        if (level && level->m_levelID.value() == levelID) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

// First level (starting from `from`) that the player has not completed yet
GJGameLevel* firstUnbeaten(CCArray* levels, int from) {
    if (!levels) {
        return nullptr;
    }

    auto stats = GameStatsManager::sharedState();

    for (int i = from; i < static_cast<int>(levels->count()); i++) {
        auto level = typeinfo_cast<GJGameLevel*>(
            levels->objectAtIndex(static_cast<unsigned int>(i))
        );

        if (!level) {
            continue;
        }

        int id = level->m_levelID.value();

        if (id <= 0) {
            continue;
        }

        if (stats->hasCompletedOnlineLevel(id)) {
            continue;
        }

        return level;
    }

    return nullptr;
}

void showMessage(char const* title, char const* text) {
    FLAlertLayer::create(title, text, "OK")->show();
}

// Leaves the finished level and opens the page of another level
void openLevelPage(PlayLayer* playLayer, GJGameLevel* next) {
    // Keep the level alive while scenes are switching
    Ref<GJGameLevel> target = next;

    // The game's own "leave level" routine (stops music, resets state...)
    playLayer->onQuit();

    auto scene = LevelInfoLayer::scene(target, false);

    if (!scene) {
        return;
    }

    CCDirector::sharedDirector()->replaceScene(
        CCTransitionFade::create(0.5f, scene)
    );
}

// How wide a button really looks on screen (in game units)
float visualWidth(CCNode* button) {
    float width = button->getContentSize().width;

    if (auto inner = button->getChildByIndex(0)) {
        if (inner->getContentSize().width > 0.f) {
            width = inner->getContentSize().width * inner->getScaleX();
        }
    }

    return width * button->getScaleX();
}

// Button with an icon, drawn at the given size (in game units)
CCMenuItemSpriteExtra* makeIconButton(
    char const* frame,
    float size,
    CCObject* target,
    SEL_MenuHandler callback
) {
    auto sprite = CCSprite::createWithSpriteFrameName(frame);

    if (!sprite) {
        return nullptr;
    }

    float width = sprite->getContentSize().width;

    if (width > 0.f) {
        sprite->setScale(size / width);
    }

    return CCMenuItemSpriteExtra::create(sprite, target, callback);
}

// ============================================================
//  Loads the following pages of the list until it finds a level
//  that has not been beaten yet
// ============================================================

enum class LoadResult {
    Found,
    NoMore,
    Failed,
    Cancelled
};

class NextPageLoader : public CCObject, public LevelManagerDelegate {
public:
    using Done = std::function<void(GJGameLevel*, LoadResult)>;

    inline static NextPageLoader* s_current = nullptr;

    GJSearchObject* m_page = nullptr;
    std::string m_key;
    int m_pagesLeft = 30;
    bool m_finished = false;
    Done m_done;

    ~NextPageLoader() {
        if (m_page) {
            m_page->release();
        }
    }

    static bool isBusy() {
        return s_current != nullptr;
    }

    static void cancelCurrent() {
        if (s_current) {
            s_current->finish(nullptr, LoadResult::Cancelled);
        }
    }

    // Starts with the page that comes right after `lastPage`
    static void start(GJSearchObject* lastPage, Done done) {
        if (!lastPage) {
            done(nullptr, LoadResult::NoMore);
            return;
        }

        auto firstPage = lastPage->getPageObject(lastPage->m_page + 1);

        if (!firstPage) {
            done(nullptr, LoadResult::Failed);
            return;
        }

        auto loader = new NextPageLoader();
        loader->m_done = std::move(done);

        keepOnly(loader->m_page, firstPage);
        keepOnly(s_current, loader);
        loader->release();

        loader->request();
    }

    // Stops listening to the game's level manager
    void detach() {
        auto manager = GameLevelManager::sharedState();

        if (manager->m_levelManagerDelegate ==
            static_cast<LevelManagerDelegate*>(this)) {
            manager->m_levelManagerDelegate = nullptr;
        }
    }

    // Asks for the current page (or takes it from the game's cache)
    void request() {
        auto manager = GameLevelManager::sharedState();

        m_key = m_page->getKey();

        if (auto cached = manager->getStoredOnlineLevels(m_key.c_str())) {
            this->handleLevels(cached);
            return;
        }

        manager->m_levelManagerDelegate = this;
        manager->getOnlineLevels(m_page);
    }

    void handleLevels(CCArray* levels) {
        if (m_finished) {
            return;
        }

        this->detach();

        // Empty page: the list has ended
        if (!levels || levels->count() == 0) {
            this->finish(nullptr, LoadResult::NoMore);
            return;
        }

        if (auto found = firstUnbeaten(levels, 0)) {
            // From now on this is "the list the player is in"
            keepOnly(s_search, m_page);
            keepOnly(s_levels, levels);

            this->finish(found, LoadResult::Found);
            return;
        }

        // Everything here is beaten: try the next page
        m_pagesLeft -= 1;

        if (m_pagesLeft <= 0) {
            this->finish(nullptr, LoadResult::NoMore);
            return;
        }

        auto next = m_page->getPageObject(m_page->m_page + 1);

        if (!next) {
            this->finish(nullptr, LoadResult::NoMore);
            return;
        }

        keepOnly(m_page, next);
        this->request();
    }

    void finish(GJGameLevel* level, LoadResult result) {
        if (m_finished) {
            return;
        }

        m_finished = true;
        this->detach();

        Done done = std::move(m_done);
        m_done = nullptr;

        // Stay alive while the callback runs
        this->retain();

        if (s_current == this) {
            keepOnly<NextPageLoader>(s_current, nullptr);
        }

        if (done) {
            done(level, result);
        }

        this->release();
    }

    // ---------- Answers from the game ----------

    void onFinished(CCArray* levels, char const* key) {
        if (m_finished || !key || m_key != key) {
            return;
        }

        this->handleLevels(levels);
    }

    void onFailed() {
        if (m_finished) {
            return;
        }

        this->finish(nullptr, LoadResult::Failed);
    }

    void loadLevelsFinished(CCArray* levels, char const* key) override {
        this->onFinished(levels, key);
    }

    void loadLevelsFinished(CCArray* levels, char const* key, int) override {
        this->onFinished(levels, key);
    }

    void loadLevelsFailed(char const*) override {
        this->onFailed();
    }

    void loadLevelsFailed(char const*, int) override {
        this->onFailed();
    }
};

} // namespace

// ============================================================
//  Remember the list the player is browsing
// ============================================================

class $modify(NextLevelBrowserLayer, LevelBrowserLayer) {
    void setupLevelBrowser(CCArray* items) {
        LevelBrowserLayer::setupLevelBrowser(items);

        if (!items || items->count() == 0 || !m_searchObject) {
            return;
        }

        // Only lists of online levels are interesting
        auto first = typeinfo_cast<GJGameLevel*>(items->objectAtIndex(0));

        if (!first || first->m_levelID.value() <= 0) {
            return;
        }

        keepOnly(s_search, m_searchObject);
        keepOnly(s_levels, items);
    }
};

// ============================================================
//  The "next level" button on the level complete screen
// ============================================================

class $modify(NextLevelEndLayer, EndLevelLayer) {
    void customSetup() {
        EndLevelLayer::customSetup();

        // A leftover search from an earlier screen must not block this one
        NextPageLoader::cancelCurrent();

        if (!m_playLayer || !m_playLayer->m_level) {
            return;
        }

        // Only for online levels
        if (m_playLayer->m_level->m_levelID.value() <= 0) {
            return;
        }

        CCNode* menu = m_mainLayer ? m_mainLayer->getChildByID("button-menu") : nullptr;

        if (!menu) {
            return;
        }

        // The two big circle buttons: retry and exit
        auto retry = menu->getChildByID("retry-button");
        auto exitButton = menu->getChildByID("exit-button");

        if (!retry || !exitButton) {
            return;
        }

        // Same size as the neighbouring buttons
        float size = visualWidth(retry);

        if (size < 20.f || size > 90.f) {
            size = 56.f;
        }

        auto button = makeIconButton(
            "GJ_playBtn2_001.png",
            size,
            this,
            menu_selector(NextLevelEndLayer::onNextLevel)
        );

        if (!button) {
            // Fallback: a plain text button
            auto sprite = ButtonSprite::create(
                "Next",
                60,
                true,
                "bigFont.fnt",
                "GJ_button_01.png",
                30.f,
                0.7f
            );

            if (sprite) {
                button = CCMenuItemSpriteExtra::create(
                    sprite,
                    this,
                    menu_selector(NextLevelEndLayer::onNextLevel)
                );
            }
        }

        if (!button) {
            return;
        }

        button->setID("next-level-button"_spr);

        // Exactly between the retry and exit buttons
        button->setPosition({
            (retry->getPositionX() + exitButton->getPositionX()) / 2.f,
            retry->getPositionY()
        });

        menu->addChild(button);

        // The keybind from the mod settings does the same as the button
        this->addEventListener(
            KeybindSettingPressedEventV3(Mod::get(), "next-level-key"),
            [this](Keybind const&, bool down, bool repeat, double) {
                if (down && !repeat) {
                    this->onNextLevel(nullptr);
                }
            }
        );
    }

    void onNextLevel(CCObject*) {
        if (!m_playLayer || !m_playLayer->m_level) {
            return;
        }

        // Already looking for a level
        if (NextPageLoader::isBusy()) {
            return;
        }

        int currentID = m_playLayer->m_level->m_levelID.value();

        if (currentID <= 0) {
            return;
        }

        // 1. Like the level that was just beaten (once, if enabled)
        if (Mod::get()->getSettingValue<bool>("like-level")) {
            auto manager = GameLevelManager::sharedState();

            if (!manager->hasLikedItemFullCheck(LikeItemType::Level, currentID, 0)) {
                manager->likeItem(LikeItemType::Level, currentID, true, 0);
            }
        }

        // 2. Which list did the player come from?
        int index = findLevelIndex(s_levels, currentID);

        if (index < 0 || !s_search) {
            showMessage(
                "Next Level",
                "This level was not opened from a level list, so there is "
                "no list to continue. Open levels from a search or list "
                "page to use this button."
            );
            return;
        }

        Ref<PlayLayer> playLayer = m_playLayer;

        // 3. Next unbeaten level on the same page?
        if (auto next = firstUnbeaten(s_levels, index + 1)) {
            openLevelPage(playLayer, next);
            return;
        }

        // 4. Look through the following pages
        Ref<Notification> note = Notification::create(
            "Looking for the next level...",
            NotificationIcon::Loading,
            0.f
        );

        note->show();

        NextPageLoader::start(
            s_search,
            [playLayer, note](GJGameLevel* found, LoadResult result) {
                note->hide();

                if (result == LoadResult::Cancelled) {
                    return;
                }

                // The player already left this screen
                if (!playLayer->isRunning()) {
                    return;
                }

                if (found) {
                    openLevelPage(playLayer, found);
                    return;
                }

                if (result == LoadResult::Failed) {
                    showMessage(
                        "Next Level",
                        "Could not load the next page of the list. You may "
                        "be offline, or the list may have ended."
                    );
                    return;
                }

                showMessage(
                    "No More Levels",
                    "There are no more unbeaten levels in this list."
                );
            }
        );
    }
};
