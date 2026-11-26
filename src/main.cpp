#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class $modify(MyMenuLayer, MenuLayer) {

	bool init() {
		// Сначала даём игре создать обычное меню
		if (!MenuLayer::init()) {
			return false;
		}

		// Рисуем картинку для кнопки (иконка "лайк" из самой игры)
		auto sprite = CCSprite::createWithSpriteFrameName("GJ_likeBtn_001.png");

		// Делаем из картинки нажимаемую кнопку
		auto button = CCMenuItemSpriteExtra::create(
			sprite,
			this,
			menu_selector(MyMenuLayer::onMyButton)
		);
		button->setID("my-button"_spr);

		// Находим нижнее меню и кладём туда нашу кнопку
		auto menu = this->getChildByID("bottom-menu");
		if (menu) {
			menu->addChild(button);
			menu->updateLayout();
		}

		return true;
	}

	// Что происходит при нажатии на кнопку
	void onMyButton(CCObject*) {
		FLAlertLayer::create(
			"Привет!",
			"Мой первый мод работает!",
			"OK"
		)->show();
	}
};
